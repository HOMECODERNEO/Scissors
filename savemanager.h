#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <Includes.h>

struct SaveManagerData;

class SaveManager : public QObject{
    Q_OBJECT

signals:
    void OperationEnd(SaveManagerData Data);
    void Event_LoadLanguage(TranslateData data);

public:
    void ProgramExit();
    explicit SaveManager(QObject *parent = nullptr, std::function<void(SaveManagerData returnData)> operationEnd_func = nullptr);

    // HISTORY
    void LoadHistory();
    void SaveHistory(QList<SaveManagerFileData> &data);

    // SETTINGS
    void LoadSettings();
    void SaveSettings(ProgramSetting data);

    // TRANSLATE
    void LoadTranslations(QString lang, QComboBox *box);
    void ChangeProgramLanguage(TranslateData* container, QString lang);

    // PENDASHPATTERNS
    QMap<QString, QVector<QVector<qreal>>> LoadPenDashPatterns();

    //BRUSHPATTERNS
    void LoadBrushPatterns(QComboBox *box);

    // CHECK VERSION
    void CheckProgramNewVersion();

public slots:
    QBitmap LoadBrushPatternAtIndex(int index);

private:
    // OTHER
    QByteArray Cipher(const QByteArray& data, const QByteArray& key);

    // HISTORY
    QList<int> FindFreeIDs(const QSet<int> &usedIDs, int count);

    void WriteMetaDataFile(QString savePath, QByteArray data);
    QList<SaveManagerFileData> ReadMetaDataFile(QString filePath);

    // VERSION CHECK
    void onReplyFinished(QNetworkReply *reply);

private:
    QFile *_brushPatternsLock;
    QNetworkAccessManager *_networkChecker;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////// SAVEMANAGERDATA /////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct SaveManagerData{

    enum SAVEMANAGER_OPERATIONS{
        SAVEMANAGER_HISTORY_LOADED,
        SAVEMANAGER_SETTINGS_LOADED,
        SAVEMANAGER_VERSIONCHECKER_DONE
    };

    SaveManagerData(SAVEMANAGER_OPERATIONS type){
        SetReturnType(type);
    }

    // RETURN TYPE
    void SetReturnType(SAVEMANAGER_OPERATIONS type){ _returnType = type; }
    SAVEMANAGER_OPERATIONS GetReturnType(){ return _returnType; }

    // HISTORY
    void SetHistoryData(QList<SaveManagerFileData> data){ _historyData = data; }
    QList<SaveManagerFileData> GetHistoryData(){ return _historyData; }

    //SETTINGS
    void SetProgramSettings(ProgramSetting data){ _settingsData = data; }
    ProgramSetting GetProgramSettings(){ return _settingsData; }

    // VERSION CHECKER
    void SetVersionCheckerParam(byte type, QString mainMessage, QString secondaryMessage, QString windowMessage, QString url){
        _versionCheckerUrl = url;
        _versionCheckerType = type;
        _versionCheckerMainMessage = mainMessage;
        _versionCheckerWindowMessage = windowMessage;
        _versionCheckerSecondaryMessage = secondaryMessage;
    }

    byte GetVersionCheckerType() const { return _versionCheckerType; }
    QString GetVersionCheckerUrl() const { return _versionCheckerUrl; }
    QString GetVersionCheckerMessage() const { return _versionCheckerMainMessage; }
    QString GetVersionCheckerWindowMessage() const { return _versionCheckerWindowMessage; }
    QString GetVersionCheckerMessageAdditional() const { return _versionCheckerSecondaryMessage; }

private:
    byte _versionCheckerType = 0;
    QString _versionCheckerMainMessage, _versionCheckerSecondaryMessage, _versionCheckerWindowMessage, _versionCheckerUrl;

    ProgramSetting _settingsData;
    SAVEMANAGER_OPERATIONS _returnType;
    QList<SaveManagerFileData> _historyData;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////////// END ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // SAVEMANAGER_H
