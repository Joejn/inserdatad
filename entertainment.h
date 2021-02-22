#ifndef ENTERTAINMENT_H
#define ENTERTAINMENT_H

#include <QObject>
#include <QMqttClient>
#include <QTimer>
#include <QDir>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMediaMetaData>
#include <QTime>

#include "config.h"

class Entertainment : public QObject
{
    Q_OBJECT
public:
    explicit Entertainment(QObject *parent = nullptr);
    void connectToBroker(const QMqttClient *client);
    void startCommand(const QByteArray &msg, const QMqttTopicName &topic);
    void publishData(const QString &topic, const QString &msg);

    void setSongList();
    void setCurrentSong(const int &index);
    void playMusic(const bool &isPlaying);
    void setVolume(const int &volume);

    void setMuted(const bool &isMuted);
    void songBack();
    void songNext();

public slots:
    void setTopic();
    void setDuration(qint64 duration);
    void setCurrentPosition();
    void connectedToBroker();

private:
    QMqttClient *m_client;
    QMqttClient *m_client_publish;
    QString m_hostName;
    int m_port;
    QString m_username;
    QTimer *m_timer;
    QTime m_publishDuration;
    QTime m_publishCurrentPosition;
    QTime m_publishCurrentSongNext;
    QTime m_publishCurrentSongBack;
    QTime m_publishVolumeSongBack;
    QString m_musicDirPath;
    QStringList m_songList;
    QDir m_musicDir;
    QMediaPlayer *m_player;
    QMediaPlaylist *m_playlist;
    int m_playlistSize;
    Config m_config;
    bool m_isLooped;
};

#endif // ENTERTAINMENT_H
