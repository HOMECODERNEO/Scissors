#ifndef INCLUDES_H
#define INCLUDES_H

#include <Defines.h>

#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QLabel>
#include <QQueue>
#include <QTimer>
#include <QThread>
#include <QPixmap>
#include <QScreen>
#include <QBuffer>
#include <QPainter>
#include <QMimeData>
#include <QSettings>
#include <QComboBox>
#include <QClipboard>
#include <QFileDialog>
#include <QJsonObject>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QImageReader>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QApplication>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
/////////////////////////////
#include <animationsmanager.h> //#include<QWidget, QObject, QPropertyAnimation, QGraphicsOpacityEffect>
/////////////////////////////
////////////////////
#include <Windows.h>
#include <QDebug>

class SaveManagerFileData{

    private:
        int _imageID;
        QByteArray _imageData;
        QByteArray _fileOldName;

    public:
        explicit SaveManagerFileData(int id = 0, QPixmap image = QPixmap()){
            SetID(id);
            SetImage(image);
        }

        ////////////////////////////////////////////////////////////////////////

        void SetID(byte id){ _imageID = id; }
        void SetData(QByteArray image){ _imageData = image; }
        void SetFileOldName(QString name){ _fileOldName = name.toUtf8(); }
        void SetImage(QPixmap image){
            QByteArray _data;
            QBuffer buffer(&_data);

            if(buffer.open(QIODevice::WriteOnly)){
                if(image.save(&buffer, "PNG")){
                    SetData(_data);
                    return;
                }
            }

            SetData(QByteArray());
        }

        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////

        int GetID() const{ return _imageID; }
        QByteArray GetData(){ return _imageData; }
        QString GetFileOldName(){ return QString::fromUtf8(_fileOldName); }
        QPixmap GetImage(){ return QPixmap::fromImage(QImage::fromData(GetData(), "PNG")); }

        ////////////////////////////////////////////////////////////////////////

        QByteArray CombineData(){
            QByteArray result;
            QDataStream stream(&result, QIODevice::WriteOnly);
            stream << GetID();
            result.append(GetData());

            return result;
        }

        void DecoupleData(QByteArray data){
            QDataStream stream(&data, QIODevice::ReadOnly);

            int id;
            stream >> id;
            SetID(id);

            SetData(data.mid(sizeof(int)));
        }
};

class ProgramSetting{
    private:
        byte _historySize = 0;

        bool _startup = false;
        bool _useSounds = false;
        bool _useStopFrame = false;
        bool _viewerShowPercent = false;
        bool _showScreenshotZoneGeometry = false;
        bool _useModificationScreenShotArea = false;

        float _volumeDeleteFile = 0;
        float _volumeNotification = 0;
        float _volumeHistoryClear = 0;
        float _volumeMakeScreenshot = 0;

        int _hookPrtSc_Timeout = 0;

        QString _language = "ENG";
        QRect _settingsWindowGeometry;

    public:        
        void Set_UseSound(bool use){ _useSounds = use; }
        void Set_Startup(bool status){ _startup = status; }
        void Set_StopFrame(bool use){ _useStopFrame = use; }
        void Set_HistorySize(byte size){ _historySize = size; }
        void Set_PrtSc_Timeout(int ms){ _hookPrtSc_Timeout = ms; }
        void Set_ViewerShowPercent(bool use){ _viewerShowPercent = use; }
        void Set_ProgramLanguage(QString language){ _language = language; }
        void Set_VolumeDeleteFile(float value){ _volumeDeleteFile = value; }
        void Set_VolumeHistoryClear(float value){ _volumeHistoryClear = value; }
        void Set_VolumeNotification(float value){ _volumeNotification = value; }
        void Set_VolumeMakeScreenshot(float value){ _volumeMakeScreenshot = value; }
        void Set_ShowModificationArea(bool use){ _useModificationScreenShotArea = use; }
        void Set_ShowScreenshotZoneGeometry(bool show){ _showScreenshotZoneGeometry = show; }
        void Set_SettingsWindowGeometry(QRect geometry){ _settingsWindowGeometry = geometry; }

        bool Get_Startup(){ return _startup; }
        bool Get_UseSound(){ return _useSounds; }
        bool Get_StopFrame(){ return _useStopFrame; }
        byte Get_HistorySize(){ return _historySize; }
        QString Get_ProgramLanguage(){ return _language; }
        int Get_PrtSc_Timeout(){ return _hookPrtSc_Timeout; }
        float Get_VolumeDeleteFile(){ return _volumeDeleteFile; }
        bool Get_ViewerShowPercent(){ return _viewerShowPercent; }
        float Get_VolumeHistoryClear(){ return _volumeHistoryClear; }
        float Get_VolumeNotification(){ return _volumeNotification; }
        float Get_VolumeMakeScreenshot(){ return _volumeMakeScreenshot; }
        QRect Get_SettingsWindowGeometry(){ return _settingsWindowGeometry; }
        bool Get_ShowModificationArea(){ return _useModificationScreenShotArea; }
        bool Get_ShowScreenshotZoneGeometry(){ return _showScreenshotZoneGeometry; }
};

class TranslateData{

    private:
        QMap<QString, QString> _language;

    public:
        void Clear(){
            _language.clear();
        }

        void Set(QString key, QString translate){
            _language[key] = translate;
        }

        QString translate(const QString &key){
            return _language.value(key, "NOT_FOUND");
        }

};

#endif // INCLUDES_H
