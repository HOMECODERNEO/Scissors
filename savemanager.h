#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <Includes.h>

class SaveManager : public QObject{
    Q_OBJECT

signals:
    void OperationEnd(byte type, QList<SaveManagerFileData> _listData, ProgramSetting _settingsData);

public:
    explicit SaveManager(QObject *parent = nullptr, std::function<void(byte, QList<SaveManagerFileData>, ProgramSetting)> operationEnd_func = nullptr);

    void LoadHistory();
    void SaveHistory(QList<SaveManagerFileData> &data);

    void LoadSettings();
    void SaveSettings(ProgramSetting data);

public slots:
    void RemoveHistoryFile(QString hash);

private:
    QByteArray Cipher(const QByteArray& data, const QByteArray& key);

};

#endif // SAVEMANAGER_H
