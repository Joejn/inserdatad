#include "entertainment.h"

Entertainment::Entertainment(QObject *parent) : QObject(parent)
{

    m_player = new QMediaPlayer;
    m_playlist = new QMediaPlaylist;
    m_playlistSize = 0;
    m_isLooped = false;

    m_hostName = m_config.getData("mqtt/hostname", false);
    m_port = (m_config.getData("mqtt/port", false)).toInt();
    m_username = m_config.getData("mqtt/username", false);
    m_musicDirPath = m_config.getData("entertainment/musicDirPath", false);

    m_musicDir.setPath(m_musicDirPath);
    m_publishDuration = QTime::currentTime();
    m_publishCurrentPosition = QTime::currentTime();
    m_publishCurrentSongNext = QTime::currentTime();
    m_publishCurrentSongBack = QTime::currentTime();

    m_client = new QMqttClient(this);
    m_client->setHostname(m_hostName);
    m_client->setPort(m_port);
    m_client->setUsername(m_username);
    m_client->setPassword(m_config.getData("mqtt/password", true));

    connectToBroker(m_client);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
            startCommand(message, topic);
        });

    connect(m_player, &QMediaPlayer::positionChanged, [this](const int &position){
        setCurrentPosition();
    });

    connect(m_player, &QMediaPlayer::currentMediaChanged, this, [&](const QMediaContent media){
        publishData("entertainment/currentSong", QString::number(m_playlist->currentIndex()));
    });

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [&](const QMediaPlayer::MediaStatus status){
                if((status == QMediaPlayer::LoadedMedia) || (status == QMediaPlayer::BufferedMedia)){
                    publishData("entertainment/title", m_player->metaData(QMediaMetaData::Title).toString());
                    publishData("entertainment/albumArtist", m_player->metaData(QMediaMetaData::AlbumArtist).toString());
                    publishData("entertainment/albumTitle", m_player->metaData(QMediaMetaData::AlbumTitle).toString());
                }
            });

    m_timer = new QTimer(this);
    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(setTopic()));
    m_timer->start(1000);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    m_client_publish = new QMqttClient(this);
    m_client_publish->setHostname(m_hostName);
    m_client_publish->setPort(m_port);
    m_client_publish->setUsername(m_username);
    m_client_publish->setPassword(m_config.getData("mqtt/password", true));

    if(m_client_publish->state() == QMqttClient::Disconnected){
        m_client_publish->connectToHost();
    }

    connect(m_client_publish, SIGNAL(connected()), this, SLOT(connectedToBroker()));
    connect(m_player, &QMediaPlayer::durationChanged, this, [&](qint64 dur){
        setDuration(dur);
    });
}

void Entertainment::connectToBroker(const QMqttClient *client)
{
    if(client->state() == QMqttClient::Disconnected){
        m_client->connectToHost();
    }
}

void Entertainment::startCommand(const QByteArray &msg, const QMqttTopicName &topic)
{
    if(topic.name() != "entertainment/currentPosition"){
        qDebug() << "Topic: " << topic.name();
        qDebug() << "Message: " << msg;
    }

    if(topic.name() == "entertainment/reload"){
        setSongList();
    }

    if(topic.name() == "entertainment/playing"){
        bool isPlaying = false;
        if(msg == "true"){
            isPlaying = true;
        }
        playMusic(isPlaying);
    }

    if(topic.name() == "entertainment/looped"){
        if(msg == "true"){
            m_playlist->setPlaybackMode(QMediaPlaylist::PlaybackMode::CurrentItemInLoop);
            m_isLooped = true;
        } else {
            m_playlist->setPlaybackMode(QMediaPlaylist::PlaybackMode::Sequential);
            m_isLooped = false;
        }
    }

    if(topic.name() == "entertainment/setCurrentPosition"){
        m_player->setPosition(msg.toUInt());
    }

    if(topic.name() == "entertainment/volume"){
        setVolume(msg.toInt());
    }

    if(topic.name() == "entertainment/currentSong"){
        setCurrentSong(msg.toInt());
    }

    if(topic.name() == "entertainment/muted"){
        bool isMuted = false;
        if(msg == "true"){
            isMuted = true;
        }
        setMuted(isMuted);
    }

    if(topic.name() == "entertainment/back"){
        songBack();
    }

    if(topic.name() == "entertainment/next"){
        songNext();
    }
}

void Entertainment::setSongList()
{
    m_songList.clear();
    m_songList = m_musicDir.entryList(QStringList() << "*mp3" << "*MP3" << "*wav" << "*WAV", QDir::Files);
    QString songListStr = m_songList.join(";");

    m_playlist->clear();
    m_playlistSize = 0;

    for(const auto &val : m_songList){
        QString songWithPath = m_musicDirPath + "/" + val;
        m_playlist->addMedia(QUrl::fromLocalFile(songWithPath));
        ++m_playlistSize;
    }

    m_player->setPlaylist(m_playlist);
    publishData("entertainment/songsList", songListStr);
}

void Entertainment::publishData(const QString &topic, const QString &msg)
{
    if(m_client_publish->publish(topic, msg.toUtf8())){
        qDebug() << "Faild to publish";
    } else {
        qDebug() << "published";
    }
}

void Entertainment::setCurrentSong(const int &index)
{
    publishData("entertainment/looped", "false");
    m_playlist->setCurrentIndex(index);
}

void Entertainment::playMusic(const bool &isPlaying)
{
    if(isPlaying){
        m_player->play();
    } else {
        m_player->pause();
    }
}

void Entertainment::setVolume(const int &volume)
{
    m_player->setVolume(volume);
}

void Entertainment::setMuted(const bool &isMuted)
{
    m_player->setMuted(isMuted);
}

void Entertainment::songBack()
{
    publishData("entertainment/looped", "false");
    if(m_publishCurrentSongBack.addMSecs(500) < QTime::currentTime()){
        if((m_playlist->currentIndex() - 1) > 0){
            m_playlist->setCurrentIndex(m_playlist->previousIndex());
            publishData("entertainment/currentSong", QString::number(m_playlist->currentIndex()));
        } else {
            m_playlist->setCurrentIndex(m_playlistSize - 1);
            publishData("entertainment/currentSong", QString::number(m_playlist->currentIndex()));
        }
        m_publishCurrentSongBack = QTime::currentTime();
    }

}

void Entertainment::songNext()
{
    publishData("entertainment/looped", "false");
    if(m_publishCurrentSongNext.addMSecs(500) < QTime::currentTime()){
        if(m_playlist->currentIndex() + 1 < m_playlistSize){
            m_playlist->setCurrentIndex(m_playlist->nextIndex());
            publishData("entertainment/currentSong", QString::number(m_playlist->currentIndex()));
        } else {
            m_playlist->setCurrentIndex(0);
            publishData("entertainment/currentSong", QString::number(m_playlist->currentIndex()));
        }
        m_publishCurrentSongNext = QTime::currentTime();
    }
}

void Entertainment::setDuration(qint64 duration)
{
        publishData("entertainment/duration", QString::number(duration));
}

void Entertainment::setCurrentPosition()
{
    if(m_publishCurrentPosition.addMSecs(500) < QTime::currentTime()){
        publishData("entertainment/currentPosition", QString::number(m_player->position()));
        m_publishCurrentPosition = QTime::currentTime();
    }
}

void Entertainment::connectedToBroker()
{
    setSongList();
    publishData("entertainment/volume", "50");
}

void Entertainment::setTopic()
{
    QString topic = "entertainment/#";
    auto subscription = m_client->subscribe(topic);
    if (!subscription) {
        qDebug() << "subscription of topic " + topic + " failed. Retry in one second";
        connectToBroker(m_client);
    }
}
