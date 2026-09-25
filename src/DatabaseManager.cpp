#include "DatabaseManager.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // Unikalna nazwa polaczenia - przydatne, gdyby w przyszlosci ktos chcial
    // otworzyc wiecej niz jedno polaczenie w tym samym programie.
    m_connectionName = "warehouse_dashboard_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

void DatabaseManager::closeConnection()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::openConnection(const DbConfig &config, QString &errorOut)
{
    closeConnection();
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }

    m_db = QSqlDatabase::addDatabase(config.driver, m_connectionName);

    if (config.driver == "QODBC") {
        // Dla ODBC caly connection string (serwer, baza, autoryzacja) jest w jednym polu.
        m_db.setDatabaseName(config.connectionString);
    } else {
        // QMYSQL / QPSQL - klasyczne pola host/port/baza/uzytkownik/haslo.
        m_db.setHostName(config.host);
        if (config.port > 0)
            m_db.setPort(config.port);
        m_db.setDatabaseName(config.databaseName);
        m_db.setUserName(config.user);
        m_db.setPassword(config.password);
    }

    if (!m_db.open()) {
        errorOut = QString("Nie udalo sie polaczyc z baza danych: %1")
                       .arg(m_db.lastError().text());
        return false;
    }

    return true;
}

bool DatabaseManager::isConnected() const
{
    return m_db.isOpen();
}

QSqlQueryModel *DatabaseManager::executeQuery(const QString &sql, QString &errorOut)
{
    QSqlQueryModel *model = new QSqlQueryModel();

    if (!m_db.isOpen()) {
        errorOut = "Brak polaczenia z baza danych.";
        return model; // pusty model
    }

    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        errorOut = QString("Blad zapytania SQL: %1").arg(query.lastError().text());
        return model; // pusty model, ale nie crashujemy programu
    }

    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        errorOut = QString("Blad zapytania SQL: %1").arg(model->lastError().text());
    }

    return model;
}
