#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog()
    : m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);
    m_ui->engineListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    QSettings settings;
    engineList = settings.value("list").toJsonArray();
    for (auto i : engineList) {
        auto engine = i.toObject();
        m_ui->engineListWidget->addItem(engine["name"].toString() + "("
                                        + engine["path"].toString() + ","
                                        + (engine["protocol"].toInt() ? QString("UCCI") : QString("UCI")) + ")");
    }
    m_ui->rotate->setChecked(settings.value("rotate", true).toBool());
    m_ui->animation->setChecked(settings.value("animation", true).toBool());

    connect(m_ui->chooseFile, &QToolButton::clicked, this, &SettingsDialog::onChooseTriggered);
    connect(m_ui->addEngine, &QToolButton::clicked, this, &SettingsDialog::onAddTriggered);
    connect(m_ui->deleteEngine, &QToolButton::clicked, this, &SettingsDialog::onRemoveTriggered);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply),
            &QPushButton::clicked, this, &SettingsDialog::onApplyTriggered);
    connect(this, &SettingsDialog::accepted, this, &SettingsDialog::onApplyTriggered);
    connect(m_ui->rotate, &QCheckBox::checkStateChanged, this, &SettingsDialog::onApplyEnabled);
    connect(m_ui->animation, &QCheckBox::checkStateChanged, this, &SettingsDialog::onApplyEnabled);
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::onAddTriggered()
{
    QJsonObject engine;
    auto name = m_ui->engineName->text();
    auto path = m_ui->enginePath->text();
    auto protocol = m_ui->engineType->currentIndex();
    if (!QFile::exists(path)) return;
    engine["name"] = name;
    engine["path"] = path;
    engine["protocol"] = protocol;
    engineList.append(engine);
    m_ui->engineListWidget->addItem(name + "(" + path + ","
                                    + (protocol ? QString("UCCI") : QString("UCI")) + ")");
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::onChooseTriggered()
{
    m_ui->enginePath->setText(
        QFileDialog::getOpenFileName(this, tr("选择引擎程序")));
}

void SettingsDialog::onRemoveTriggered()
{
    auto index = m_ui->engineListWidget->currentRow();
    if (index == -1) return;
    engineList.removeAt(index);
    auto item = m_ui->engineListWidget->takeItem(index);
    if (item) delete item;
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void SettingsDialog::onApplyTriggered()
{
    QSettings settings;
    settings.setValue("list", engineList);
    settings.setValue("rotate", m_ui->rotate->isChecked());
    settings.setValue("animation", m_ui->animation->isChecked());
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);
}

void SettingsDialog::onApplyEnabled()
{
    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
