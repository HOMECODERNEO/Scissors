#include "savemanager.h"

// Конструктор
SaveManager::SaveManager(QObject *parent, std::function<void(byte, QList<SaveManagerFileData>, ProgramSetting)> operationEnd_func) : QObject(parent) {
    if (operationEnd_func){
        connect(this, &SaveManager::OperationEnd, [operationEnd_func](byte type, QList<SaveManagerFileData> _listData, ProgramSetting _programSettings){
            operationEnd_func(type, _listData, _programSettings);
        });
    }
}

// Сохраняем настройки программы
void SaveManager::SaveSettings(ProgramSetting data){

    QSettings PortSetting(QApplication::applicationDirPath() + SETTINGS_DIR + SETTINGS_FILE, QSettings::IniFormat);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("GENERAL");
    PortSetting.setValue("Startup", data.Get_Startup());
    PortSetting.setValue("PrtScTimeOut", data.Get_PrtSc_Timeout());
    PortSetting.setValue("Lang", data.Get_ProgramLanguage());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("SOUND");
    PortSetting.setValue("Sounds", data.Get_UseSound());

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.setValue("Notification", data.Get_VolumeNotification());
    PortSetting.setValue("HistoryClear", data.Get_VolumeHistoryClear());
    PortSetting.setValue("DeleteFile", data.Get_VolumeDeleteFile());
    PortSetting.setValue("MakeScreenshot", data.Get_VolumeMakeScreenshot());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("HISTORY");
    PortSetting.setValue("HistorySize", data.Get_HistorySize());
    PortSetting.setValue("ViewShowPercent", data.Get_ViewerShowPercent());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("CAPTURE");
    PortSetting.setValue("UseStopFrame", data.Get_StopFrame());
    PortSetting.setValue("ShowZoneSize", data.Get_ShowScreenshotZoneGeometry());
    PortSetting.setValue("ModificationScreenShotArea", data.Get_ShowModificationArea());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("DATA");
    PortSetting.setValue("FormSettings", data.Get_SettingsWindowGeometry());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.sync();
}

// Загружаем настройки программы
void SaveManager::LoadSettings(){

    ProgramSetting _settingsData;
    QSettings PortSetting(QApplication::applicationDirPath() + SETTINGS_DIR + SETTINGS_FILE, QSettings::IniFormat);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("GENERAL");
    _settingsData.Set_Startup(PortSetting.value("Startup", false).toBool());
    _settingsData.Set_PrtSc_Timeout(PortSetting.value("PrtScTimeOut", 350).toInt());
    _settingsData.Set_ProgramLanguage(PortSetting.value("Lang", "English").toString());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("SOUND");
    _settingsData.Set_UseSound(PortSetting.value("Sounds", true).toBool());

    _settingsData.Set_VolumeNotification(PortSetting.value("Notification", 50).toFloat());
    _settingsData.Set_VolumeHistoryClear(PortSetting.value("HistoryClear", 50).toFloat());
    _settingsData.Set_VolumeDeleteFile(PortSetting.value("DeleteFile", 50).toFloat());
    _settingsData.Set_VolumeMakeScreenshot(PortSetting.value("MakeScreenshot", 50).toFloat());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("HISTORY");
    _settingsData.Set_HistorySize(PortSetting.value("HistorySize", 10).toInt());
    _settingsData.Set_ViewerShowPercent(PortSetting.value("ViewShowPercent", true).toBool());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("CAPTURE");
    _settingsData.Set_StopFrame(PortSetting.value("UseStopFrame", true).toBool());
    _settingsData.Set_ShowScreenshotZoneGeometry(PortSetting.value("ShowZoneSize", true).toBool());
    _settingsData.Set_ShowModificationArea(PortSetting.value("ModificationScreenShotArea", true).toBool());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("DATA");
    _settingsData.Set_SettingsWindowGeometry(PortSetting.value("FormSettings", "").toRect());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    emit OperationEnd(1, QList<SaveManagerFileData>(), _settingsData);
}

// Удаляем файл с данными изображения
void SaveManager::RemoveHistoryFile(QString hash){
    QFile checkFile(QApplication::applicationDirPath() + "/cache/" + hash + ".cache");

    checkFile.remove();
}

// Сохраняем историю скриншотов
void SaveManager::SaveHistory(QList<SaveManagerFileData> &data){

    // Путь сохранения
    QString savePath = QApplication::applicationDirPath() + "/cache/";

    // Проверка существованния директории, и ее создание если ее нет
    if(!QDir(savePath).exists())
        QDir().mkdir(savePath);

    // Перебираем весь список для сохранения
    QString fileHash;
    QByteArray _saveData, encryptionKey, encryptedEntity;

    for(int i = 0; i < data.count(); i++){
        _saveData = data[i].CombineData();

        // Формирование названия файла по его хэш-сумме
        fileHash = QCryptographicHash::hash(_saveData, QCryptographicHash::Sha256).toHex();

        // Проверяем наличие старого файла
        QFile checkFile(savePath + data[i].GetFileOldName() + ".cache");
        checkFile.remove();

        // Создание и открытие файла для записи
        QFile file(savePath + fileHash + ".cache");
        if(file.open(QIODevice::WriteOnly)){
            encryptionKey = QCryptographicHash::hash(ALGORITHM_KEY, QCryptographicHash::Sha256);
            encryptedEntity = Cipher(_saveData, encryptionKey);

            data[i].SetFileOldName(fileHash);

            file.write(encryptedEntity);
            file.close();
        }
    }
}

// Загружаем историю скриншотов
void SaveManager::LoadHistory(){
    QByteArray readFileData;
    QList<SaveManagerFileData> data;
    SaveManagerFileData fileDataBuff;

    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + "/cache/";

    // Проверяем по пути загрузки лежащие файлы с расширением *.cache и формируем список количества файлов
    QDir folder(loadPath);
    folder.setNameFilters(QStringList("*.cache"));

    // Проходимся по каждому файлу для его проверки
    foreach(QString fileName, folder.entryList()){
        QFile file(folder.filePath(fileName));

        // Открываем для чтения
        if(file.open(QIODevice::ReadOnly)){
            readFileData = file.readAll();
            file.close();

            // Извлекаем хеш-сумму из имени файла
            QString hashInFileName = fileName.split(".")[0];

            // Расшифровываем данные с файла с помощью ключа
            QByteArray decryptedData = Cipher(readFileData, QCryptographicHash::hash(ALGORITHM_KEY, QCryptographicHash::Sha256));

            // Извлекаем данные из расшифрованного файла
            fileDataBuff.DecoupleData(decryptedData);

            // Проверяем названия файла с его хэш-суммой в нём
            if(hashInFileName == QCryptographicHash::hash(decryptedData, QCryptographicHash::Sha256).toHex()){

                fileDataBuff.SetFileOldName(hashInFileName);
                data.append(fileDataBuff);

            }else{

                QFile checkFile(loadPath + hashInFileName + ".cache");
                checkFile.remove();
            }
        }
    }

    // Сортируем по ID
    std::sort(data.begin(), data.end(), [](const SaveManagerFileData& data1, const SaveManagerFileData& data2){
        return data1.GetID() < data2.GetID();
    });

    emit OperationEnd(0, data, ProgramSetting());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Простая функция шифрования методом XOR (Работает в оба направления шифрования и расшифрования)
QByteArray SaveManager::Cipher(const QByteArray& data, const QByteArray& key){
    QByteArray encryptedData = data;

    for(int i = 0; i < data.size(); ++i){
        encryptedData[i] = data[i] ^ key[i % key.size()];
    }

    return encryptedData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
