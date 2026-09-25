#pragma once
// =============================================================================
// DatabaseManager
//
// Jedyne miejsce w programie, ktore rozmawia z baza danych.
// - otwiera polaczenie na podstawie DbConfig
// - wykonuje dowolna kwerende SQL i zwraca wynik jako QSqlQueryModel,
//   gotowy do podpiecia bezposrednio pod QTableView
//
// Kazdy panel (QueryPanel) korzysta z tego samego, jednego polaczenia.
// =============================================================================

#include <QObject>
#include <QSqlDatabase>
#include "ConfigManager.h"

class QSqlQueryModel;

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // Otwiera polaczenie zgodnie z konfiguracja. Zwraca true przy sukcesie.
    bool openConnection(const DbConfig &config, QString &errorOut);

    bool isConnected() const;

    // Zamyka biezace polaczenie - przydatne przed otwarciem nowego (np. po zmianie
    // ustawien w oknie "Ustawienia polaczenia" bez restartowania calego programu).
    void closeConnection();

    // Wykonuje zapytanie SQL. Zwraca nowy QSqlQueryModel (wlasnosc wywolujacego -
    // przekaz go jako parent do QTableView albo usun recznie / uzyj setModel + deleteLater).
    // W razie bledu model bedzie pusty, a errorOut opisze problem.
    QSqlQueryModel *executeQuery(const QString &sql, QString &errorOut);

private:
    QSqlDatabase m_db;
    QString m_connectionName;
};
