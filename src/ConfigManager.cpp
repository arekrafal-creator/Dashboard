#include "ConfigManager.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QRegularExpression>

static bool readJsonObject(const QString &filePath, QJsonObject &outObject, bool &fileExisted, QString &errorOut)
{
    QFile file(filePath);
    fileExisted = file.exists();
    if (!fileExisted)
        return true; // brak pliku nie jest bledem - patrz komentarze w naglowku

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorOut = QString("Nie mozna otworzyc pliku: %1").arg(filePath);
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        errorOut = QString("Blad w skladni JSON w pliku %1 (pozycja %2): %3")
                       .arg(filePath).arg(parseError.offset).arg(parseError.errorString());
        return false;
    }
    if (!doc.isObject()) {
        errorOut = QString("Plik %1 powinien zawierac obiekt JSON { ... }").arg(filePath);
        return false;
    }

    outObject = doc.object();
    return true;
}

static bool writeJsonObject(const QString &filePath, const QJsonObject &object, QString &errorOut)
{
    QFileInfo info(filePath);
    QDir().mkpath(info.absolutePath()); // upewnij sie, ze folder config/ istnieje

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorOut = QString("Nie mozna zapisac pliku: %1").arg(filePath);
        return false;
    }

    QJsonDocument doc(object);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ConfigManager::loadDbConfig(const QString &filePath, DbConfig &outConfig, QString &errorOut)
{
    QJsonObject root;
    bool fileExisted = false;
    if (!readJsonObject(filePath, root, fileExisted, errorOut))
        return false;

    if (!fileExisted)
        return true; // zostaja wartosci domyslne z DbConfig{}

    outConfig.driver             = root.value("driver").toString("QODBC");
    outConfig.connectionString   = root.value("connection_string").toString();
    outConfig.host                = root.value("host").toString();
    outConfig.port                = root.value("port").toInt();
    outConfig.databaseName        = root.value("database_name").toString();
    outConfig.user                = root.value("user").toString();
    outConfig.password            = root.value("password").toString();
    outConfig.refreshIntervalSec  = root.value("refresh_interval_sec").toInt(300);

    return true;
}

bool ConfigManager::saveDbConfig(const QString &filePath, const DbConfig &config, QString &errorOut)
{
    QJsonObject root;
    root["driver"]              = config.driver;
    root["connection_string"]   = config.connectionString;
    root["host"]                 = config.host;
    root["port"]                 = config.port;
    root["database_name"]        = config.databaseName;
    root["user"]                  = config.user;
    root["password"]              = config.password;
    root["refresh_interval_sec"]  = config.refreshIntervalSec;

    return writeJsonObject(filePath, root, errorOut);
}

bool ConfigManager::loadPanels(const QString &filePath,
                                QVector<PanelConfig> &outPanels,
                                int &globalRefreshIntervalSecOut,
                                QString &errorOut)
{
    outPanels.clear();
    globalRefreshIntervalSecOut = 300;

    QJsonObject root;
    bool fileExisted = false;
    if (!readJsonObject(filePath, root, fileExisted, errorOut))
        return false;

    if (!fileExisted)
        return true; // pierwsze uruchomienie - brak paneli, uzytkownik doda je w aplikacji

    globalRefreshIntervalSecOut = root.value("refresh_interval_sec").toInt(300);

    QJsonArray panelsArray = root.value("panels").toArray();
    for (const QJsonValue &val : panelsArray) {
        if (!val.isObject())
            continue;
        QJsonObject obj = val.toObject();

        PanelConfig cfg;
        cfg.id    = obj.value("id").toString();
        cfg.title = obj.value("title").toString(cfg.id);
        cfg.sql   = obj.value("sql").toString();
        cfg.refreshIntervalSec = obj.value("refresh_interval_sec").toInt(-1);

        if (cfg.id.isEmpty() || cfg.sql.isEmpty())
            continue; // pomijamy niekompletne wpisy

        outPanels.push_back(cfg);
    }

    return true;
}

bool ConfigManager::savePanels(const QString &filePath,
                                const QVector<PanelConfig> &panels,
                                int globalRefreshIntervalSec,
                                QString &errorOut)
{
    QJsonObject root;
    root["refresh_interval_sec"] = globalRefreshIntervalSec;

    QJsonArray panelsArray;
    for (const PanelConfig &cfg : panels) {
        QJsonObject obj;
        obj["id"] = cfg.id;
        obj["title"] = cfg.title;
        obj["sql"] = cfg.sql;
        obj["refresh_interval_sec"] = cfg.refreshIntervalSec;
        panelsArray.append(obj);
    }
    root["panels"] = panelsArray;

    return writeJsonObject(filePath, root, errorOut);
}

QString ConfigManager::slugify(const QString &title)
{
    QString result = title.toLower().trimmed();
    result.replace(QRegularExpression("[^a-z0-9]+"), "_");
    result.replace(QRegularExpression("^_+|_+$"), "");
    if (result.isEmpty())
        result = "panel";
    return result;
}
