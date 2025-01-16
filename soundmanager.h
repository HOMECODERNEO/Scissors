#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <Includes.h>

class SoundManager : public QObject{
    Q_OBJECT

public:
    explicit SoundManager(QObject *parent = nullptr);
    void PlaySound(SOUNDMANAGER_SOUND_TYPE type);

signals:
    ProgramSetting GetProgramSettings();

private:
    QAudioOutput *_audioOutput;
    QMediaPlayer *_mediaPlayer;

};

#endif // SOUNDMANAGER_H
