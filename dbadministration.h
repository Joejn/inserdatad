#ifndef DBADMINISTRATION_H
#define DBADMINISTRATION_H

#include <QObject>
#include <QStringList>
#include <QPair>

#include "config.h"

class DbAdministration : public QObject
{
    Q_OBJECT
public:
    explicit DbAdministration(QObject *parent = nullptr, const QString &hostname = "", const QString& dbName = "", const QString &user = "", const QString &password = "");

    void setHostname(const QString &hostame);
    void setDbName(const QString &dbName);
    void setDbUserName(const QString &user);
    void setDbUserPassword(const QString &password);
    bool setDataListFromDb(const QString &statement, const QString &positions);
    bool execStatement(const QString &statement);

    QString getHostname();
    QString getDbName();
    QString getDbUserName();
    QString getDbUserPassword();
    QStringList getDataListFromDb(const unsigned int &index);

    bool dbServerIsReachable();
    bool getIsConnected();

signals:
    void isConnectedChanged(const bool &isConnected);

private:
    QString m_hostname;
    QString m_dbName;
    QString m_user;
    QString m_password;
    QString m_connectionTimeout;
    Config m_config;
    QStringList m_positions;
    QList<QStringList> m_dataList;
    unsigned int m_dataListMaxIndex;
    bool m_isConnected;

};

#endif // DBADMINISTRATION_H
