#pragma once
// =============================================================================
// QueryPanel
//
// Jeden "kafelek" na dashboardzie = jeden tytul + jedna tabela + wlasny timer
// odswiezania. Kazdy panel jest w pelni niezalezny od pozostalych, dzieki
// czemu dodanie / usuniecie / zmiana kwerendy w queries.json nic nie psuje.
//
// To jest jedyna klasa, ktora warto skopiowac / rozbudowac, jesli chcesz
// dodac nowy TYP wizualizacji (np. wykres) - patrz README.md.
// =============================================================================

#include <QWidget>
#include "ConfigManager.h"

class QLabel;
class QTableView;
class QTimer;
class DatabaseManager;

class QueryPanel : public QWidget {
    Q_OBJECT
public:
    explicit QueryPanel(const PanelConfig &config, DatabaseManager *dbManager, QWidget *parent = nullptr);

    // Uruchamia automatyczne odswiezanie co intervalSec sekund
    // (jesli panel ma wlasny refreshIntervalSec w configu, ten parametr jest ignorowany).
    void startAutoRefresh(int defaultIntervalSec);

    const PanelConfig &config() const { return m_config; }

public slots:
    // Wykonuje kwerende ponownie i odswieza tabele. Mozna wywolac recznie
    // (np. pod przycisk "Odswiez teraz"), a takze jest wywolywane przez timer.
    void refreshNow();

private:
    void buildUi();
    void applyRowColoring(); // proste kolorowanie wierszy na podstawie tresci komorek

    PanelConfig m_config;
    DatabaseManager *m_db;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QTableView *m_tableView = nullptr;
    QTimer *m_timer = nullptr;
};
