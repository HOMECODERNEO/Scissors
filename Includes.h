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
#include <QLineEdit>
#include <QMimeData>
#include <QSettings>
#include <QComboBox>
#include <QRunnable>
#include <QClipboard>
#include <QThreadPool>
#include <QFileDialog>
#include <QJsonObject>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QColorSpace>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QProgressBar>
#include <QPainterPath>
#include <QColorDialog>
#include <QImageReader>
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

enum SOUNDMANAGER_SOUND_TYPE{
    SOUND_TYPE_POPUP,
    SOUND_TYPE_PRINTSCREEN,
    SOUND_TYPE_REMOVE_FILE,
    SOUND_TYPE_BUTTON_HOVER,
    SOUND_TYPE_BUTTON_CLICK,
    SOUND_TYPE_RECYCLE_CLEANING,
    SOUND_TYPE_GET_CLIPBOARD_IMAGE
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////// SAVEMANAGERFILEDATA //////////////////////////
//////////////////////////////////////////////////////////////////////////

class SaveManagerFileData{

    private:
        QByteArray _imageData;
        int _imageID = 0, _fileNameID = -1;
        bool _null = true, _imageHidden = false;

    public:
        explicit SaveManagerFileData(int id = -1, QPixmap image = QPixmap(), int fileNameID = -1, bool hidden = false){
            SetID(id);
            SetImage(image);
            SetFileNameID(_fileNameID);

            SetHidden(hidden);

            if(id == -1 && image.isNull() && fileNameID == -1)
                _null = true;
        }

        ////////////////////////////////////////////////////////////////////////

        void SetID(int id){ _imageID = id; _null = false; }
        void SetHidden(bool hiden){ _imageHidden = hiden; _null = false; }
        void SetData(QByteArray data){ _imageData = data; _null = false; }
        void SetFileNameID(int fileNameID){ _fileNameID = fileNameID; _null = false; }

        void SetImage(QPixmap pixmap){
            QByteArray _data;
            QBuffer buffer(&_data);

            if(buffer.open(QIODevice::WriteOnly)){
                if(pixmap.save(&buffer, "PNG")){
                    SetData(_data);

                    _null = false;
                    return;
                }
            }

            SetData(QByteArray());
        }

        void SetImage(QImage image){
            QByteArray _data;
            QBuffer buffer(&_data);

            if(buffer.open(QIODevice::WriteOnly)){
                if(image.save(&buffer, "PNG")){
                    SetData(_data);

                    _null = false;
                    return;
                }
            }

            SetData(QByteArray());
        }

        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////

        int GetID() const { return _imageID; }
        bool GetNull() const { return _null; }
        bool GetHidden() const { return _imageHidden; }
        int GetFileNameID() const { return _fileNameID; }
        const QByteArray& GetData() const { return _imageData; }

        QPixmap GetPixmap() const{
            QPixmap buffer;

            if(buffer.loadFromData(GetData(),"PNG"))
                return buffer;
            else
                return QPixmap();
        }

        QImage GetImage() const{
            QImage buffer;

            if(buffer.loadFromData(GetData(),"PNG"))
                return buffer;
            else
                return QImage();
        }

        ////////////////////////////////////////////////////////////////////////

        QByteArray CombineMetaData(){
            QByteArray result;
            QDataStream stream(&result, QIODevice::WriteOnly);

            stream << GetID();
            stream << GetFileNameID();
            stream << GetHidden();

            return result;
        }

        void DecoupleMetaData(QByteArray data){
            QDataStream stream(&data, QIODevice::ReadOnly);

            int id, fileID;
            bool hidden;

            stream >> id;
            stream >> fileID;
            stream >> hidden;

            SetID(id);
            SetFileNameID(fileID);
            SetHidden(hidden);
        }

        ////////////////////////////////////////////////////////////////////////

        // Переопределение операции сравнения двох классов
        bool operator == (const SaveManagerFileData& other) const{
            return (_imageID == other._imageID &&
                    _imageHidden == other._imageHidden &&
                    _imageData == other._imageData &&
                    _fileNameID == other._fileNameID);
        }

        bool operator != (const SaveManagerFileData& other) const{
            return !(*this == other);
        }

        ////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////// PROGTAMSETTING ////////////////////////////
//////////////////////////////////////////////////////////////////////////

class ProgramSetting{
    private:
        byte _historySize = 0;

        bool _startup = false;
        bool _useSounds = false;
        bool _showShortcut = false;
        bool _useStopFrame = false;
        bool _startupMessage = false;
        bool _viewerShowPercent = false;
        bool _historySmoothScroll = false;
        bool _showScreenshotZoneGeometry = false;
        bool _useModificationScreenShotArea = false;

        float _volumeDeleteFile = 0.0f;
        float _volumeButtonClick = 0.0f;
        float _volumeButtonHover = 0.0f;
        float _volumeNotification = 0.0f;
        float _volumeHistoryClear = 0.0f;
        float _volumeMakeScreenshot = 0.0f;
        float _volumeClipboardGetImage = 0.0f;

        int _hookPrtSc_Timeout = 0;

        QString _language = "English";
        QRect _settingsWindowGeometry;

    public:        
        void Set_UseSound(bool use){ _useSounds = use; }
        void Set_Startup(bool status){ _startup = status; } 
        void Set_StopFrame(bool use){ _useStopFrame = use; }
        void Set_HistorySize(byte size){ _historySize = size; }
        void Set_ShowShortcut(bool show){ _showShortcut = show; }
        void Set_PrtSc_Timeout(int ms){ _hookPrtSc_Timeout = ms; }
        void Set_StartupMessage(bool show){ _startupMessage = show; }
        void Set_ViewerShowPercent(bool use){ _viewerShowPercent = use; }
        void Set_ProgramLanguage(QString language){ _language = language; }
        void Set_VolumeDeleteFile(float value){ _volumeDeleteFile = value; }
        void Set_HistorySmoothScroll(bool use){ _historySmoothScroll = use; }
        void Set_VolumeButtonClick(float value){ _volumeButtonClick = value; }
        void Set_VolumeButtonHover(float value){ _volumeButtonHover = value; }
        void Set_VolumeHistoryClear(float value){ _volumeHistoryClear = value; }
        void Set_VolumeNotification(float value){ _volumeNotification = value; }
        void Set_VolumeMakeScreenshot(float value){ _volumeMakeScreenshot = value; }
        void Set_ShowModificationArea(bool use){ _useModificationScreenShotArea = use; }
        void Set_VolumeClipboardGetImage(float value) { _volumeClipboardGetImage = value; }
        void Set_ShowScreenshotZoneGeometry(bool show){ _showScreenshotZoneGeometry = show; }
        void Set_SettingsWindowGeometry(QRect geometry){ _settingsWindowGeometry = geometry; }

        bool Get_Startup(){ return _startup; }
        bool Get_UseSound(){ return _useSounds; }
        bool Get_StopFrame(){ return _useStopFrame; }
        byte Get_HistorySize(){ return _historySize; }
        bool Get_ShowShortcut(){ return _showShortcut; }
        QString Get_ProgramLanguage(){ return _language; }
        bool Get_StartupMessage(){ return _startupMessage; }
        int Get_PrtSc_Timeout(){ return _hookPrtSc_Timeout; }
        float Get_VolumeDeleteFile(){ return _volumeDeleteFile; }
        bool Get_ViewerShowPercent(){ return _viewerShowPercent; }
        float Get_VolumeButtonClick(){ return _volumeButtonClick; }
        float Get_VolumeButtonHover(){ return _volumeButtonHover; }
        float Get_VolumeHistoryClear(){ return _volumeHistoryClear; }
        float Get_VolumeNotification(){ return _volumeNotification; }
        bool Get_HistorySmoothScroll(){ return _historySmoothScroll; }
        float Get_VolumeMakeScreenshot(){ return _volumeMakeScreenshot; }
        QRect Get_SettingsWindowGeometry(){ return _settingsWindowGeometry; }
        float Get_VolumeClipboardGetImage() { return _volumeClipboardGetImage; }
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
