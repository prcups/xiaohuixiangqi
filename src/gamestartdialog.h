#ifndef GAMESTARTDIALOG_H
#define GAMESTARTDIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include <QValidator>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include "piece.h"
#include "player.h"

namespace Ui
{
class GameStartDialog;
}

enum EngineProtocol
{
    UCI, UCCI
};

struct EngineType
{
    QString path;
    EngineProtocol protocol;
};

class GameStartDialog : public QDialog
{
    Q_OBJECT

public:
    GameStartDialog();
    ~GameStartDialog();
    int GetPlayerSelection(PieceColor color);
    int GetPlayerDiffSelection(PieceColor color);
    int GetPlayerDepthSelection(PieceColor color);
    EngineType GetEngineType(int index);
    TimeSettings GetTimeSettings(PieceColor color);

private:
    QScopedPointer<Ui::GameStartDialog> m_ui;
    QScopedPointer<QIntValidator> v;
    QScopedPointer<QIntValidator> timeValidator;
    QJsonArray engineList;

public slots:
    void handleRedPlayerChanged(int index);
    void handleBlackPlayerChanged(int index);
    void handleRedDiffChanged(int index);
    void handleBlackDiffChanged(int index);
    void handleDepthChanged();
};

#endif // GAMESTARTDIALOG_H
