#include "soundmanager.h"

SoundManager::SoundManager(QObject *parent): QObject{ parent }{

    _audioOutput = new QAudioOutput();
    _mediaPlayer = new QMediaPlayer();
    _mediaPlayer->setAudioOutput(_audioOutput);
}

void SoundManager::PlaySound(SOUNDMANAGER_SOUND_TYPE type){
    if(!emit GetProgramSettings().Get_UseSound())
        return;

    _mediaPlayer->stop();
    _mediaPlayer->setSource(QUrl());

    switch(type){
        case SOUND_TYPE_REMOVE_FILE:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/remove_file.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeDeleteFile() / 100);
            break;
        }

        case SOUND_TYPE_RECYCLE_CLEANING:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/recycle_cleaning.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeHistoryClear() / 100);
            break;
        }

        case SOUND_TYPE_PRINTSCREEN:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/printscreen.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeMakeScreenshot() / 100);
            break;
        }

        case SOUND_TYPE_POPUP:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/popup.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeNotification() / 100);
            break;
        }

        case SOUND_TYPE_GET_CLIPBOARD_IMAGE:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/clipboard_image_add.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeClipboardGetImage() / 100);
            break;
        }

        case SOUND_TYPE_BUTTON_HOVER:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/button_hover.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeButtonHover() / 100);
            break;
        }

        case SOUND_TYPE_BUTTON_CLICK:{
            _mediaPlayer->setSource(QUrl("qrc:/Sounds/Resourse/Sounds/button_click.wav"));
            _audioOutput->setVolume(emit GetProgramSettings().Get_VolumeButtonClick() / 100);
            break;
        }
    }

    _mediaPlayer->play();
}
