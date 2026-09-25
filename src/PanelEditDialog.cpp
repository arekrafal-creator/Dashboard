#include "PanelEditDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFont>

PanelEditDialog::PanelEditDialog(const PanelConfig &initial, bool isNew, QWidget *parent)
    : QDialog(parent)
    , m_initial(initial)
    , m_isNew(isNew)
{
    setWindowTitle(isNew ? "Dodaj nowy panel" : "Edytuj panel");
    setMinimumSize(560, 420);
    buildUi(isNew);
}

void PanelEditDialog::buildUi(bool isNew)
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    m_titleEdit = new QLineEdit(m_initial.title, this);
    m_titleEdit->setPlaceholderText("np. Pick taski wg statusu");
    formLayout->addRow("Tytul panelu:", m_titleEdit);
    mainLayout->addLayout(formLayout);

    auto *sqlLabel = new QLabel("Zapytanie SQL (wklej tutaj):", this);
    mainLayout->addWidget(sqlLabel);

    m_sqlEdit = new QPlainTextEdit(this);
    m_sqlEdit->setPlainText(m_initial.sql);
    m_sqlEdit->setPlaceholderText("SELECT ...");
    QFont monoFont("Consolas");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(11);
    m_sqlEdit->setFont(monoFont);
    m_sqlEdit->setMinimumHeight(180);
    mainLayout->addWidget(m_sqlEdit, 1);

    auto *refreshLayout = new QHBoxLayout();
    m_customRefreshCheck = new QCheckBox("Wlasny czas odswiezania dla tego panelu", this);
    m_refreshIntervalSpin = new QSpinBox(this);
    m_refreshIntervalSpin->setRange(5, 3600);
    m_refreshIntervalSpin->setSuffix(" sek.");
    m_refreshIntervalSpin->setEnabled(false);

    bool hasCustomRefresh = m_initial.refreshIntervalSec > 0;
    m_customRefreshCheck->setChecked(hasCustomRefresh);
    m_refreshIntervalSpin->setEnabled(hasCustomRefresh);
    m_refreshIntervalSpin->setValue(hasCustomRefresh ? m_initial.refreshIntervalSec : 300);

    connect(m_customRefreshCheck, &QCheckBox::toggled, m_refreshIntervalSpin, &QSpinBox::setEnabled);

    refreshLayout->addWidget(m_customRefreshCheck);
    refreshLayout->addWidget(m_refreshIntervalSpin);
    refreshLayout->addStretch(1);
    mainLayout->addLayout(refreshLayout);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Save)->setText(isNew ? "Dodaj panel" : "Zapisz zmiany");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Anuluj");
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PanelEditDialog::onSaveClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void PanelEditDialog::onSaveClicked()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Brak tytulu", "Podaj tytul panelu.");
        return;
    }
    if (m_sqlEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Brak zapytania SQL", "Wklej zapytanie SQL dla tego panelu.");
        return;
    }
    accept();
}

PanelConfig PanelEditDialog::resultConfig() const
{
    PanelConfig cfg = m_initial; // zachowuje id, jesli to edycja istniejacego panelu
    cfg.title = m_titleEdit->text().trimmed();
    cfg.sql = m_sqlEdit->toPlainText().trimmed();
    cfg.refreshIntervalSec = m_customRefreshCheck->isChecked() ? m_refreshIntervalSpin->value() : -1;
    return cfg;
}
