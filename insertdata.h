#ifndef INSERTDATA_H
#define INSERTDATA_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QtMqtt/QtMqtt>
#include <QMqttClient>
#include <QMqttSubscription>
#include <QtSql/QSqlDatabase>

#include "config.h"
#include "dbadministration.h"

class InsertData : public QObject
{
    Q_OBJECT
public:
    explicit InsertData(QObject *parent = nullptr);
    void insertIntoDatabase(const QString &msg, const QMqttTopicName &topic);

public slots:
    void startInsertSensorData();
    void startInsertAccescontrol();

private:
    QMqttClient *m_client_insertIntoDatabase;
    QMqttClient *m_client_insertIntoDatabaseAccescontrol;
    QMqttClient *m_client_addButton;
    QMqttClient *m_client_addButtonNodeName;
    DbAdministration *m_db;
    QString m_dbHostname;
    QString m_dbName;
    QString m_dbUser;
    Config m_config;
    QTimer *m_timer;
    QStringList m_tableNames;
    QString m_hostname;
    int m_port;
    QString m_username;

signals:

};

#endif // INSERTDATA_H
