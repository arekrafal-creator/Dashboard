#pragma once
// =============================================================================
// ConfigManager
//
// Odpowiada za wczytanie ORAZ zapisanie dwoch plikow JSON:
//   - config/db_config.json  -> dane polaczenia z baza danych
//   - config/queries.json    -> lista paneli (kwerend)
//
// W tej wersji programu uzytkownik NIE musi recznie edytowac tych plikow -
// robi to przez okna dialogowe w aplikacji (Polaczenie > Ustawienia,
// Kwerendy > Zarzadzaj kwerendami). Pliki JSON sa tylko wewnetrznym
// "magazynem" danych miedzy uruchomieniami programu.
// Mozna je oczywiscie nadal edytowac recznie, jesli ktos woli.
// =============================================================================

#include <QString>
#include <QVector>

// Konfiguracja polaczenia z baza danych
struct DbConfig {
    QString driver = "QODBC";     // "QODBC", "QMYSQL" albo "QPSQL"
    QString connectionString;      // uzywane dla QODBC
    QString host;                  // uzywane dla QMYSQL / QPSQL
    int     port = 0;
    QString databaseName;
    QString user;
    QString password;
    int     refreshIntervalSec = 300;
};

// Konfiguracja pojedynczego panelu / kwerendy.
// Kolejnosc w liscie = kolejnosc wyswietlania na dashboardzie
// (uklad siatki jest wyliczany automatycznie - nie trzeba podawac wiersza/kolumny).
struct PanelConfig {
    QString id;                    // wewnetrzny identyfikator (generowany automatycznie z tytulu)
    QString title;
    QString sql;
    int refreshIntervalSec = -1;   // -1 = uzyj wartosci globalnej
};

class ConfigManager {
public:
    static bool loadDbConfig(const QString &filePath, DbConfig &outConfig, QString &errorOut);
    static bool saveDbConfig(const QString &filePath, const DbConfig &config, QString &errorOut);

    // Jesli plik nie istnieje, zwraca true z pusta lista paneli (nie traktujemy tego jako blad -
    // to normalna sytuacja przy pierwszym uruchomieniu programu).
    static bool loadPanels(const QString &filePath,
                            QVector<PanelConfig> &outPanels,
                            int &globalRefreshIntervalSecOut,
                            QString &errorOut);

    static bool savePanels(const QString &filePath,
                            const QVector<PanelConfig> &panels,
                            int globalRefreshIntervalSec,
                            QString &errorOut);

    // Generuje bezpieczny identyfikator (id) na podstawie tytulu panelu,
    // np. "Pick taski - Status" -> "pick_taski_status".
    static QString slugify(const QString &title);
};
