#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>

#include "simplecrypt.h"

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);

    Q_INVOKABLE QString getData(const QString &key, const bool &isPassword);
    Q_INVOKABLE void setData(const QString &key, QString data, const bool &isPassword);
    QString encrypt(const QString &plainText);
    QString decrypt(const QString &encryptedText);

private:
    QString m_configFile;
    SimpleCrypt m_crypto;

};

#endif // CONFIG_H
