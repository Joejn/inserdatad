#include "config.h"
#include <QDebug>

Config::Config(QObject *parent) : QObject(parent)
{
    m_configFile = "./config.conf";
    m_crypto.setKey(0xf26d13983f3fbd93);
}

QString Config::getData(const QString &key, const bool &isPassword)
{
    QSettings settings(QString(m_configFile), QSettings::IniFormat);
    QString data = settings.value(key, "None").toString();

    if(isPassword)
        data = decrypt(data);

    return data;
}

void Config::setData(const QString &key, QString data, const bool &isPassword)
{

    if(isPassword)
        data = encrypt(data);

    QSettings settings(QString(m_configFile), QSettings::IniFormat);
    settings.setValue(key, data);
    settings.sync();
}

QString Config::encrypt(const QString &plainText)
{
    return m_crypto.encryptToString(plainText);
}

QString Config::decrypt(const QString &encryptedText)
{
    return m_crypto.decryptToString(encryptedText);
}
