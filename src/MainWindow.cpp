#include "MainWindow.h"
#include "DatabaseManager.h"
#include "QueryPanel.h"
#include "ClockWidget.h"
#include "ConnectionSettingsDialog.h"
#include "PanelManagerDialog.h"

#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLayoutItem>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Dashboard Magazynowy");
    resize(1600, 900);

    m_dbManager = new DatabaseManager(this);

    applyStylesheet();
    buildMenuBar();

    // ---------------------------------------------------------------
    // Glowny uklad okna: pasek statusu/zegara na gorze, siatka paneli ponizej.
    // ---------------------------------------------------------------
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *topBar = new QWidget(central);
    topBar->setObjectName("topBar");
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(20, 14, 20, 14);

    auto *appTitle = new QLabel("Dashboard Magazynowy - Live", topBar);
    appTitle->setObjectName("appTitle");

    m_connectionStatusLabel = new QLabel("Laczenie z baza danych...", topBar);
    m_connectionStatusLabel->setObjectName("connectionStatusError");

    auto *clock = new ClockWidget(topBar);

    topBarLayout->addWidget(appTitle);
    topBarLayout->addStretch(1);
    topBarLayout->addWidget(m_connectionStatusLabel);
    topBarLayout->addSpacing(30);
    topBarLayout->addWidget(clock);
    mainLayout->addWidget(topBar);

    auto *scrollArea = new QScrollArea(central);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_gridHost = new QWidget();
    m_gridLayout = new QGridLayout(m_gridHost);
    m_gridLayout->setContentsMargins(16, 16, 16, 16);
    m_gridLayout->setSpacing(16);

    m_emptyStateLabel = new QLabel(
        "Brak zdefiniowanych kwerend.\n\n"
        "Otworz menu \"Kwerendy > Zarzadzaj kwerendami...\", aby dodac pierwszy panel.",
        m_gridHost);
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setStyleSheet("color: #5b6472; font-size: 18px; padding: 60px;");

    scrollArea->setWidget(m_gridHost);
    mainLayout->addWidget(scrollArea, 1);

    // ---------------------------------------------------------------
    // Wczytaj to, co zostalo zapisane przy poprzednim uruchomieniu (jesli cokolwiek jest).
    // ---------------------------------------------------------------
    QString error;
    ConfigManager::loadDbConfig(configPath("db_config.json"), m_dbConfig, error);
    ConfigManager::loadPanels(configPath("queries.json"), m_panelConfigs, m_globalRefreshIntervalSec, error);

    tryConnectOrAskUser();
    rebuildPanelsGrid();
}

MainWindow::~MainWindow() = default;

QString MainWindow::configPath(const QString &fileName) const
{
    return QApplication::applicationDirPath() + "/config/" + fileName;
}

void MainWindow::applyStylesheet()
{
    QString stylePath = QApplication::applicationDirPath() + "/resources/style.qss";
    QFile styleFile(stylePath);
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        qApp->setStyleSheet(stream.readAll());
        styleFile.close();
    }
}

void MainWindow::buildMenuBar()
{
    auto *connectionMenu = menuBar()->addMenu("Polaczenie");
    auto *connectionSettingsAction = connectionMenu->addAction("Ustawienia polaczenia...");
    connect(connectionSettingsAction, &QAction::triggered, this, &MainWindow::onEditConnectionSettings);

    auto *queriesMenu = menuBar()->addMenu("Kwerendy");
    auto *managePanelsAction = queriesMenu->addAction("Zarzadzaj kwerendami...");
    connect(managePanelsAction, &QAction::triggered, this, &MainWindow::onManagePanels);

    auto *viewMenu = menuBar()->addMenu("Widok");
    auto *refreshAction = viewMenu->addAction("Odswiez wszystko teraz");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshAllNow);
    auto *fullscreenAction = viewMenu->addAction("Pelny ekran (F11)");
    connect(fullscreenAction, &QAction::triggered, this, [this]() {
        setWindowState(isFullScreen() ? Qt::WindowMaximized : Qt::WindowFullScreen);
    });
}

void MainWindow::tryConnectOrAskUser()
{
    QString error;
    bool connected = m_dbManager->openConnection(m_dbConfig, error);

    if (!connected) {
        QMessageBox::information(this, "Skonfiguruj polaczenie z baza danych",
            "Nie mozna polaczyc sie z baza danych (albo to pierwsze uruchomienie programu).\n"
            "Wypelnij dane polaczenia w kolejnym oknie.");
        onEditConnectionSettings(); // to wywola ponowna probe polaczenia i zapisze konfiguracje
        return;
    }

    m_connectionStatusLabel->setObjectName("connectionStatusOk");
    m_connectionStatusLabel->setText("Polaczono z baza danych");
    m_connectionStatusLabel->style()->unpolish(m_connectionStatusLabel);
    m_connectionStatusLabel->style()->polish(m_connectionStatusLabel);
}

void MainWindow::onEditConnectionSettings()
{
    ConnectionSettingsDialog dialog(m_dbConfig, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_dbConfig = dialog.resultConfig();
    m_globalRefreshIntervalSec = m_dbConfig.refreshIntervalSec;
    saveDbConfigToDisk();

    QString error;
    bool connected = m_dbManager->openConnection(m_dbConfig, error);
    if (connected) {
        m_connectionStatusLabel->setObjectName("connectionStatusOk");
        m_connectionStatusLabel->setText("Polaczono z baza danych");
        onRefreshAllNow();
    } else {
        m_connectionStatusLabel->setObjectName("connectionStatusError");
        m_connectionStatusLabel->setText(error);
        QMessageBox::warning(this, "Blad polaczenia", error);
    }
    m_connectionStatusLabel->style()->unpolish(m_connectionStatusLabel);
    m_connectionStatusLabel->style()->polish(m_connectionStatusLabel);
}

void MainWindow::onManagePanels()
{
    PanelManagerDialog dialog(m_panelConfigs, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_panelConfigs = dialog.resultPanels();
    savePanelsToDisk();
    rebuildPanelsGrid();
}

void MainWindow::onRefreshAllNow()
{
    for (QueryPanel *panel : m_panelWidgets) {
        panel->refreshNow();
    }
}

void MainWindow::saveDbConfigToDisk()
{
    QString error;
    if (!ConfigManager::saveDbConfig(configPath("db_config.json"), m_dbConfig, error)) {
        QMessageBox::warning(this, "Nie udalo sie zapisac ustawien", error);
    }
}

void MainWindow::savePanelsToDisk()
{
    QString error;
    if (!ConfigManager::savePanels(configPath("queries.json"), m_panelConfigs, m_globalRefreshIntervalSec, error)) {
        QMessageBox::warning(this, "Nie udalo sie zapisac kwerend", error);
    }
}

void MainWindow::rebuildPanelsGrid()
{
    // Usun poprzednie kafelki z ukladu i z pamieci (m_emptyStateLabel jest
    // wielokrotnego uzytku, wiec tylko go ukrywamy zamiast usuwac).
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyStateLabel)
            item->widget()->deleteLater();
        delete item;
    }
    m_panelWidgets.clear();
    m_emptyStateLabel->hide();

    if (m_panelConfigs.isEmpty()) {
        m_gridLayout->addWidget(m_emptyStateLabel, 0, 0);
        m_emptyStateLabel->show();
        return;
    }

    for (int i = 0; i < m_panelConfigs.size(); ++i) {
        const PanelConfig &cfg = m_panelConfigs[i];
        int row = i / kColumnsCount;
        int col = i % kColumnsCount;

        auto *panel = new QueryPanel(cfg, m_dbManager, m_gridHost);
        m_gridLayout->addWidget(panel, row, col);
        panel->startAutoRefresh(m_globalRefreshIntervalSec);
        m_panelWidgets.push_back(panel);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        setWindowState(isFullScreen() ? Qt::WindowMaximized : Qt::WindowFullScreen);
    } else if (event->key() == Qt::Key_Escape && isFullScreen()) {
        setWindowState(Qt::WindowMaximized);
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
