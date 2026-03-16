#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QGraphicsView>
#include <QPointer>
#include <QMessageBox>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QLabel>
#include <QTransform>
#include <QVBoxLayout>
#include <QGroupBox>

#include "board.h"
#include "gamestartdialog.h"
#include "log.h"
#include "settingsdialog.h"

class BoardView : public QGraphicsView
{
    void resizeEvent ( QResizeEvent * event ) override;
public:
    BoardView();
};

class LogWindow: public QDockWidget
{
    Q_OBJECT

    QScopedPointer <QPlainTextEdit> edit;
public:
    LogWindow();
    void clear();
public slots:
    void onLogReceived(const QString & str);
};

class TimePanel : public QDockWidget
{
    Q_OBJECT

    QWidget *widget;
    QLabel *redStepLabel;
    QLabel *redTotalLabel;
    QLabel *blackStepLabel;
    QLabel *blackTotalLabel;
    QPointer <Player> player[2];
    QPointer <Board> board;
    QTimer *updateTimer;
public:
    TimePanel();
    void SetPlayers(Player *red, Player *black, Board *gameBoard);
    void UpdateTime();
public slots:
    void handleBoardInfoChanged(const BoardInfo &info);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    LogWindow logWindow;
    TimePanel timePanel;
    QPointer <Board> board;
    QPointer <Player> player[2];
    BoardView boardView;
    QLabel status;
    QAction *undo, *redo, *draw, *resign, *pause;
    QSettings settings;
private slots:
    void onStatusUpdated(const QString & str);
    void onDialogWanted(const QString & str);
    void onCreateTriggered();
    void onAboutTriggered();
    void onAboutQtTriggered();
    void onShowLogTriggered();
    void onPauseTriggered();
    void onUndoTriggered();
    void onRedoTriggered();
    void onDrawTriggered();
    void onResignTriggered();
    void onSettingsTriggered();
    void onBoardInfoChanged(const BoardInfo& info);
public:
    /**
     * Default constructor
     */
    MainWindow();

    /**
     * Destructor
     */
    ~MainWindow();

};

#endif // MAINWINDOW_H
