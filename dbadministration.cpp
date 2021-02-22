#include "dbadministration.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QProcess>
#include <QVariant>
#include <QDebug>

DbAdministration::DbAdministration(QObject *parent, const QString &hostname, const QString& dbName, const QString &user, const QString &password) : QObject(parent),
    m_hostname(hostname),
    m_dbName(dbName),
    m_user(user),
    m_password(password),
    m_isConnected(true)
{
    m_connectionTimeout = m_config.getData("db/timeout", false);
    if(m_connectionTimeout == "None"){
        m_connectionTimeout = "10";
    }
}

// setter //////////////////////////////////////////////////////////////////////////////////

void DbAdministration::setHostname(const QString &hostame)
{
    m_hostname = hostame;
}

void DbAdministration::setDbName(const QString &dbName)
{
    m_dbName = dbName;
}

void DbAdministration::setDbUserName(const QString &user)
{
    m_user = user;
}

void DbAdministration::setDbUserPassword(const QString &password)
{
    m_password = password;
}

// getter //////////////////////////////////////////////////////////////////////////////////

QString DbAdministration::getHostname()
{
    return m_hostname;
}

QString DbAdministration::getDbName()
{
    return m_dbName;
}

QString DbAdministration::getDbUserName()
{
    return m_user;
}

QString DbAdministration::getDbUserPassword()
{
    return m_password;
}

QStringList DbAdministration::getDataListFromDb(const unsigned int &index)
{
    QStringList dataAtIndex;
    for(const auto &val : m_dataList){
        dataAtIndex.append(val.at(index));
    }
    return dataAtIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////

bool DbAdministration::setDataListFromDb(const QString &statement, const QString &positions)
{
    m_dataList.clear();
    m_positions.clear();

    m_positions = positions.split(";");
    bool isConnected = false;

    if(dbServerIsReachable()){
        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QPSQL7");
        db.setHostName(m_hostname);
        db.setDatabaseName(m_dbName);
        db.setUserName(m_user);
        db.setPassword(m_password);

        if(db.open()){
            QSqlQuery query(statement);
            isConnected = true;
                while (query.next()) {
                    QStringList data;
                    for(const auto &val : m_positions){
                        data.append(query.value(val.toInt()).toString());
                    }
                    m_dataList.append(data);
                }
        }
    }

    // if(m_isConnected != isConnected){
        m_isConnected = isConnected;
        emit isConnectedChanged(m_isConnected);
    // }

    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    return isConnected;
}

bool DbAdministration::execStatement(const QString &statement)
{
    bool isConnected = false;
    if(dbServerIsReachable()){
        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QPSQL7");
        db.setHostName(m_hostname);
        db.setDatabaseName(m_dbName);
        db.setUserName(m_user);
        db.setPassword(m_password);

        if(db.open()){
            QSqlQuery query;
            if(query.exec(statement)){
                isConnected = true;
            }
            db.close();
        }
    }

    if(m_isConnected != isConnected){
        m_isConnected = isConnected;
        emit isConnectedChanged(m_isConnected);
    }

    QSqlDatabase::removeDatabase("qt_sql_default_connection");
    return isConnected;
}

bool DbAdministration::dbServerIsReachable()
{
    QString cmd;

    #if (defined (_WIN32) || defined (_WIN64))
        cmd = "ping " + m_hostname + " -n 1 -w " + m_connectionTimeout;
    #else
        cmd = "ping " + m_hostname + " -c 1 -W " + m_connectionTimeout;
    #endif

    if(QProcess::execute(cmd) == 0){
        return true;
    }
    return false;
}

bool DbAdministration::getIsConnected()
{
    return m_isConnected;
}
