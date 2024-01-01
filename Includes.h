#ifndef INCLUDES_H
#define INCLUDES_H

#include <Defines.h>

#include <QDir>
#include <QtWin>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QScreen>
#include <QBuffer>
#include <QPainter>
#include <QSettings>
#include <QComboBox>
#include <QClipboard>
#include <QFileDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QCryptographicHash>
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
        bool _useStopFrame = false;
        bool _showScreenshotZoneGeometry = false;
        bool _useModificationScreenShotArea = false;
        QString _language = "ENG";

    public:
        void Set_Startup(bool status){ _startup = status; }
        void Set_StopFrame(bool use){ _useStopFrame = use; }
        void Set_HistorySize(byte size){ _historySize = size; }
        void Set_ProgramLanguage(QString language){ _language = language; }
        void Set_ShowModificationArea(bool use){ _useModificationScreenShotArea = use; }
        void Set_ShowScreenshotZoneGeometry(bool show){ _showScreenshotZoneGeometry = show; }

        bool Get_Startup(){ return _startup; }
        bool Get_StopFrame(){ return _useStopFrame; }
        byte Get_HistorySize(){ return _historySize; }
        QString Get_ProgramLanguage(){ return _language; }
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
            return _language.value(key, "not found");
        }

};

#endif // INCLUDES_H
