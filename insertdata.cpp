#include "insertdata.h"

#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <qsqldatabase.h>
#include <QSqlError>
#include <QtSql>

#include <QFile>
#include <QTextStream>

#include <QTime>
#include <QDateTime>

InsertData::InsertData(QObject *parent) : QObject(parent)
{
    qDebug() << "service starting";

    m_dbHostname = m_config.getData("db/hostname", false);
    m_dbName = m_config.getData("db/name", false);
    m_dbUser = m_config.getData("db/user", false);
    m_db = new DbAdministration(this, m_dbHostname, m_dbName, m_dbUser, m_config.getData("db/password", true));

    m_hostname = m_config.getData("mqtt/hostname", false);
    m_port = (m_config.getData("mqtt/port", false)).toInt();
    m_username = m_config.getData("mqtt/username", false);

    m_client_insertIntoDatabase = new QMqttClient(this);
    m_client_insertIntoDatabase->setHostname(m_hostname);
    m_client_insertIntoDatabase->setPort(m_port);
    m_client_insertIntoDatabase->setUsername(m_username);
    m_client_insertIntoDatabase->setPassword(m_config.getData("mqtt/password", true));

    m_client_insertIntoDatabaseAccescontrol = new QMqttClient(this);
    m_client_insertIntoDatabaseAccescontrol->setHostname(m_hostname);
    m_client_insertIntoDatabaseAccescontrol->setPort(m_port);
    m_client_insertIntoDatabaseAccescontrol->setUsername(m_username);
    m_client_insertIntoDatabaseAccescontrol->setPassword(m_config.getData("mqtt/password", true));

    if(m_client_insertIntoDatabase->state() == QMqttClient::Disconnected){
        m_client_insertIntoDatabase->connectToHost();
    }

    if(m_client_insertIntoDatabaseAccescontrol->state() == QMqttClient::Disconnected){
        m_client_insertIntoDatabaseAccescontrol->connectToHost();
    }

    qDebug() << "state:" << m_client_insertIntoDatabase->state();

    connect(m_client_insertIntoDatabase, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        insertIntoDatabase(message, topic);
    });

    connect(m_client_insertIntoDatabaseAccescontrol, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        insertIntoDatabase(message, topic);
    });

    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(startInsertSensorData()));
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(startInsertAccescontrol()));
    m_timer->start(1000);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    m_client_addButtonNodeName = new QMqttClient(this);
    m_client_addButtonNodeName->setHostname(m_hostname);
    m_client_addButtonNodeName->setPort(m_port);
    m_client_addButtonNodeName->setUsername(m_username);
    m_client_addButtonNodeName->setPassword(m_config.getData("mqtt/password", true));


    if(m_client_addButtonNodeName->state() == QMqttClient::Disconnected){
        m_client_addButtonNodeName->connectToHost();
    }
}

void InsertData::startInsertSensorData()
{
    QString topic = "Sensors/#";
    auto subscription = m_client_insertIntoDatabase->subscribe(topic);
    if (!subscription) {
        qDebug() << "subscription of topic " + topic + " failed. Retry in one second";
    }
}

void InsertData::startInsertAccescontrol()
{
    QString topic = "AccesControl/Content";
    auto subscription = m_client_insertIntoDatabaseAccescontrol->subscribe(topic);
    if (!subscription) {
        qDebug() << "subscription of topic " + topic + " failed. Retry in one second";
    }
}

void InsertData::insertIntoDatabase(const QString &msg, const QMqttTopicName &topic)
{

    if(msg == "nan"){
        return;
    }

    if(msg.isEmpty()){
        return;
    }

    QString tableName = topic.name().left(topic.name().length() - 4);
    tableName = tableName.replace(0, 8, "");

    /* QStringList tableValues = msg.split(QLatin1Char(';'));
    QList<QPair<QString, QString>> tableNamesValues;

    for(const auto &val : tableValues){
        QStringList tabelValueList = val.split(QLatin1Char(','));
        if(tabelValueList.at(0).isEmpty() || tabelValueList.at(1).isEmpty()){
            continue;
        }

        tableNamesValues.append(qMakePair(tabelValueList.at(0), tabelValueList.at(1)));
    } */

    QString statement;
    if(topic.name() == "AccesControl/Content"){

        QFile file("./resolve.txt");
        QString name = msg;
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        } else {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                line.replace(" ", "");
                QStringList id;
                id = line.split(":");
                id = id[1].split(",");

                if(id.contains(msg)){
                    name = "";
                    for(const auto &val : line){
                        if(val == ":")
                            break;
                        name += val;
                    }
                }
            }
        }

        statement = "INSERT INTO accescontrol (name, scandatetime) VALUES ('" + name + "', '" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "');";

    } else {
        statement = "INSERT INTO " + tableName + " (measurementdatetime, SENSOREVALUE) VALUES ('" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm") + "', "  + msg + ");";
    }

    if(!(m_db->execStatement(statement))){
        qDebug() << "statement : " << statement << "konnte nicht druchgeführt werden";
    }
}
