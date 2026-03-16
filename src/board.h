#ifndef BOARD_H
#define BOARD_H

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QStringView>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVector>
#include <QPair>
#include <QList>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include "piece.h"
#include "player.h"
#include "log.h"
#include "record.h"

class Board;
class Player;

struct BoardInfo
{
    int endType;
    bool isBlack;
    bool isHuman;
    bool ifJiangjun;
    bool hasPrev;
    bool hasNext;
    bool isPaused;
    bool rivalIsHuman;
};
Q_DECLARE_METATYPE(BoardInfo)

class BoardBackground : public QGraphicsItem
{
    float boardLeft, boardTop, boardBottom, boardRight,
          heightDis, widthDis, heightMargin, widthMargin;
    void drawSoldierPos(QPainter *painter, int xPos, int yPos);
    QRectF boundingRect() const override;
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) override;
    float yToPosX(int xPos)
    {
        return boardLeft + xPos * widthDis;
    }
    float xToPosY(int yPos)
    {
        return boardTop + yPos * heightDis;
    }
public:
    friend Board;
    BoardBackground();
};

class Frame : public QGraphicsRectItem
{
public:
    Frame();
};

enum BoardStatus
{
    BoardBanned, BoardPrepared, PieceSelected
};

class Board : public QGraphicsScene
{
    Q_OBJECT
    QPointer <Piece> content[10][9];
    BoardBackground* background;
    Player *player[2];
    int curPlayerColor;
    BoardStatus status = BoardBanned;
    Piece *selectedPiece;
    int movePos, lastPos, lastEatPos, fenCachePos;
    bool isPaused, draw;
    Frame focusFrame, oldFrame, newFrame;
    QString fenCache;
    QList <Record> recordList;
    QHash <Record, int> recordMap;
    QSettings settings;

    static const QVector <QPair<int, int>> jiangOffset;
    static const QVector <QPair<int, int>> maOffset;
    static const QVector <QPair<int, int>> shiOffset;
    static const QVector <QPair<int, int>> xiangOffset;
    static const QVector <QPair<int, int>> zuOffset;

    bool initPieces(QStringView fenMain);
    float yToPosX(int xPos);
    float xToPosY(int yPos);
    QString toShortFenStr();
    void doPause();
    Record getRecord(int fromX, int fromY, int toX, int toY);
    void switchToMove(int to);
    void handleAbnormalEnd(EndType type);

    void handlePutEvent(QPointF & pos);
    void mousePressEvent ( QGraphicsSceneMouseEvent * event ) override;
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) override;
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

    bool judgeMove(int fromX, int fromY, int toX, int toY);
    bool judgeChe(int fromX, int fromY, int toX, int toY);
    bool judgeMa(int fromX, int fromY, int toX, int toY);
    bool judgePao(int fromX, int fromY, int toX, int toY);
    bool judgeZu(int fromX, int fromY, int toX, int toY);
    bool judgeXiang(int fromX, int fromY, int toX, int toY);
    bool judgeShi(int fromX, int fromY, int toX, int toY);
    bool judgeJiang(int fromX, int fromY, int toX, int toY);
    bool judgeJiangjun(PieceColor color);
    bool judgeMoveToJiangjun(int fromX, int fromY, int toX, int toY, PieceColor color);
    bool judgePossibleToMove(PieceColor color);

private slots:
    void prepareNextMove();

public:
    Board(Player *red, Player *black);
    ~Board() noexcept;
    void Start();
    void Rotate(bool ok);
    Player* GetCurPlayer();
    Player* GetRivalPlayer();
    void SetMovable(bool allowMove);
    Piece* GetPiece(int x, int y);
    bool Move(int fromX, int fromY, int toX, int toY);
    QString ToFenString();
    void ChangePaused();
    void Undo();
    void Redo();
    void Resign();
    bool GetDraw();
    bool RequestDraw();

signals:
    void BoardInfoChanged(const BoardInfo &info);
};

#endif // BOARD_H
