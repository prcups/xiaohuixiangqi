#include "mainwindow.h"

void BoardView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}

BoardView::BoardView()
{
    setAcceptDrops(true);
}

LogWindow::LogWindow(): edit(new QPlainTextEdit)
{
    edit->setMaximumBlockCount(200);
    setVisible(false);
    setWindowTitle(tr("日志"));
    setAllowedAreas(Qt::LeftDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetClosable);
    edit->setReadOnly(true);
    setWidget(edit.get());
}

void LogWindow::clear()
{
    edit->clear();
}

void LogWindow::onLogReceived(const QString &str)
{
    edit->insertPlainText(str);
    edit->ensureCursorVisible();
}

TimePanel::TimePanel()
{
    widget = new QWidget();
    auto layout = new QVBoxLayout();
    
    auto redGroup = new QGroupBox(tr("红方"));
    auto redLayout = new QVBoxLayout();
    redStepLabel = new QLabel(tr("单步：--"));
    redTotalLabel = new QLabel(tr("全局：--"));
    redLayout->addWidget(redStepLabel);
    redLayout->addWidget(redTotalLabel);
    redGroup->setLayout(redLayout);
    
    auto blackGroup = new QGroupBox(tr("黑方"));
    auto blackLayout = new QVBoxLayout();
    blackStepLabel = new QLabel(tr("单步：--"));
    blackTotalLabel = new QLabel(tr("全局：--"));
    blackLayout->addWidget(blackStepLabel);
    blackLayout->addWidget(blackTotalLabel);
    blackGroup->setLayout(blackLayout);
    
    layout->addWidget(redGroup);
    layout->addWidget(blackGroup);
    layout->addStretch();
    
    widget->setLayout(layout);
    setWidget(widget);
    setWindowTitle(tr("计时"));
    setAllowedAreas(Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::NoDockWidgetFeatures);
    
    updateTimer = new QTimer(this);
    updateTimer->setInterval(1000);
    connect(updateTimer, &QTimer::timeout, this, &TimePanel::UpdateTime);
}

void TimePanel::SetPlayers(Player *red, Player *black, Board *gameBoard)
{
    player[0] = red;
    player[1] = black;
    board = gameBoard;
    connect(board, &Board::BoardInfoChanged, this, &TimePanel::handleBoardInfoChanged);
}

void TimePanel::UpdateTime()
{
    if (player[0] && player[1]) {
        int redStep = player[0]->GetRemainingStepTime();
        int redTotal = player[0]->GetRemainingTotalTime();
        int blackStep = player[1]->GetRemainingStepTime();
        int blackTotal = player[1]->GetRemainingTotalTime();
        
        if (redStep >= 0)
            redStepLabel->setText(tr("单步：%1").arg(redStep));
        else
            redStepLabel->setText(tr("单步：--"));
        
        if (redTotal > 0)
            redTotalLabel->setText(tr("全局：%1").arg(redTotal));
        else
            redTotalLabel->setText(tr("全局：--"));
        
        if (blackStep >= 0)
            blackStepLabel->setText(tr("单步：%1").arg(blackStep));
        else
            blackStepLabel->setText(tr("单步：--"));
        
        if (blackTotal > 0)
            blackTotalLabel->setText(tr("全局：%1").arg(blackTotal));
        else
            blackTotalLabel->setText(tr("全局：--"));
    }
}

void TimePanel::handleBoardInfoChanged(const BoardInfo &info)
{
    if (info.endType != NotEnd || info.isPaused) {
        updateTimer->stop();
    } else {
        int curPlayerIdx = info.isBlack ? 1 : 0;
        if (player[curPlayerIdx] && player[curPlayerIdx]->GetRemainingStepTime() >= 0) {
            updateTimer->start();
        } else {
            updateTimer->stop();
        }
    }
    UpdateTime();
}

MainWindow::MainWindow()
{
    setWindowTitle(tr("小卉象棋"));
    setWindowIcon(QIcon(":/xiaohuixiangqi.png"));

    addDockWidget(Qt::LeftDockWidgetArea, &logWindow);
    addDockWidget(Qt::RightDockWidgetArea, &timePanel);
    connect(&log(), &Log::NewLogOutput, &logWindow, &LogWindow::onLogReceived);
    connect(&bar(), &Log::NewLogOutput, this, &MainWindow::onStatusUpdated);
    connect(&dialog(), &Log::NewLogOutput, this, &MainWindow::onDialogWanted);

    auto menubar = menuBar();

    auto gameMenu = menubar->addMenu(tr("对局"));
    gameMenu->addAction(tr("新建"), this, &MainWindow::onCreateTriggered);
    gameMenu->addAction(tr("查看日志"), this, &MainWindow::onShowLogTriggered);
    gameMenu->addSeparator();
    gameMenu->addAction(tr("退出"), this, &MainWindow::close);

    auto operationMenu = menubar->addMenu(tr("操作"));
    undo = operationMenu->addAction(tr("上一步"), this, &MainWindow::onUndoTriggered);
    undo->setDisabled(true);
    redo = operationMenu->addAction(tr("下一步"), this, &MainWindow::onRedoTriggered);
    redo->setDisabled(true);
    draw = operationMenu->addAction(tr("求和/接受求和"), this, &MainWindow::onDrawTriggered);
    draw->setDisabled(true);
    resign = operationMenu->addAction(tr("认输"), this, &MainWindow::onResignTriggered);
    resign->setDisabled(true);
    pause = operationMenu->addAction(tr("暂停"), this, &MainWindow::onPauseTriggered);
    pause->setDisabled(true);
    operationMenu->addSeparator();
    operationMenu->addAction(tr("设置"), this, &MainWindow::onSettingsTriggered);

    auto aboutMenu = menubar->addMenu(tr("关于"));
    aboutMenu->addAction(tr("关于"), this, &MainWindow::onAboutTriggered);
    aboutMenu->addAction(tr("关于Qt"), this, &MainWindow::onAboutQtTriggered);

    this->setMinimumSize(560, 640);
    this->setCentralWidget(&boardView);

    statusBar()->addWidget(&status);
}

MainWindow::~MainWindow()
{
    delete player[0];
    delete player[1];
    delete board;
}

void MainWindow::onDialogWanted(const QString &str)
{
    QMessageBox::information(this, "", str);
}

void MainWindow::onStatusUpdated(const QString &str)
{
    status.setText(str);
}

void MainWindow::onCreateTriggered()
{
    GameStartDialog gameStartDialog;
    if (!gameStartDialog.exec()) return;
    for (int i = 0; i < 2; ++i) {
        auto engineIndex = gameStartDialog.GetPlayerSelection(PieceColor(i));
        if (engineIndex == 0)
            player[i] = new Player(PieceColor(i));
        else {
            EngineType type = gameStartDialog.GetEngineType(engineIndex);

            int depth;
            switch (gameStartDialog.GetPlayerDiffSelection(PieceColor(i))) {
            case 0:
                depth = 5;
                break;
            case 1:
                depth = 10;
                break;
            case 2:
                depth = gameStartDialog.GetPlayerDepthSelection(PieceColor(i));
            }
            if (type.protocol == UCI)
                player[i] = new UCIEngine(PieceColor(i), type.path, depth);
            else
                player[i] = new UCCIEngine(PieceColor(i), type.path, depth);
        }
        player[i]->SetTimeSettings(gameStartDialog.GetTimeSettings(PieceColor(i)));
    }

    logWindow.clear();

    auto newBoard = new Board(player[0], player[1]);
    player[0]->SetBoard(newBoard);
    player[1]->SetBoard(newBoard);


    boardView.setScene(newBoard);
    delete board;
    board = newBoard;
    timePanel.SetPlayers(player[0], player[1], board);
    connect(board, &Board::BoardInfoChanged, this, &MainWindow::onBoardInfoChanged);

    if (player[0]->GetType() == Computer && player[1]->GetType() == Human) {
        boardView.rotate(180);
        board->Rotate(true);
    }

    board->Start();
}

void MainWindow::onAboutTriggered()
{
    QMessageBox::about(this, tr("关于"), tr("小卉象棋 \n一个简单的象棋界面"));
}

void MainWindow::onAboutQtTriggered()
{
    QMessageBox::aboutQt(this, tr("关于Qt"));
}

void MainWindow::onShowLogTriggered()
{
    logWindow.setVisible(true);
}

void MainWindow::onPauseTriggered()
{
    board->ChangePaused();
}

void MainWindow::onUndoTriggered()
{
    board->Undo();
}

void MainWindow::onRedoTriggered()
{
    board->Redo();
}

void MainWindow::onDrawTriggered()
{
    if (!board->RequestDraw())
        bar() << tr("请走一着棋，若对方不同意求和会继续行棋，否则对局以和结束");
}

void MainWindow::onResignTriggered()
{
    board->Resign();
}

void MainWindow::onSettingsTriggered()
{
    SettingsDialog settingsDialog;
    settingsDialog.exec();
}

void MainWindow::onBoardInfoChanged(const BoardInfo &info)
{
    if (settings.value("rotate", true).toBool()) {
        if (info.isHuman) {
            if (info.isBlack) {
                boardView.resetTransform();
                boardView.rotate(180);
                board->Rotate(true);
            } else {
                boardView.resetTransform();
                board->Rotate(false);
            }
        }
    }

    undo->setEnabled(info.hasPrev);
    redo->setEnabled(info.hasNext);

    auto canDrawAndResign = info.isHuman && info.endType == NotEnd
                            && !info.isPaused;
    draw->setEnabled(canDrawAndResign);
    resign->setEnabled(canDrawAndResign);

    if (info.endType) {
        pause->setDisabled(true);
        if (info.endType == BlackWin) bar() << tr("黑方胜利");
        else if (info.endType == Draw) bar() << tr("双方和棋");
        else bar() << tr("红方胜利");
        return;
    }

    if (info.ifJiangjun)
        bar() << (info.isBlack ? tr("红方") : tr("黑方")) + tr("将军");

    pause->setEnabled(true);
    if (info.isPaused) {
        bar() << tr("游戏已暂停");
        pause->setText(tr("继续"));
    } else pause->setText(tr("暂停"));
}
