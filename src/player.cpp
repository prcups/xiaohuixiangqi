#include "board.h"
#include "player.h"
#include <algorithm>

Player::~Player()
{
    delete stepTimer;
}

Player::Player(PieceColor color)
    : playerColor(color), type(Human),
      stepTimeLimit(0), totalTimeLimit(0), remainingTotalTime(0), remainingStepTime(0)
{
    stepTimer = new QTimer(this);
    stepTimer->setInterval(1);
    connect(stepTimer, &QTimer::timeout, this, &Player::onStepTimeout);
}

void Player::Go()
{
    if (!board) return;
    if (board->GetDraw())
        bar() << tr("对方求和，如同意请点击求和/接受求和选项");
    board->SetMovable(true);
    if (stepTimeLimit > 0) {
        remainingStepTime = stepTimeLimit;
        stepTimer->start();
    }
}

PieceColor Player::GetColor()
{
    return playerColor;
}

PlayerType Player::GetType()
{
    return type;
}

void Player::Pause()
{
    board->SetMovable(false);
    stepTimer->stop();
}

void Player::SetBoard(Board *newBoard)
{
    board = newBoard;
}

void Player::SetTimeSettings(const TimeSettings &settings)
{
    stepTimeLimit = settings.stepTime * 1000;
    totalTimeLimit = settings.totalTime * 1000;
    remainingTotalTime = settings.totalTime * 1000;
}

int Player::GetRemainingTotalTime() const
{
    if (totalTimeLimit <= 0) return 0;
    return remainingTotalTime / 1000;
}

int Player::GetRemainingStepTime() const
{
    if (stepTimeLimit <= 0) return -1;
    if (!stepTimer->isActive()) {
        if (board->GetCurPlayer() == this) return stepTimeLimit / 1000;
        return -1;
    }
    return remainingStepTime / 1000;
}

int Player::GetRemainingStepTimeLimit() const
{
    return stepTimeLimit;
}

int Player::GetRemainingTotalTimeLimit() const
{
    return totalTimeLimit;
}

void Player::MoveCompleted()
{
    stepTimer->stop();
}

void Player::ResumeTimer()
{
    if (stepTimeLimit > 0 && remainingStepTime > 0) {
        stepTimer->start();
    }
}

void Player::ClearTimeLimits()
{
    stepTimeLimit = 0;
    totalTimeLimit = 0;
    stepTimer->stop();
}

void Player::onStepTimeout()
{
    if (remainingStepTime > 0) {
        --remainingStepTime;
        if (totalTimeLimit > 0 && remainingTotalTime > 0) {
            --remainingTotalTime;
        }
    }
    
    if (remainingStepTime <= 0) {
        stepTimer->stop();
        bar() << (playerColor == Red ? tr("红方") : tr("黑方")) + tr("超时判负");
        if (board) board->Resign();
        return;
    }
    
    if (totalTimeLimit > 0 && remainingTotalTime <= 0) {
        stepTimer->stop();
        bar() << (playerColor == Red ? tr("红方") : tr("黑方")) + tr("超时判负");
        if (board) board->Resign();
    }
}

UCIEngine::UCIEngine(PieceColor color, QString enginePath, int depth)
    : Player(color), depth(depth)
{
    type = Computer;
    engineProcess = new QProcess();
    connect(engineProcess, &QProcess::readyRead, this, &UCIEngine::handleOutput);
    engineProcess->start(enginePath, QStringList());
    engineProcess->write("uci\n");
}

UCIEngine::~UCIEngine()
{
    engineProcess->kill();
    engineProcess->waitForFinished();
    delete engineProcess;
}

void UCIEngine::Go()
{
    if (!board) return;
    if (status != EnginePrepared) {
        deferGo = 1;
        return;
    }
    bar() << (playerColor == Red ? tr("红方") : tr("黑方")) + "正在思考...";
    status = EngineThinking;
    QString output = "position fen ";
    output.append(board->ToFenString());
    output.append("\ngo depth ");
    output.append(QString::number(depth));
    if (board->GetDraw()) output.append(" draw");
    if (stepTimeLimit > 0) {
        output.append(" movetime ");
        output.append(QString::number(stepTimeLimit));
    }
    if (totalTimeLimit > 0) {
        output.append(playerColor == Red ? " wtime " : " btime ");
        output.append(QString::number(remainingTotalTime));
    }
    if (board->GetRivalPlayer()->GetRemainingTotalTimeLimit() > 0) {
        output.append(playerColor == Red ? " btime " : " wtime ");
        output.append(QString::number(board->GetRivalPlayer()->GetRemainingTotalTime()));
    }
    output.append("\n");
    engineProcess->write(output.toLocal8Bit());
    if (stepTimeLimit > 0) {
        remainingStepTime = stepTimeLimit;
        stepTimer->start();
    }
}

void UCIEngine::Pause()
{
    if (status == EngineThinking) {
        status = EnginePrepared;
        engineProcess->write("stop\n");
    }
}

void UCIEngine::handleOutput()
{
    QString output;
    switch (status) {
    case EngineCreated:
        while (1) {
            output = engineProcess->readLine();
            if (output.isEmpty()) break;
            log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: ")) << output;
            if (output == "uciok\n") {
                status = EnginePrepared;
                if (deferGo) {
                    Go();
                    deferGo = 0;
                }
                break;
            }
        }
        break;
    case EnginePrepared:
        output = engineProcess->readAll();
        log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: "));
        break;
    case EngineThinking:
        while (1) {
            output = engineProcess->readLine();
            if (output.isEmpty()) break;
            log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: "))
                  << output;
            if (output.left(8) == "bestmove") {
                auto bestmoveList = output.split(QRegularExpression("[ \n]"));
                if (bestmoveList.size() > 1) {
                    if (bestmoveList[1] == "(none)") {
                        board->Resign();
                        return;
                    }
                    for (int i = 0; i < bestmoveList.size(); ++i) {
                        if (bestmoveList[i] == "draw" && board->RequestDraw()) return;
                        if (bestmoveList[i] == "resign") {
                            board->Resign();
                            return;
                        }
                    }
                    handleShortMoveString(bestmoveList[1]);
                }
                status = EnginePrepared;
                break;
            }
        }
        break;
    }
}

void UCIEngine::handleShortMoveString(const QString &moveString)
{
    int fromX = moveString[1].toLatin1() - '0';
    int fromY = moveString[0].toLatin1() - 'a';
    int toX = moveString[3].toLatin1() - '0';
    int toY = moveString[2].toLatin1() - 'a';
    auto fromPiece = board->GetPiece(fromX, fromY);
    auto toPiece = board->GetPiece(toX, toY);
    if (fromPiece != nullptr && toPiece != nullptr && !fromPiece->Invalid
            && (toPiece->Invalid || fromPiece->GetColor() != toPiece->GetColor()))
        board->Move(fromX, fromY, toX, toY);
}

UCCIEngine::UCCIEngine(PieceColor color, QString enginePath, int depth)
    : Player(color), depth(depth)
{
    type = Computer;
    engineProcess = new QProcess();
    connect(engineProcess, &QProcess::readyRead, this, &UCCIEngine::handleOutput);
    engineProcess->start(enginePath, QStringList());
    engineProcess->write("ucci\n");
}

UCCIEngine::~UCCIEngine()
{
    engineProcess->kill();
    engineProcess->waitForFinished();
    delete engineProcess;
}

void UCCIEngine::Go()
{
    if (!board) return;
    if (status != EnginePrepared) {
        deferGo = 1;
        return;
    }
    bar() << (playerColor == Red ? tr("红方") : tr("黑方")) + "正在思考...";
    status = EngineThinking;
    QString output = "position fen ";
    output.append(board->ToFenString());
    output.append("\ngo depth ");
    output.append(QString::number(depth));
    if (board->GetDraw()) output.append(" draw");
    if (stepTimeLimit > 0 || totalTimeLimit > 0) {
        output.append(" time ");
        output.append(QString::number(stepTimeLimit < remainingTotalTime
                                            ? stepTimeLimit : remainingTotalTime));
    }
    auto rivalPlayer = board->GetRivalPlayer();
    if (rivalPlayer->GetRemainingTotalTimeLimit() > 0
        || rivalPlayer->GetRemainingStepTimeLimit() > 0
    ) {
        output.append(" opptime ");
        output.append(QString::number(rivalPlayer->GetRemainingStepTimeLimit()
            < rivalPlayer->GetRemainingTotalTime() ? rivalPlayer->GetRemainingStepTimeLimit()
            : rivalPlayer->GetRemainingTotalTime()));
    }
    output.append("\n");
    engineProcess->write(output.toLocal8Bit());
    if (stepTimeLimit > 0) {
        remainingStepTime = stepTimeLimit;
        stepTimer->start();
    }
}

void UCCIEngine::Pause()
{
    if (status == EngineThinking) {
        status = EnginePrepared;
        engineProcess->write("stop\n");
    }
}

void UCCIEngine::handleOutput()
{
    QString output;
    switch (status) {
    case EngineCreated:
        while (1) {
            output = engineProcess->readLine();
            if (output.isEmpty()) break;
            log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: ")) << output;
            if (output == "ucciok\n") {
                status = EnginePrepared;
                if (deferGo) {
                    Go();
                    deferGo = 0;
                }
                break;
            }
        }
        break;
    case EnginePrepared:
        output = engineProcess->readAll();
        log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: "));
        break;
    case EngineThinking:
        while (1) {
            output = engineProcess->readLine();
            if (output.isEmpty()) break;
            log() << (this->playerColor == Red ? QString("Red: ") : QString("Black: "))
                  << output;
            if (output.left(10) == "nobestmove") {
                board->Resign();
                return;
            }
            if (output.left(8) == "bestmove") {
                auto bestmoveList = output.split(QRegularExpression("[ \n]"));
                if (bestmoveList.size() > 1) {
                    for (int i = 0; i < bestmoveList.size(); ++i) {
                        if (bestmoveList[i] == "draw" && board->RequestDraw()) return;
                        if (bestmoveList[i] == "resign") {
                            board->Resign();
                            return;
                        }
                    }
                    handleShortMoveString(bestmoveList[1]);
                }
                status = EnginePrepared;
                break;
            }
        }
        break;
    }
}

void UCCIEngine::handleShortMoveString(const QString &moveString)
{
    int fromX = moveString[1].toLatin1() - '0';
    int fromY = moveString[0].toLatin1() - 'a';
    int toX = moveString[3].toLatin1() - '0';
    int toY = moveString[2].toLatin1() - 'a';
    auto fromPiece = board->GetPiece(fromX, fromY);
    auto toPiece = board->GetPiece(toX, toY);
    if (fromPiece != nullptr && toPiece != nullptr && !fromPiece->Invalid
            && (toPiece->Invalid || fromPiece->GetColor() != toPiece->GetColor()))
        board->Move(fromX, fromY, toX, toY);
}
