#include "ConnectionSettingsDialog.h"
#include "DatabaseManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QWidget>

ConnectionSettingsDialog::ConnectionSettingsDialog(const DbConfig &current, QWidget *parent)
    : QDialog(parent)
    , m_initial(current)
{
    setWindowTitle("Ustawienia polaczenia z baza danych");
    setMinimumWidth(480);
    buildUi();
}

void ConnectionSettingsDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *infoLabel = new QLabel(
        "Wpisz dane potrzebne do polaczenia z Twoja baza SQL.\n"
        "Mozesz to zmienic pozniej w kazdej chwili (menu Polaczenie > Ustawienia polaczenia).",
        this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // --- Wybor sterownika ---
    auto *driverLayout = new QFormLayout();
    m_driverCombo = new QComboBox(this);
    m_driverCombo->addItem("SQL Server / ODBC (QODBC)", "QODBC");
    m_driverCombo->addItem("MySQL / MariaDB (QMYSQL)", "QMYSQL");
    m_driverCombo->addItem("PostgreSQL (QPSQL)", "QPSQL");
    driverLayout->addRow("Typ bazy danych:", m_driverCombo);
    mainLayout->addLayout(driverLayout);

    // --- Strony zalezne od wybranego sterownika ---
    m_stack = new QStackedWidget(this);

    // Strona QODBC
    auto *odbcPage = new QWidget();
    auto *odbcLayout = new QFormLayout(odbcPage);
    m_connectionStringEdit = new QLineEdit(odbcPage);
    m_connectionStringEdit->setPlaceholderText(
        "DRIVER={ODBC Driver 17 for SQL Server};SERVER=...;DATABASE=...;UID=...;PWD=...;");
    odbcLayout->addRow("Connection string:", m_connectionStringEdit);
    auto *odbcHint = new QLabel(
        "Przyklad: DRIVER={ODBC Driver 17 for SQL Server};SERVER=NAZWA_SERWERA;"
        "DATABASE=NAZWA_BAZY;UID=uzytkownik;PWD=haslo;", odbcPage);
    odbcHint->setWordWrap(true);
    odbcHint->setStyleSheet("color: #8a94a6; font-size: 11px;");
    odbcLayout->addRow("", odbcHint);
    m_stack->addWidget(odbcPage);

    // Strona QMYSQL / QPSQL
    auto *classicPage = new QWidget();
    auto *classicLayout = new QFormLayout(classicPage);
    m_hostEdit = new QLineEdit(classicPage);
    m_hostEdit->setPlaceholderText("np. 127.0.0.1");
    m_portSpin = new QSpinBox(classicPage);
    m_portSpin->setRange(0, 65535);
    m_databaseEdit = new QLineEdit(classicPage);
    m_userEdit = new QLineEdit(classicPage);
    m_passwordEdit = new QLineEdit(classicPage);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    classicLayout->addRow("Serwer (host):", m_hostEdit);
    classicLayout->addRow("Port:", m_portSpin);
    classicLayout->addRow("Nazwa bazy:", m_databaseEdit);
    classicLayout->addRow("Uzytkownik:", m_userEdit);
    classicLayout->addRow("Haslo:", m_passwordEdit);
    m_stack->addWidget(classicPage);

    mainLayout->addWidget(m_stack);

    // --- Czas odswiezania ---
    auto *refreshLayout = new QFormLayout();
    m_refreshIntervalSpin = new QSpinBox(this);
    m_refreshIntervalSpin->setRange(5, 3600);
    m_refreshIntervalSpin->setSuffix(" sek.");
    m_refreshIntervalSpin->setValue(300);
    refreshLayout->addRow("Domyslny czas odswiezania:", m_refreshIntervalSpin);
    mainLayout->addLayout(refreshLayout);

    // --- Test polaczenia ---
    auto *testLayout = new QHBoxLayout();
    auto *testButton = new QPushButton("Testuj polaczenie", this);
    m_testResultLabel = new QLabel("", this);
    testLayout->addWidget(testButton);
    testLayout->addWidget(m_testResultLabel, 1);
    mainLayout->addLayout(testLayout);
    connect(testButton, &QPushButton::clicked, this, &ConnectionSettingsDialog::onTestConnectionClicked);

    // --- Przyciski OK / Anuluj ---
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Save)->setText("Zapisz");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Anuluj");
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConnectionSettingsDialog::onSaveClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_driverCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionSettingsDialog::onDriverChanged);

    // --- Wczytaj wartosci startowe ---
    int driverIndex = m_driverCombo->findData(m_initial.driver);
    m_driverCombo->setCurrentIndex(driverIndex >= 0 ? driverIndex : 0);
    m_connectionStringEdit->setText(m_initial.connectionString);
    m_hostEdit->setText(m_initial.host);
    m_portSpin->setValue(m_initial.port);
    m_databaseEdit->setText(m_initial.databaseName);
    m_userEdit->setText(m_initial.user);
    m_passwordEdit->setText(m_initial.password);
    m_refreshIntervalSpin->setValue(m_initial.refreshIntervalSec > 0 ? m_initial.refreshIntervalSec : 300);
    onDriverChanged(m_driverCombo->currentIndex());
}

void ConnectionSettingsDialog::onDriverChanged(int /*index*/)
{
    QString driver = m_driverCombo->currentData().toString();
    m_stack->setCurrentIndex(driver == "QODBC" ? 0 : 1);
    m_testResultLabel->clear();
}

DbConfig ConnectionSettingsDialog::collectFormValues() const
{
    DbConfig cfg;
    cfg.driver = m_driverCombo->currentData().toString();
    cfg.connectionString = m_connectionStringEdit->text().trimmed();
    cfg.host = m_hostEdit->text().trimmed();
    cfg.port = m_portSpin->value();
    cfg.databaseName = m_databaseEdit->text().trimmed();
    cfg.user = m_userEdit->text().trimmed();
    cfg.password = m_passwordEdit->text();
    cfg.refreshIntervalSec = m_refreshIntervalSpin->value();
    return cfg;
}

void ConnectionSettingsDialog::onTestConnectionClicked()
{
    DbConfig candidate = collectFormValues();

    // Tymczasowy, niezalezny DatabaseManager - test nie wplywa na aktualnie
    // dzialajace polaczenie glownego programu.
    DatabaseManager tester;
    QString error;
    if (tester.openConnection(candidate, error)) {
        m_testResultLabel->setText("Polaczenie dziala poprawnie.");
        m_testResultLabel->setStyleSheet("color: #4be07a; font-weight: 600;");
        tester.closeConnection();
    } else {
        m_testResultLabel->setText(error);
        m_testResultLabel->setStyleSheet("color: #ff5c5c; font-weight: 600;");
    }
}

void ConnectionSettingsDialog::onSaveClicked()
{
    accept();
}

DbConfig ConnectionSettingsDialog::resultConfig() const
{
    return collectFormValues();
}
