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

    PortSetting.beginGroup("GENERAL");
    PortSetting.setValue("Startup", data.Get_Startup());
    PortSetting.setValue("ShowZoneSize", data.Get_ShowScreenshotZoneGeometry());
    PortSetting.setValue("UseStopFrame", data.Get_StopFrame());
    PortSetting.setValue("HistorySize", data.Get_HistorySize());
    PortSetting.setValue("ModificationScreenShotArea", data.Get_ShowModificationArea());
    PortSetting.endGroup();

    PortSetting.beginGroup("LANGUAGE");
    PortSetting.setValue("Lang", data.Get_ProgramLanguage());
    PortSetting.endGroup();

    PortSetting.sync();
}

// Загружаем настройки программы
void SaveManager::LoadSettings(){

    ProgramSetting _settingsData;
    QSettings PortSetting(QApplication::applicationDirPath() + SETTINGS_DIR + SETTINGS_FILE, QSettings::IniFormat);

    PortSetting.beginGroup("GENERAL");
    _settingsData.Set_Startup(PortSetting.value("Startup", false).toBool());
    _settingsData.Set_ShowScreenshotZoneGeometry(PortSetting.value("ShowZoneSize", false).toBool());
    _settingsData.Set_StopFrame(PortSetting.value("UseStopFrame", false).toBool());
    _settingsData.Set_HistorySize(PortSetting.value("HistorySize", 5).toInt());
    _settingsData.Set_ShowModificationArea(PortSetting.value("ModificationScreenShotArea", false).toBool());
    PortSetting.endGroup();

    PortSetting.beginGroup("LANGUAGE");
    _settingsData.Set_ProgramLanguage(PortSetting.value("Lang", "ENG").toString());
    PortSetting.endGroup();

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
