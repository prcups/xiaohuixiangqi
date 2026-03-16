#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>
#include "piece.h"
#include "log.h"

struct TimeSettings
{
    int stepTime;
    int totalTime;
};

enum PlayerType
{
    Human, Computer
};

enum EngineStatus
{
    EngineCreated, EnginePrepared, EngineThinking
};

class Board;

/**
 * @todo write docs
 */
class Player : public QObject
{
    Q_OBJECT

protected:
    PieceColor playerColor;
    Board *board = nullptr;
    PlayerType type;
    int stepTimeLimit;
    int totalTimeLimit;
    int remainingTotalTime;
    int remainingStepTime;
    QTimer *stepTimer;

protected slots:
    void onStepTimeout();

public:
    virtual ~Player();

    /**
     * Default constructor
     */
    Player(PieceColor color);
    void SetBoard(Board *newBoard);
    PieceColor GetColor();
    PlayerType GetType();
    virtual void Go();
    virtual void Pause();
    void SetTimeSettings(const TimeSettings &settings);
    int GetRemainingTotalTime() const;
    int GetRemainingStepTime() const;
    int GetRemainingStepTimeLimit() const;
    int GetRemainingTotalTimeLimit() const;
    void MoveCompleted();
    void ResumeTimer();
    void ClearTimeLimits();
    Piece *JiangPtr;
};

class UCIEngine : public Player
{
    Q_OBJECT

    QProcess *engineProcess;
    EngineStatus status = EngineCreated;
    bool deferGo = 0;
    int depth;

private slots:
    void handleOutput();
    void handleShortMoveString(const QString & moveString);

public:
    UCIEngine(PieceColor color, QString enginePath, int depth);
    ~UCIEngine();
    void Go() override;
    void Pause() override;
};

class UCCIEngine : public Player
{
    Q_OBJECT

    QProcess *engineProcess;
    EngineStatus status = EngineCreated;
    bool deferGo = 0;
    int depth;

private slots:
    void handleOutput();
    void handleShortMoveString(const QString & moveString);

public:
    UCCIEngine(PieceColor color, QString enginePath, int depth);
    ~UCCIEngine();
    void Go() override;
    void Pause() override;
};

#endif // PLAYER_H
