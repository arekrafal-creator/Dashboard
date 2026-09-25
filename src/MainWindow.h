#pragma once
// =============================================================================
// MainWindow
//
// Skleja wszystko w calosc. W tej wersji programu KONFIGURACJA (polaczenie
// z baza i lista kwerend) jest wpisywana przez uzytkownika w oknach
// dialogowych aplikacji, a nie recznie w plikach .json - pliki .json sluza
// tylko jako wewnetrzny zapis miedzy uruchomieniami.
//
// Uklad siatki paneli jest wyliczany automatycznie (stala liczba kolumn)
// na podstawie kolejnosci paneli - nie trzeba podawac wiersza/kolumny.
// =============================================================================

#include <QMainWindow>
#include <QVector>
#include "ConfigManager.h"

class QGridLayout;
class QLabel;
class QWidget;
class DatabaseManager;
class QueryPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override; // F11 = pelny ekran, Esc = wyjscie z pelnego ekranu

private slots:
    void onEditConnectionSettings();
    void onManagePanels();
    void onRefreshAllNow();

private:
    void buildMenuBar();
    void applyStylesheet();
    void tryConnectOrAskUser();     // proba polaczenia; jesli sie nie uda - pokazuje okno ustawien
    void rebuildPanelsGrid();       // czysci i od nowa buduje siatke kafelkow na podstawie m_panelConfigs
    void saveDbConfigToDisk();
    void savePanelsToDisk();
    QString configPath(const QString &fileName) const;

    DatabaseManager *m_dbManager = nullptr;

    QWidget *m_gridHost = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QLabel *m_emptyStateLabel = nullptr;
    QLabel *m_connectionStatusLabel = nullptr;

    DbConfig m_dbConfig;
    QVector<PanelConfig> m_panelConfigs;
    int m_globalRefreshIntervalSec = 300;

    QVector<QueryPanel *> m_panelWidgets; // aktualnie wyswietlane kafelki - do odswiezania "wszystko teraz"

    static const int kColumnsCount = 2;   // liczba kolumn siatki - zmien tu, jesli chcesz inny uklad
};
