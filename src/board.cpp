#include "board.h"

const QVector <QPair<int, int>> Board::jiangOffset = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
const QVector <QPair<int, int>> Board::maOffset = {{1, 2}, {-1, 2}, {1, -2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
const QVector <QPair<int, int>> Board::shiOffset = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
const QVector <QPair<int, int>> Board::xiangOffset = {{2, 2}, {2, -2}, {-2, 2}, {-2, -2}};
const QVector <QPair<int, int>> Board::zuOffset = {{1, 0}, {0, -1}, {0, 1}};

QRectF BoardBackground::boundingRect() const
{
    return QRectF(0, 0, 960, 1120);
}

BoardBackground::BoardBackground()
{
    auto rect = boundingRect();
    if (rect.height() / 9 > rect.width() / 8) {
        widthMargin = 60;
        heightMargin = (rect.height() - rect.width() / 8 * 9) / 2 + 60;
    } else {
        heightMargin = 60;
        widthMargin = (rect.width() - rect.height() / 9 * 8) / 2 + 60;
    }
    boardLeft = rect.x() + widthMargin;
    boardTop = rect.y() + heightMargin;
    boardRight = rect.x() + rect.width() - widthMargin;
    boardBottom = rect.y() + rect.height() - heightMargin;
    heightDis = (boardBottom - boardTop) / 9;
    widthDis = (boardRight - boardLeft) / 8;
}

void BoardBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(QColor(131, 203, 172, 127));
    painter->drawRect(boundingRect());
    painter->setBrush(Qt::transparent);

    painter->drawRect(boardLeft, boardTop, boardRight - boardLeft, boardBottom - boardTop);
    for (int i = 1; i < 9; ++i) {
        painter->drawLine(boardLeft, xToPosY(i), boardRight, xToPosY(i));
    }
    for (int i = 1; i < 8; ++i) {
        painter->drawLine(yToPosX(i), boardTop, yToPosX(i), xToPosY(4));
        painter->drawLine(yToPosX(i), xToPosY(5), yToPosX(i), boardBottom);
    }
    painter->drawLine(yToPosX(3), boardTop, yToPosX(5), xToPosY(2));
    painter->drawLine(yToPosX(5), boardTop, yToPosX(3), xToPosY(2));
    painter->drawLine(yToPosX(3), boardBottom, yToPosX(5), xToPosY(7));
    painter->drawLine(yToPosX(5), boardBottom, yToPosX(3), xToPosY(7));
    for (int i = 0; i < 5; ++i) {
        drawSoldierPos(painter, i * 2, 3);
        drawSoldierPos(painter, i * 2, 6);
    }
}

void BoardBackground::drawSoldierPos(QPainter *painter, int xPos, int yPos)
{
    if (xPos != 0) {
        painter->drawLine(yToPosX(xPos) - 16, xToPosY(yPos) - 6, yToPosX(xPos) - 6, xToPosY(yPos) - 6);
        painter->drawLine(yToPosX(xPos) - 6, xToPosY(yPos) - 6, yToPosX(xPos) - 6, xToPosY(yPos) - 16);
        painter->drawLine(yToPosX(xPos) - 16, xToPosY(yPos) + 6, yToPosX(xPos) - 6, xToPosY(yPos) + 6);
        painter->drawLine(yToPosX(xPos) - 6, xToPosY(yPos) + 6, yToPosX(xPos) - 6, xToPosY(yPos) + 16);
    }
    if (xPos != 8) {
        painter->drawLine(yToPosX(xPos) + 16, xToPosY(yPos) - 6, yToPosX(xPos) + 6, xToPosY(yPos) - 6);
        painter->drawLine(yToPosX(xPos) + 6, xToPosY(yPos) - 6, yToPosX(xPos) + 6, xToPosY(yPos) - 16);
        painter->drawLine(yToPosX(xPos) + 16, xToPosY(yPos) + 6, yToPosX(xPos) + 6, xToPosY(yPos) + 6);
        painter->drawLine(yToPosX(xPos) + 6, xToPosY(yPos) + 6, yToPosX(xPos) + 6, xToPosY(yPos) + 16);
    }
}

Frame::Frame(): QGraphicsRectItem(0, 0, 90, 90)
{
    setZValue(-1);
}

Board::Board(Player *red, Player *black)
{
    player[0] = red;
    player[1] = black;
    background = new BoardBackground;
    addItem(background);
    recordList.append(Record{
        .type = GameStart,
        .endType = NotEnd,
        .isBlack = true,
        .ifJiangjun = false,
        .ifEat = false,
        .requestDraw = false,
        .lastEat = 0,
        .fenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR"
    });
    initPieces(recordList[0].fenStr);
    curPlayerColor = Red;
    movePos = lastPos = lastEatPos = 0;
    isPaused = draw = 0;
}

void Board::Start()
{
    auto endType = (recordList[movePos].endType);
    emit BoardInfoChanged(BoardInfo{
        .endType = endType,
        .isBlack = (curPlayerColor == Black),
        .isHuman = (GetCurPlayer()->GetType() == Human),
        .ifJiangjun = (recordList[movePos].ifJiangjun),
        .hasPrev = (movePos != 0),
        .hasNext = (movePos != lastPos),
        .isPaused = isPaused,
        .rivalIsHuman = (player[curPlayerColor ^ 1]->GetType() == Human)
    });

    if (!isPaused && !endType)
        player[curPlayerColor]->Go();
}

Board::~Board() noexcept
{
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 9; ++j)
            delete content[i][j];
    delete background;
}

bool Board::initPieces(QStringView fenMain)
{
    int line = 9, column = 0;
    for (QChar t : fenMain) {
        if (line < 0) return false;
        auto color = t.isLower() ? Black : Red;
        switch (t.unicode()) {
        case 'r':
        case 'R':
            if (column > 8) return false;
            content[line][column] = new Piece(Che, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case 'n':
        case 'N':
            if (column > 8) return false;
            content[line][column] = new Piece(Ma, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case 'b':
        case 'B':
            if (column > 8) return false;
            content[line][column] = new Piece(Xiang, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case 'a':
        case 'A':
            if (column > 8) return false;
            content[line][column] = new Piece(Shi, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case 'k':
        case 'K':
            if (column > 8) return false;
            content[line][column] = new Piece(Jiang, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            player[color]->JiangPtr = content[line][column];
            ++column;
            break;
        case 'c':
        case 'C':
            if (column > 8) return false;
            content[line][column] = new Piece(Pao, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case 'p':
        case 'P':
            if (column > 8) return false;
            content[line][column] = new Piece(Zu, color, line, column);
            content[line][column]->setPos(yToPosX(column), xToPosY(line));
            addItem(content[line][column]);
            ++column;
            break;
        case '/':
            if (column != 9) return false;
            column = 0;
            --line;
            break;
        case '0'...'9':
            for (int i = t.unicode() - '0'; i > 0; --i) {
                if (column > 8) return false;
                content[line][column] = new Piece(line, column);
                content[line][column]->setPos(yToPosX(column), xToPosY(line));
                addItem(content[line][column]);
                ++column;
            }
            break;
        default:
            return false;
        }
    }
    if (line != -1) return false;
    return true;
}

QString Board::toShortFenStr()
{
    QString fen;
    short invalidCount = 0;
    for (int i = 9; i >= 0; --i)
        for (int j = 0; j < 9; ++j) {
            if (content[i][j]->Invalid)
                ++invalidCount;
            else {
                if (invalidCount != 0) fen.append(QString::number(invalidCount));
                invalidCount = 0;
                if (content[i][j]->GetColor() == Black) {
                    switch (content[i][j]->GetType()) {
                    case Che:
                        fen.append('r');
                        break;
                    case Pao:
                        fen.append('c');
                        break;
                    case Ma:
                        fen.append('n');
                        break;
                    case Zu:
                        fen.append('p');
                        break;
                    case Xiang:
                        fen.append('b');
                        break;
                    case Shi:
                        fen.append('a');
                        break;
                    case Jiang:
                        fen.append('k');
                    }
                } else {
                    switch (content[i][j]->GetType()) {
                    case Che:
                        fen.append('R');
                        break;
                    case Pao:
                        fen.append('C');
                        break;
                    case Ma:
                        fen.append('N');
                        break;
                    case Zu:
                        fen.append('P');
                        break;
                    case Xiang:
                        fen.append('B');
                        break;
                    case Shi:
                        fen.append('A');
                        break;
                    case Jiang:
                        fen.append('K');
                    }
                }
            }
            if (j == 8) {
                if (invalidCount != 0) fen.append(QString::number(invalidCount));
                invalidCount = 0;
                if (i != 0) fen.append('/');
            }
        }
    return fen;
}

QString Board::ToFenString()
{
    if (fenCache.isEmpty()) {
        fenCache = (recordList[lastEatPos].fenStr);
        bool isBlack = (recordList[lastEatPos].isBlack);
        if (isBlack) fenCache.append(" w");
        else fenCache.append(" b");
        fenCache.append(" - - 0 1");
        if (lastEatPos != movePos)
            fenCache.append(" moves");
        for (int i = lastEatPos + 1; i <= movePos; ++i) {
            fenCache.append(" ");
            fenCache.append(recordList[i].ToMoveString());
        }
        fenCachePos = movePos + 1;
    } else {
        if (fenCachePos == lastEatPos + 1)
            fenCache.append(" moves");
        for (; fenCachePos <= movePos; ++fenCachePos) {
            fenCache.append(" ");
            fenCache.append(recordList[fenCachePos].ToMoveString());
        }
    }

    return fenCache;
}

float Board::xToPosY(int yPos)
{
    return background->xToPosY(9 - yPos) - 45;
}

float Board::yToPosX(int xPos)
{
    return background->yToPosX(xPos) - 45;
}

void Board::prepareNextMove()
{
    draw = 0;
    player[curPlayerColor]->MoveCompleted();
    curPlayerColor ^= 1;
    bar() << tr("就绪");
    Start();
}

void Board::SetMovable(bool allowMove)
{
    if (allowMove)
        status = BoardPrepared;
    else
        status = BoardBanned;
}

void Board::Rotate(bool ok)
{
    for (int i = 0; i <= 9; ++i)
        for (int j = 0; j <= 8; ++j)
            content[i][j]->setRotation(ok ? 180 : 0);
}

void Board::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (status == BoardBanned) return;
    auto pos = event->lastScenePos();
    if (!items(pos).isEmpty()) {
        auto clickedPiece =  dynamic_cast<Piece *>(items(pos).first());
        if (clickedPiece == nullptr || clickedPiece->Invalid
                || clickedPiece->GetColor() != curPlayerColor) return;
        selectedPiece = clickedPiece;
        status = PieceSelected;
        focusFrame.setPos(yToPosX(selectedPiece->Y), xToPosY(selectedPiece->X));
        if (focusFrame.scene() != this)
            addItem(&focusFrame);
    }
}

void Board::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
            .length() < QApplication::startDragDistance()) {
        return;
    }

    if (status != PieceSelected) return;
    auto pos = event->lastScenePos();
    if (selectedPiece->contains(selectedPiece->mapFromScene(pos))) {
        QDrag *drag = new QDrag(event->widget());
        QMimeData *mime = new QMimeData;
        mime->setText("xhxq");
        drag->setMimeData(mime);
        drag->setPixmap(selectedPiece->GetPixmap());
        drag->exec();
    }
}

void Board::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->text() != "xhxq") return;
}

void Board::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    auto pos = event->scenePos();
    handlePutEvent(pos);
}

void Board::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    auto pos = event->lastScenePos();
    handlePutEvent(pos);
}

void Board::handlePutEvent(QPointF &pos)
{
    if (status != PieceSelected) return;
    if (!items(pos).isEmpty()) {
        auto clickedPiece =  dynamic_cast<Piece *>(items(pos).first());
        if (clickedPiece == nullptr
                || (clickedPiece->Invalid == 0
                    && clickedPiece->GetColor() == selectedPiece->GetColor())) return;
        if (Move(selectedPiece->X, selectedPiece->Y, clickedPiece->X, clickedPiece->Y)) {
            SetMovable(false);
            removeItem(&focusFrame);
        }
    }
}

Record Board::getRecord(int fromX, int fromY, int toX, int toY)
{
    Record record = {
        .type = NormalMove,
        .fromX = fromX,
        .fromY = fromY,
        .toX = toX,
        .toY = toY,
        .endType = NotEnd,
        .isBlack = (curPlayerColor == Black),
        .ifEat = false,
        .requestDraw = draw,
        .lastEat = lastEatPos
    };

    if (!content[toX][toY]->Invalid) {
        record.ifEat = true;
        record.dstType = content[toX][toY]->GetType();
        record.dstColor = content[toX][toY]->GetColor();
        record.lastEat = movePos;
    }

    if (movePos - record.lastEat >= 100) record.endType = Draw;

    auto tempPiece = content[toX][toY];
    content[toX][toY] = content[fromX][fromY];
    content[toX][toY]->X = toX;
    content[toX][toY]->Y = toY;
    content[fromX][fromY] = new Piece(fromX, fromY);

    record.ifJiangjun = judgeJiangjun(PieceColor(curPlayerColor ^ 1));
    if (!judgePossibleToMove(PieceColor(curPlayerColor ^ 1)))
        record.endType = (curPlayerColor == Red ? RedWin : BlackWin);
    record.fenStr = toShortFenStr();

    delete content[fromX][fromY];
    content[fromX][fromY] = content[toX][toY];
    content[fromX][fromY]->X = fromX;
    content[fromX][fromY]->Y = fromY;
    content[toX][toY] = tempPiece;

    return record;
}

bool Board::Move(int fromX, int fromY, int toX, int toY)
{
    if (!judgeMove(fromX, fromY, toX, toY))
        return false;

    Record record = getRecord(fromX, fromY, toX, toY);

    if (recordMap.isEmpty())
        for (int i = lastEatPos; i <= movePos; ++i)
            ++recordMap[recordList[i]];

    int &c = recordMap[record];
    if (c == 2 && record.ifJiangjun) {
        bar() << tr("不允许长将");
        return false;
    }
    ++c;

    oldFrame.setPos(yToPosX(fromY), xToPosY(fromX));
    if (oldFrame.scene() != this)
        addItem(&oldFrame);
    newFrame.setPos(yToPosX(toY), xToPosY(toX));
    if (newFrame.scene() != this)
        addItem(&newFrame);

    removeItem(content[toX][toY]);
    delete content[toX][toY];
    content[toX][toY] = content[fromX][fromY];
    content[toX][toY]->X = toX;
    content[toX][toY]->Y = toY;
    if (content[toX][toY]->GetType() == Jiang)
        player[content[toX][toY]->GetColor()]->JiangPtr = content[toX][toY];
    content[fromX][fromY] = new Piece(fromX, fromY);
    addItem(content[fromX][fromY]);
    content[fromX][fromY]->setPos(yToPosX(fromY), xToPosY(fromX));
    content[fromX][fromY]->X = fromX;
    content[fromX][fromY]->Y = fromY;

    if (settings.value("animation", true).toBool()) {
        QPropertyAnimation *animation = new QPropertyAnimation(content[toX][toY], "pos");
        animation->setDuration(100);
        animation->setEndValue(QPointF(yToPosX(toY), xToPosY(toX)));
        animation->start();
    } else content[toX][toY]->setPos(yToPosX(toY), xToPosY(toX));

    lastEatPos = record.lastEat;
    if (record.ifEat) {
        fenCache.clear();
        recordMap.clear();
    }

    lastPos = ++movePos;
    if (movePos >= recordList.size())
        recordList.append(record);
    else recordList[movePos] = record;

    if (!record.endType)
        QTimer::singleShot(200, this, &Board::prepareNextMove);
    else {
        if (record.endType == BlackWin) dialog() << tr("黑方胜利");
        else if (record.endType == Draw) dialog() << tr("双方和棋");
        else dialog() << tr("红方胜利");
        QTimer::singleShot(200, this, &Board::Start);
    }
    return true;
}

Player *Board::GetCurPlayer()
{
    return player[curPlayerColor];
}

Player *Board::GetRivalPlayer()
{
    return player[curPlayerColor ^ 1];
}

bool Board::judgeMove(int fromX, int fromY, int toX, int toY)
{
    switch (content[fromX][fromY]->GetType()) {
    case Che:
        if (!judgeChe(fromX, fromY, toX, toY)) return false;
        break;
    case Ma:
        if (!judgeMa(fromX, fromY, toX, toY)) return false;
        break;
    case Pao:
        if (!judgePao(fromX, fromY, toX, toY)) return false;
        break;
    case Zu:
        if (!judgeZu(fromX, fromY, toX, toY)) return false;
        break;
    case Xiang:
        if (!judgeXiang(fromX, fromY, toX, toY)) return false;
        break;
    case Shi:
        if (!judgeShi(fromX, fromY, toX, toY)) return false;
        break;
    case Jiang:
        if (!judgeJiang(fromX, fromY, toX, toY)) return false;
    }

    if (judgeMoveToJiangjun(fromX, fromY, toX, toY, PieceColor(curPlayerColor))) return false;
    return true;
}

bool Board::judgeMoveToJiangjun(int fromX, int fromY, int toX, int toY, PieceColor color)
{
    auto tempPiece = content[toX][toY];
    content[toX][toY] = content[fromX][fromY];
    content[toX][toY]->X = toX;
    content[toX][toY]->Y = toY;
    content[fromX][fromY] = new Piece(fromX, fromY);
    auto jiangJun = judgeJiangjun(color);
    delete content[fromX][fromY];
    content[fromX][fromY] = content[toX][toY];
    content[fromX][fromY]->X = fromX;
    content[fromX][fromY]->Y = fromY;
    content[toX][toY] = tempPiece;
    if (jiangJun) return true;
    else return false;
}

bool Board::judgePossibleToMove(PieceColor color)
{
    int dstX, dstY;
    for (int i = 0; i <= 9; ++i)
        for (int j = 0; j <= 8; ++j) {
            if (content[i][j]->Invalid == 0 && content[i][j]->GetColor() == color) {
                switch (content[i][j]->GetType()) {
                case Che:
                case Pao:
                    for (int t = i - 1; t >= 0; --t) {
                        if (content[t][j]->Invalid) {
                            if (!judgeMoveToJiangjun(i, j, t, j, color))
                                return true;
                        } else {
                            if (content[t][j]->GetColor() != color) {
                                if (content[i][j]->GetType() == Pao) {
                                    for (int k = t - 1; k >= 0; --k) {
                                        if (!content[k][j]->Invalid && content[k][j]->GetColor() != color) {
                                            if (!judgeMoveToJiangjun(i, j, k, j, color))
                                                return true;
                                            break;
                                        }
                                    }
                                } else if (!judgeMoveToJiangjun(i, j, t, j, color))
                                    return true;
                                break;
                            }
                            break;
                        }
                    }
                    for (int t = i + 1; t <= 9; ++t) {
                        if (content[t][j]->Invalid) {
                            if (!judgeMoveToJiangjun(i, j, t, j, color))
                                return true;
                        } else {
                            if (content[t][j]->GetColor() != color) {
                                if (content[i][j]->GetType() == Pao) {
                                    for (int k = t + 1; k <= 9; ++k) {
                                        if (!content[k][j]->Invalid && content[k][j]->GetColor() != color) {
                                            if (!judgeMoveToJiangjun(i, j, k, j, color))
                                                return true;
                                            break;
                                        }
                                    }
                                } else if (!judgeMoveToJiangjun(i, j, t, j, color))
                                    return true;
                            }
                            break;
                        }
                    }
                    for (int t = j - 1; t >= 0; --t) {
                        if (content[i][t]->Invalid) {
                            if (!judgeMoveToJiangjun(i, j, i, t, color))
                                return true;
                        } else {
                            if (content[i][t]->GetColor() != color) {
                                if (content[i][j]->GetType() == Pao) {
                                    for (int k = t - 1; k >= 0; --k) {
                                        if (!content[i][k]->Invalid && content[i][k]->GetColor() != color) {
                                            if (!judgeMoveToJiangjun(i, j, i, k, color))
                                                return true;
                                            break;
                                        }
                                    }
                                } else if (!judgeMoveToJiangjun(i, j, i, t, color))
                                    return true;
                            }
                            break;
                        }
                    }
                    for (int t = j + 1; t <= 8; ++t) {
                        if (content[i][t]->Invalid) {
                            if (!judgeMoveToJiangjun(i, j, i, t, color))
                                return true;
                        } else {
                            if (content[i][t]->GetColor() != color) {
                                if (content[i][j]->GetType() == Pao) {
                                    for (int k = t + 1; k <= 8; ++k) {
                                        if (!content[i][k]->Invalid && content[i][k]->GetColor() != color) {
                                            if (!judgeMoveToJiangjun(i, j, i, k, color))
                                                return true;
                                            break;
                                        }
                                    }
                                } else if (!judgeMoveToJiangjun(i, j, i, t, color))
                                    return true;
                            }
                            break;
                        }
                    }
                    break;
                case Ma:
                    for (auto offset : maOffset) {
                        dstX = i + offset.first;
                        if (dstX < 0) continue;
                        if (dstX > 9) continue;
                        dstY = j + offset.second;
                        if (dstY < 0) continue;
                        if (dstY > 8) continue;
                        if (!content[dstX][dstY]->Invalid
                                && content[dstX][dstY]->GetColor() == color) continue;
                        if (content[i + offset.first / 2][j + offset.second / 2]->Invalid && !judgeMoveToJiangjun(i, j, dstX, dstY, color))
                            return true;
                    }
                    break;
                case Zu:
                    if (color == Red) {
                        if (i < 5) {
                            if ((content[i + 1][j]->Invalid || content[i + 1][j]->GetColor() == Black)
                                    && !judgeMoveToJiangjun(i, j, i + 1, j, color))
                                return true;
                        } else {
                            for (auto offset : zuOffset) {
                                dstX = i + offset.first;
                                if (dstX < 0) continue;
                                dstY = j + offset.second;
                                if (dstY < 0) continue;
                                if (dstY > 8) continue;
                                if (!content[dstX][dstY]->Invalid
                                        && content[dstX][dstY]->GetColor() == color) continue;
                                if (!judgeMoveToJiangjun(i, j, dstX, dstY, color))
                                    return true;
                            }
                        }
                    } else {
                        if (i > 4) {
                            if ((content[i - 1][j]->Invalid || content[i - 1][j]->GetColor() == Red)
                                    && !judgeMoveToJiangjun(i, j, i - 1, j, color))
                                return true;
                        } else {
                            for (auto offset : zuOffset) {
                                dstX = i - offset.first;
                                if (dstX < 0) continue;
                                dstY = j - offset.second;
                                if (dstY < 0) continue;
                                if (dstY > 8) continue;
                                if (!content[dstX][dstY]->Invalid
                                        && content[dstX][dstY]->GetColor() == color) continue;
                                if (!judgeMoveToJiangjun(i, j, dstX, dstY, color))
                                    return true;
                            }
                        }
                    }
                    break;
                case Xiang:
                    for (auto offset : xiangOffset) {
                        dstX = i + offset.first;
                        if (dstX < (color == Red ? 0 : 5)) continue;
                        if (dstX > (color == Red ? 4 : 9)) continue;
                        dstY = j + offset.second;
                        if (dstY < 0) continue;
                        if (dstY > 8) continue;
                        if (!content[dstX][dstY]->Invalid
                                && content[dstX][dstY]->GetColor() == color) continue;
                        if (content[i + offset.first / 2][j + offset.second / 2]->Invalid && !judgeMoveToJiangjun(i, j, dstX, dstY, color))
                            return true;
                    }
                    break;
                case Shi:
                    for (auto offset : shiOffset) {
                        dstX = i + offset.first;
                        if (dstX < (color == Red ? 0 : 7)) continue;
                        if (dstX > (color == Red ? 2 : 9)) continue;
                        dstY = j + offset.second;
                        if (dstY < 3) continue;
                        if (dstY > 5) continue;
                        if (!content[dstX][dstY]->Invalid
                                && content[dstX][dstY]->GetColor() == color) continue;
                        if (!judgeMoveToJiangjun(i, j, dstX, dstY, color))
                            return true;
                    }
                    break;
                case Jiang:
                    for (auto offset : jiangOffset) {
                        dstX = i + offset.first;
                        if (dstX < (color == Red ? 0 : 7)) continue;
                        if (dstX > (color == Red ? 2 : 9)) continue;
                        dstY = j + offset.second;
                        if (dstY < 3) continue;
                        if (dstY > 5) continue;
                        if (!content[dstX][dstY]->Invalid
                                && content[dstX][dstY]->GetColor() == color) continue;
                        if (!judgeMoveToJiangjun(i, j, dstX, dstY, color))
                            return true;
                    }
                }
            }
        }
    return false;
}

bool Board::judgeChe(int fromX, int fromY, int toX, int toY)
{
    if (fromX == toX) {
        int tmin = std::min(fromY, toY),
            tmax = std::max(fromY, toY);
        for (int i = tmin + 1; i < tmax; ++i)
            if (!content[fromX][i]->Invalid) return false;
        return true;
    } else if (fromY == toY) {
        int tmin = std::min(fromX, toX),
            tmax = std::max(fromX, toX);
        for (int i = tmin + 1; i < tmax; ++i)
            if (!content[i][fromY]->Invalid) return false;
        return true;
    } else return false;
}

bool Board::judgeJiang(int fromX, int fromY, int toX, int toY)
{
    if (curPlayerColor == Red && toX > 2) return false;
    if (curPlayerColor == Black && toX < 7) return false;
    if (toY < 3 || toY > 5) return false;
    if (jiangOffset.contains({toX - fromX, toY - fromY})) return true;
    return false;
}

bool Board::judgeMa(int fromX, int fromY, int toX, int toY)
{
    auto xOffset = toX - fromX;
    auto yOffset = toY - fromY;
    auto idx = maOffset.indexOf({xOffset, yOffset});
    if (idx != -1 &&
            content[fromX + xOffset / 2][fromY + yOffset / 2]->Invalid)
        return true;
    return false;
}

bool Board::judgeZu(int fromX, int fromY, int toX, int toY)
{
    bool isGuohezu = false;
    QPair<int, int> offset;
    if (curPlayerColor == Red) {
        if (fromX > 4) isGuohezu = true;
        offset = {toX - fromX, toY - fromY};
    } else {
        if (fromX < 5) isGuohezu = true;
        offset = {fromX - toX, fromY - toY};
    }
    if (!isGuohezu) {
        if (offset == QPair<int, int> {1, 0}) return true;
    } else if (zuOffset.contains(offset)) return true;
    return false;
}

bool Board::judgePao(int fromX, int fromY, int toX, int toY)
{
    int count = 0;
    if (fromX == toX) {
        int tmin = std::min(fromY, toY),
            tmax = std::max(fromY, toY);
        for (int i = tmin + 1; i < tmax; ++i)
            if (!content[fromX][i]->Invalid) ++count;
    } else if (fromY == toY) {
        int tmin = std::min(fromX, toX),
            tmax = std::max(fromX, toX);
        for (int i = tmin + 1; i < tmax; ++i)
            if (!content[i][fromY]->Invalid) ++count;
    } else return false;
    if ((count == 0 && content[toX][toY]->Invalid)
            || (count == 1 && (!content[toX][toY]->Invalid))
       ) return true;
    return false;
}

bool Board::judgeShi(int fromX, int fromY, int toX, int toY)
{
    if (curPlayerColor == Red && toX > 2) return false;
    if (curPlayerColor == Black && toX < 7) return false;
    if (toY < 3 || toY > 5) return false;
    if (shiOffset.contains({toX - fromX, toY - fromY})) return true;
    return false;
}

bool Board::judgeXiang(int fromX, int fromY, int toX, int toY)
{
    if (curPlayerColor == Red && toX > 4) return false;
    if (curPlayerColor == Black && toX < 5) return false;
    auto xOffset = toX - fromX;
    auto yOffset = toY - fromY;
    auto idx = xiangOffset.indexOf({xOffset, yOffset});
    if (idx != -1 &&
            content[fromX + xOffset / 2][fromY + yOffset / 2]->Invalid)
        return true;
    return false;
}

bool Board::judgeJiangjun(PieceColor color)
{
    auto jiangPtr = player[color]->JiangPtr;
    auto x = jiangPtr->X;
    auto y = jiangPtr->Y;
    bool paoMode = 0;
    for (int t = x - 1; t >= 0; --t) {
        if (!content[t][y]->Invalid) {
            if (content[t][y]->GetColor() != color) {
                if (paoMode) {
                    if (content[t][y]->GetType() == Pao) return true;
                } else {
                    if (content[t][y]->GetType() == Jiang
                            || content[t][y]->GetType() == Che) return true;
                }
            }
            if (paoMode == 0) paoMode = 1;
            else break;
        }
    }

    paoMode = 0;
    for (int t = x + 1; t <= 9; ++t) {
        if (!content[t][y]->Invalid) {
            if (content[t][y]->GetColor() != color) {
                if (paoMode) {
                    if (content[t][y]->GetType() == Pao) return true;
                } else {
                    if (content[t][y]->GetType() == Jiang
                            || content[t][y]->GetType() == Che) return true;
                }
            }
            if (paoMode == 0) paoMode = 1;
            else break;
        }
    }

    paoMode = 0;
    for (int t = y - 1; t >= 0; --t) {
        if (!content[x][t]->Invalid) {
            if (content[x][t]->GetColor() != color) {
                if (paoMode) {
                    if (content[x][t]->GetType() == Pao) return true;
                } else {
                    if (content[x][t]->GetType() == Jiang
                            || content[x][t]->GetType() == Che) return true;
                }
            }
            if (paoMode == 0) paoMode = 1;
            else break;
        }
    }

    paoMode = 0;
    for (int t = y + 1; t <= 8; ++t) {
        if (!content[x][t]->Invalid) {
            if (content[x][t]->GetColor() != color) {
                if (paoMode) {
                    if (content[x][t]->GetType() == Pao) return true;
                } else {
                    if (content[x][t]->GetType() == Jiang
                            || content[x][t]->GetType() == Che) return true;
                }
            }
            if (paoMode == 0) paoMode = 1;
            else break;
        }
    }

    Piece *dst;
    for (auto &i : zuOffset) {
        if (color == Red) dst = content[x + i.first][y + i.second];
        else dst = content[x - i.first][y - i.second];
        if (!dst->Invalid && dst->GetType() == Zu && dst->GetColor() != color) return true;
    }

    for (auto &i : maOffset) {
        if (x + i.first < 0) continue;
        if (x + i.first > 9) continue;
        dst = content[x + i.first][y + i.second];
        if (!dst->Invalid && dst->GetType() == Ma && dst->GetColor() != color
                && content[dst->X - i.first / 2][dst->Y - i.second / 2]->Invalid
           ) return true;
    }
    return false;
}

Piece *Board::GetPiece(int x, int y)
{
    if (x < 0) return nullptr;
    if (x > 9) return nullptr;
    if (y < 0) return nullptr;
    if (y > 8) return nullptr;
    return content[x][y];
}

void Board::switchToMove(int to)
{
    for (int i = 0; i <= 9; ++i)
        for (int j = 0; j <= 8; ++j) {
            removeItem(content[i][j]);
            delete (content[i][j]);
        }
    if (focusFrame.scene() == this) removeItem(&focusFrame);
    if (to > lastPos) to = lastPos;
    initPieces(recordList[to].fenStr);
    movePos = to;
    oldFrame.setPos(yToPosX(recordList[to].fromY), xToPosY(recordList[to].fromX));
    if (oldFrame.scene() != this)
        addItem(&oldFrame);
    newFrame.setPos(yToPosX(recordList[to].toY), xToPosY(recordList[to].toX));
    if (newFrame.scene() != this)
        addItem(&newFrame);
    curPlayerColor = recordList[to].isBlack ? Red : Black;
    lastEatPos = recordList[to].lastEat;
    draw = recordList[to].requestDraw;
    fenCache.clear();
    recordMap.clear();
}

void Board::doPause()
{
    isPaused = true;
    player[0]->Pause();
    player[1]->Pause();
}

void Board::ChangePaused()
{
    if (!isPaused) {
        doPause();
    } else {
        isPaused = false;
        player[curPlayerColor]->ResumeTimer();
    }
    Start();
}

void Board::Undo()
{
    doPause();
    player[0]->ClearTimeLimits();
    player[1]->ClearTimeLimits();
    switchToMove(movePos - 1);
    Start();
}

void Board::Redo()
{
    doPause();
    player[0]->ClearTimeLimits();
    player[1]->ClearTimeLimits();
    switchToMove(movePos + 1);
    Start();
}

void Board::handleAbnormalEnd(EndType type)
{
    status = BoardBanned;
    if (focusFrame.scene() == this) removeItem(&focusFrame);
    player[0]->ClearTimeLimits();
    player[1]->ClearTimeLimits();
    ++movePos;
    Record record = {
        .type = EndResign,
        .endType = type,
        .isBlack = (curPlayerColor == Black),
        .ifJiangjun = recordList[movePos - 1].ifJiangjun,
        .ifEat = false,
        .requestDraw = false,
        .lastEat = lastEatPos,
        .fenStr = recordList[movePos - 1].fenStr
    };
    if (movePos >= recordList.size())
        recordList.append(record);
    else recordList[movePos] = record;
    lastPos = movePos;
    Start();
}

void Board::Resign()
{
    handleAbnormalEnd(curPlayerColor == Red ? BlackWin : RedWin);
    dialog() << (curPlayerColor == Red ? tr("红方") : tr("黑方")) + tr("认输");
}

bool Board::GetDraw()
{
    return recordList[movePos].requestDraw;
}

bool Board::RequestDraw()
{
    if (GetDraw()) {
        handleAbnormalEnd(Draw);
        dialog() << "双方和棋";
        return true;
    } else {
        draw = true;
        return false;
    }
}
