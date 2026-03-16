#include "gamestartdialog.h"
#include "ui_gamestartdialog.h"

GameStartDialog::GameStartDialog()
    : m_ui(new Ui::GameStartDialog), v(new QIntValidator(1, 50, this)),
      timeValidator(new QIntValidator(0, 9999, this))
{
    m_ui->setupUi(this);
    m_ui->RedStepTime->setValidator(timeValidator.get());
    m_ui->RedTotalTime->setValidator(timeValidator.get());
    m_ui->BlackStepTime->setValidator(timeValidator.get());
    m_ui->BlackTotalTime->setValidator(timeValidator.get());

    QSettings settings;
    engineList = settings.value("list").toJsonArray();
    QString name;
    for (auto i : engineList) {
        name = i.toObject()["name"].toString();
        m_ui->RedPlayer->addItem(name);
        m_ui->BlackPlayer->addItem(name);
    }

    connect(m_ui->RedPlayer, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameStartDialog::handleRedPlayerChanged);
    connect(m_ui->BlackPlayer, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameStartDialog::handleBlackPlayerChanged);
    connect(m_ui->RedDiff, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameStartDialog::handleRedDiffChanged);
    connect(m_ui->BlackDiff, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameStartDialog::handleBlackDiffChanged);
    connect(m_ui->RedDepth, &QLineEdit::textChanged, this, &GameStartDialog::handleDepthChanged);
    connect(m_ui->BlackDepth, &QLineEdit::textChanged, this, &GameStartDialog::handleDepthChanged);
}

int GameStartDialog::GetPlayerSelection(PieceColor color)
{
    if (color == Red) return m_ui->RedPlayer->currentIndex();
    else return m_ui->BlackPlayer->currentIndex();
}

int GameStartDialog::GetPlayerDepthSelection(PieceColor color)
{
    if (color == Red) return m_ui->RedDepth->text().toInt();
    else return m_ui->BlackDepth->text().toInt();
}

int GameStartDialog::GetPlayerDiffSelection(PieceColor color)
{
    if (color == Red) return m_ui->RedDiff->currentIndex();
    else return m_ui->BlackDiff->currentIndex();
}

EngineType GameStartDialog::GetEngineType(int index)
{
    return EngineType{
        .path = engineList[index - 1].toObject()["path"].toString(),
        .protocol = EngineProtocol(engineList[index - 1].toObject()["protocol"].toInt())
    };
}

TimeSettings GameStartDialog::GetTimeSettings(PieceColor color)
{
    if (color == Red) {
        return TimeSettings{
            .stepTime = m_ui->RedStepTime->text().toInt(),
            .totalTime = m_ui->RedTotalTime->text().toInt()
        };
    } else {
        return TimeSettings{
            .stepTime = m_ui->BlackStepTime->text().toInt(),
            .totalTime = m_ui->BlackTotalTime->text().toInt()
        };
    }
}

void GameStartDialog::handleRedPlayerChanged(int index)
{
    m_ui->RedDiff->setEnabled(index != 0);
    m_ui->RedDiffLabel->setEnabled(index != 0);
}

void GameStartDialog::handleBlackPlayerChanged(int index)
{
    m_ui->BlackDiff->setEnabled(index != 0);
    m_ui->BlackDiffLabel->setEnabled(index != 0);
}

void GameStartDialog::handleRedDiffChanged(int index)
{
    m_ui->RedDepth->setEnabled(index == 2);
    m_ui->RedDepthLabel->setEnabled(index == 2);
}

void GameStartDialog::handleBlackDiffChanged(int index)
{
    m_ui->BlackDepth->setEnabled(index == 2);
    m_ui->BlackDepthLabel->setEnabled(index == 2);
}

void GameStartDialog::handleDepthChanged()
{
    auto bs = m_ui->BlackDepth->text();
    auto rs = m_ui->RedDepth->text();
    auto pos = 0;
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
        v->validate(bs, pos) == QValidator::Acceptable
        && v->validate(rs, pos) == QValidator::Acceptable
    );
}

GameStartDialog::~GameStartDialog() {}

