#include "savemanager.h"

// Конструктор
SaveManager::SaveManager(QObject *parent, std::function<void(SaveManagerData returnData)> operationEnd_func) : QObject(parent) {
    if (operationEnd_func){
        connect(this, &SaveManager::OperationEnd, [operationEnd_func](SaveManagerData returnData){
            operationEnd_func(returnData);
        });
    }

    _networkChecker = new QNetworkAccessManager(this);
    connect(_networkChecker, &QNetworkAccessManager::finished, this, &SaveManager::onReplyFinished);

    // Контрольная проверка и создания директорий если они отсутствуют
    QString dataPath = QApplication::applicationDirPath() + MAIN_DATA_PATH;

    _brushPatternsLock = new QFile(dataPath + BRUSH_PATTERNS_PATH + "OperatingData.cache");
    _brushPatternsLock->open(QIODevice::ReadWrite | QIODevice::Text);

    if(!QDir(dataPath).exists()) QDir().mkdir(dataPath);                                                    // Если нету создаем папку
    if(!QDir(dataPath + CACHE_PATH).exists()) QDir().mkdir(dataPath + CACHE_PATH);                          // Папка хранения кэша
    if(!QDir(dataPath + LANGUAGE_PATH).exists()) QDir().mkdir(dataPath + LANGUAGE_PATH);                    // Папка переводов
    if(!QDir(dataPath + BRUSH_PATTERNS_PATH).exists()) QDir().mkdir(dataPath + BRUSH_PATTERNS_PATH);        // Папка паттернов кисти
    if(!QDir(dataPath + PEN_DASHPATTERNS_PATH).exists()) QDir().mkdir(dataPath + PEN_DASHPATTERNS_PATH);    // Папка паттернов линий
}

// Сигнал от основной программы об её закрытии
void SaveManager::ProgramExit(){
    // Закрываем буферный файл данных
    _brushPatternsLock->close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// SETTING //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Сохраняем настройки программы
void SaveManager::SaveSettings(ProgramSetting data){

    QSettings PortSetting(QApplication::applicationDirPath() + MAIN_DATA_PATH + SETTINGS_FILETYPE, QSettings::IniFormat);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("GENERAL");
    PortSetting.setValue("Startup", data.Get_Startup());
    PortSetting.setValue("StartupMessage", data.Get_StartupMessage());
    PortSetting.setValue("ShowShortcut", data.Get_ShowShortcut());
    PortSetting.setValue("PrtScTimeOut", data.Get_PrtSc_Timeout());
    PortSetting.setValue("HistorySmoothScroll", data.Get_HistorySmoothScroll());
    PortSetting.setValue("Lang", data.Get_ProgramLanguage());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("SOUND");
    PortSetting.setValue("Sounds", data.Get_UseSound());

    PortSetting.setValue("Notification", data.Get_VolumeNotification());
    PortSetting.setValue("HistoryClear", data.Get_VolumeHistoryClear());
    PortSetting.setValue("DeleteFile", data.Get_VolumeDeleteFile());
    PortSetting.setValue("MakeScreenshot", data.Get_VolumeMakeScreenshot());
    PortSetting.setValue("ButtonHover", data.Get_VolumeButtonHover());
    PortSetting.setValue("ButtonClick", data.Get_VolumeButtonClick());
    PortSetting.setValue("ClipboardGetImage", data.Get_VolumeClipboardGetImage());
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

    QSettings PortSetting(QApplication::applicationDirPath() + MAIN_DATA_PATH + SETTINGS_FILETYPE, QSettings::IniFormat);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("GENERAL");
    _settingsData.Set_Startup(PortSetting.value("Startup", false).toBool());
    _settingsData.Set_StartupMessage(PortSetting.value("StartupMessage", true).toBool());
    _settingsData.Set_ShowShortcut(PortSetting.value("ShowShortcut", true).toBool());
    _settingsData.Set_PrtSc_Timeout(PortSetting.value("PrtScTimeOut", 350).toInt());
    _settingsData.Set_HistorySmoothScroll(PortSetting.value("HistorySmoothScroll", false).toBool());
    _settingsData.Set_ProgramLanguage(PortSetting.value("Lang", "English").toString());
    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("SOUND");
    _settingsData.Set_UseSound(PortSetting.value("Sounds", true).toBool());

    _settingsData.Set_VolumeNotification(PortSetting.value("Notification", 50).toFloat());
    _settingsData.Set_VolumeHistoryClear(PortSetting.value("HistoryClear", 50).toFloat());
    _settingsData.Set_VolumeDeleteFile(PortSetting.value("DeleteFile", 50).toFloat());
    _settingsData.Set_VolumeMakeScreenshot(PortSetting.value("MakeScreenshot", 50).toFloat());
    _settingsData.Set_VolumeButtonHover(PortSetting.value("ButtonHover", 50).toFloat());
    _settingsData.Set_VolumeButtonClick(PortSetting.value("ButtonClick", 50).toFloat());
    _settingsData.Set_VolumeClipboardGetImage(PortSetting.value("ClipboardGetImage", 50).toFloat());

    PortSetting.endGroup();

    ////////////////////////////////////////////////////////////////////////////////////////////////

    PortSetting.beginGroup("HISTORY");
    _settingsData.Set_HistorySize(PortSetting.value("HistorySize", 100).toInt());
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

    SaveManagerData returnData(SaveManagerData::SAVEMANAGER_SETTINGS_LOADED);
    returnData.SetProgramSettings(_settingsData);

    emit OperationEnd(returnData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// HISTORY //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Сохраняем историю скриншотов
void SaveManager::SaveHistory(QList<SaveManagerFileData> &data){

    // Путь сохранения
    QString savePath = QApplication::applicationDirPath() + MAIN_DATA_PATH + CACHE_PATH;

    // Проверка существованния директории, и ее создание если ее нет
    if(!QDir(savePath).exists())
        QDir().mkdir(savePath);

    if(QFile(savePath + "MetaData.cache").exists()){ // Обновляем данные

        QSet<int> currentFileIDs, newFileIDs;
        QList<SaveManagerFileData *> saveList;

        // Читаем МетаДанные
        QList<SaveManagerFileData> dataList = ReadMetaDataFile(savePath + "MetaData.cache");

        ////////////////////////////////////////////////////////////////////////// Новый файл изображения

        // Идём по циклу новых данных, и смотрим наличие новых скриншотов которые нужно сохранять
        for(auto &item : data){
            if(item.GetFileNameID() == -1)
                saveList.append(&item);                     // Новый скриншот который нужно сохранить (не имеет ID файла сохранения)
            else
                newFileIDs.insert(item.GetFileNameID());    // Вставляем существующие id файлов для отсекания в алгоритме дальше
        }

        // Формируем список уже существующих ID файлов
        for(auto &item : dataList)
            currentFileIDs.insert(item.GetFileNameID());

        int dataListCount = data.count();
        int saveListCount = saveList.count();

        // Получаем список ID свободных мест среди файлов
        QList<int> freeIDs = FindFreeIDs(currentFileIDs, saveListCount);

        // Проганяем список и сохраняем все новые скриншоты
        for(int iterator = 0; iterator < saveListCount; iterator++){
            int newID = freeIDs.at(iterator);

            QFile file(savePath + QString::number(newID) + ".cache");

            if(file.open(QIODevice::WriteOnly)){
                saveList.at(iterator)->SetFileNameID(newID);

                file.write(saveList.at(iterator)->GetData());
                file.close();
            }
        }

        // Удаление с диска лишних отсутствующих файлов в списке данных
        QSet deleteBuffer = (currentFileIDs - newFileIDs);

        for(int id : deleteBuffer)
            QFile::remove(savePath + QString::number(id) + ".cache");

        // Удаляем файл и дальше не идём так как история была удалена до последнего скриншота
        if(dataListCount == 0){
            QFile::remove(savePath + "MetaData.cache");
        }else{
            QByteArray _saveMetaData;
            QBuffer writerMetaData(&_saveMetaData);
            writerMetaData.open(QIODevice::WriteOnly);
            QDataStream writerMetaDataOut(&writerMetaData);

            for(int i = 0; i < dataListCount; i++)
                writerMetaDataOut << data[i].CombineMetaData();

            writerMetaData.close();

            // Записываем МетаДанные
            WriteMetaDataFile(savePath + "MetaData.cache", _saveMetaData);
        }

    }else{ // Сохранение впервые

        QByteArray _saveMetaData;
        QBuffer writerMetaData(&_saveMetaData);
        writerMetaData.open(QIODevice::WriteOnly);
        QDataStream writerMetaDataOut(&writerMetaData);

        int dataListCount = data.count();

        for(int i = 0; i < dataListCount; i++){
            QFile file(savePath + QString::number(i) + ".cache");

            if(file.open(QIODevice::WriteOnly)){
                data[i].SetFileNameID(i);

                file.write(data[i].GetData());
                file.close();
            }

            writerMetaDataOut << data[i].CombineMetaData();
        }

        writerMetaData.close();

        // Записываем МетаДанные
        WriteMetaDataFile(savePath + "MetaData.cache", _saveMetaData);
    }
}

// Загружаем историю скриншотов
void SaveManager::LoadHistory(){

    // Путь чтения
    QString loadPath = QApplication::applicationDirPath() + MAIN_DATA_PATH + CACHE_PATH;

    QList<SaveManagerFileData> dataList;

    if(QFile(loadPath + "MetaData.cache").exists()){
        //Читаем МетаДанные
        dataList = ReadMetaDataFile(loadPath + "MetaData.cache");

        // Ищем файл изображения и читаем его с присвоением в класс изображения
        for(auto &data : dataList){
            QFile readImageFile(loadPath + QString::number(data.GetFileNameID()) + ".cache");

            if(readImageFile.open(QIODevice::ReadOnly)){
                data.SetData(readImageFile.readAll());
                readImageFile.close();
            }
        }

        // Сортируем по ID
        std::sort(dataList.begin(), dataList.end(), [](const SaveManagerFileData& data1, const SaveManagerFileData& data2){
            return data1.GetID() < data2.GetID();
        });
    }

    SaveManagerData returnData(SaveManagerData::SAVEMANAGER_HISTORY_LOADED);
    returnData.SetHistoryData(dataList);

    emit OperationEnd(returnData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// TRANSLATE /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Загружаем в форму все возможные файлы переводов программы
void SaveManager::LoadTranslations(QString lang, QComboBox *box){
    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + MAIN_DATA_PATH + LANGUAGE_PATH;

    // Проверяем по пути загрузки лежащие файлы с расширением *.tr и формируем список количества файлов
    QDir folder(loadPath);
    folder.setNameFilters(QStringList("*.tr"));

    // Если нету файлов то прекращаем дальнейший алгоритм заполнения
    if(!folder.exists())
        return;

    // Проходимся по каждому файлу для его добавления в элемент выбора языка
    box->blockSignals(true);

    foreach(QString fileName, folder.entryList())
        box->addItem(fileName.left(fileName.lastIndexOf('.')));

    box->blockSignals(false);

    // Выбираем сохранённый язык с настроек
    if(box->currentText() == lang){
        TranslateData* container = new TranslateData();
        ChangeProgramLanguage(container, lang);
    }else
        box->setCurrentText(lang);
}

// Загружаем конкретный файл перевода и его содержание
void SaveManager::ChangeProgramLanguage(TranslateData* container, QString lang){
    container->Clear();

    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + MAIN_DATA_PATH + LANGUAGE_PATH;

    // Папка или файл отсутствует, прекращаем дальнейший алгоритм загрузки
    if(!QDir(loadPath).exists() && !QDir(loadPath + lang + ".tr").exists())
        return;

    // Загружаем и читаем файл
    QSettings _lang(loadPath + lang + ".tr", QSettings::IniFormat);

    QStringList keys = _lang.allKeys();

    // Заполняем список переводов
    for(const QString &key : keys)
        container->Set(key, _lang.value(key).toString());

    // Вызываем применение перевода программы
    emit Event_LoadLanguage(*container);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// PENDASHPATTERNS ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

QMap<QString, QVector<QVector<qreal>>> SaveManager::LoadPenDashPatterns(){
    QString currentBlock;
    QVector<QVector<qreal>> sharedPatterns;
    QMap<QString, QVector<QVector<qreal>>> blockPatterns;

    // Путь файла
    QFile pattern(QApplication::applicationDirPath() + MAIN_DATA_PATH + PEN_DASHPATTERNS_PATH + PENDASHPATTERNS_FILETYPE);

    // Открываем файл с паттернами
    if(pattern.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&pattern);

        // Читаем построчно до конца файла
        while (!in.atEnd()){
            QString line = in.readLine().trimmed();

            // Если строка начинается с '[', это начало нового блока
            if(line.startsWith('[') && line.endsWith(']')){

                currentBlock = line.mid(1, line.length() - 2);

                if(currentBlock != "Shared")
                    blockPatterns[currentBlock] = {}; // Инициализируем новый блок

            }else if(!line.isEmpty() && !line.startsWith('#') && !currentBlock.isEmpty()){
                // Удаляем комментарий из строки, если он есть
                int commentIndex = line.indexOf('#');

                if (commentIndex != -1)
                    line = line.left(commentIndex).trimmed();

                // Разделяем строку по запятым
                QVector<qreal> pattern;
                QStringList numbers = line.split(',');

                for(const QString &number : numbers){

                    bool ok;
                    qreal value = number.toDouble(&ok);

                    if(ok)
                        pattern.append(value);

                }

                // Формируем список общих паттернов
                if(!pattern.isEmpty()){
                    if(currentBlock == "Shared")
                        sharedPatterns.append(pattern);
                    else
                        blockPatterns[currentBlock].append(pattern);
                }
            }
        }

        pattern.close();
    }

    // Добавляем паттерны из блока Shared ко всем остальным блокам
    for(auto &blockPatternsEntry : blockPatterns)
        for(const auto &pattern : sharedPatterns)
            blockPatternsEntry.append(pattern);

    return blockPatterns;
}

void SaveManager::LoadBrushPatterns(QComboBox *box){
    const int imageWidth = 150;
    const int imageHeight = 30;

    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + MAIN_DATA_PATH + BRUSH_PATTERNS_PATH;

    // Устанавливаем размер иконок
    box->setIconSize(QSize(imageWidth, imageHeight));

    // Добавляем пару стандартных паттернов
    QPixmap pattern(imageWidth, imageHeight);

    pattern.fill(QColor(255, 255, 255, 3));
    box->addItem(pattern, "");

    pattern.fill(QColor(255, 255, 255));
    box->addItem(pattern, "");

    // Проверяем по пути загрузки лежащие файлы с расширением *.png и формируем список количества файлов
    QDir folder(loadPath);
    folder.setNameFilters(QStringList("*.png"));

    // Если нету файлов то прекращаем дальнейший алгоритм заполнения
    if(!folder.exists())
        return;

    QStringList patternList = folder.entryList();
    int count = patternList.count();

    if(count <= 0)
        return;

    // Массивы для сравнения новой и старой информации в временном файле
    QByteArray oldFileContent, newFileContent;

    // Если старый файл существует то читаем его для сравнения содержания
    if(_brushPatternsLock->exists())
        oldFileContent = _brushPatternsLock->readAll();

    foreach(QString fileName, folder.entryList()){
        QPixmap loadPatternPixmap(loadPath + fileName);
        QBitmap maskPatternPixmap = loadPatternPixmap.createMaskFromColor(Qt::transparent);

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        maskPatternPixmap.save(&buffer, "PNG");

        // Заполняем массив данными
        newFileContent.append(byteArray.toBase64() + "\n");

        // Заполняем список паттернов
        box->addItem(loadPatternPixmap.copy(QRect(0, 0, imageWidth, imageHeight)), "");
    }

    // Если соержимое совпадает то перезапись не требуеться
    if (oldFileContent == newFileContent)
        return;

    // Перезаписываем информацию в файле
    if(!newFileContent.isEmpty()){
        _brushPatternsLock->resize(0);

        QTextStream save(_brushPatternsLock);
        save << newFileContent;

        _brushPatternsLock->flush();
    }
}

QBitmap SaveManager::LoadBrushPatternAtIndex(int index){
    int iterator = 0;
    QTextStream load(_brushPatternsLock);

    // Перемещаемся в начало всего файла
    _brushPatternsLock->seek(0);

    while(!load.atEnd()){
        QString line = load.readLine();
        QByteArray byteArray = QByteArray::fromBase64(line.toUtf8());

        QBitmap bitmap;
        bitmap.loadFromData(byteArray, "PNG");
        bitmap = bitmap.createMaskFromColor(Qt::white);

        if(!bitmap.isNull() && iterator == index)
            return bitmap;

        iterator++;
    }

    return QBitmap();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// CHECK UPDATE ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void SaveManager::CheckProgramNewVersion(){
    QNetworkRequest request(QUrl("https://api.github.com/repos/HOMECODERNEO/Scissors/releases/latest"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    _networkChecker->get(request);
}

QString parseGithubText(const QString& text) {
    QString htmlText;
    QStringList lines = text.split("\n");  // Разбиваем текст на строки

    bool inList = false;  // Флаг для отслеживания списка
    bool inNumberedList = false;

    htmlText += R"(<style>
                ul, ol{
                  margin-top: 15px;
                }

                li{
                  margin-bottom: 8px;
                }

                p{
                  margin-top: 10px;
                  line-height: 3.6;
                }

                </style>)";

    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();

        // Обработка заголовков
        if (trimmedLine.startsWith("Version")) {
            htmlText += "<h3>" + trimmedLine + "</h3>";
        }
        // Обработка элементов ненумерованного списка
        else if (trimmedLine.startsWith("*")) {
            if (!inList) {
                htmlText += "<ul>";  // Открываем список
                inList = true;
            }
            htmlText += "<li>" + trimmedLine.mid(1).trimmed() + "</li>";
        }
        // Обработка элементов нумерованного списка
        else if (trimmedLine.startsWith("1.") || trimmedLine.startsWith("2.") || trimmedLine.startsWith("3.") || trimmedLine.startsWith("4.") ||
                 trimmedLine.startsWith("5.") || trimmedLine.startsWith("6.") || trimmedLine.startsWith("7.") || trimmedLine.startsWith("8.") ||
                 trimmedLine.startsWith("9.") || trimmedLine.startsWith("10.") || trimmedLine.startsWith("11.") || trimmedLine.startsWith("12.") ||
                 trimmedLine.startsWith("13.") || trimmedLine.startsWith("14.") || trimmedLine.startsWith("15.") || trimmedLine.startsWith("16.") ||
                 trimmedLine.startsWith("17.") || trimmedLine.startsWith("18.") || trimmedLine.startsWith("19.") || trimmedLine.startsWith("20.")) {

            if (!inNumberedList) {
                htmlText += "<ol>";  // Открываем нумерованный список
                inNumberedList = true;
            }
            htmlText += "<li>" + trimmedLine.mid(2).trimmed() + "</li>";
        }
        // Если это просто строка текста
        else {
            // Закрываем открытый список, если есть
            if (inList) {
                htmlText += "</ul>";
                inList = false;
            }
            if (inNumberedList) {
                htmlText += "</ol>";
                inNumberedList = false;
            }
            htmlText += "<p>" + trimmedLine + "</p>";  // Добавляем абзац
        }
    }

    // Закрываем любой оставшийся список
    if (inList) {
        htmlText += "</ul>";
    }
    if (inNumberedList) {
        htmlText += "</ol>";
    }

    return htmlText;
}

void SaveManager::onReplyFinished(QNetworkReply *reply) {
    SaveManagerData returnData(SaveManagerData::SAVEMANAGER_VERSIONCHECKER_DONE);

    if(reply->error() == QNetworkReply::NoError){
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();

        // Извлекаем текст из интересующих нас разделов
        QString version = root.value("tag_name").toString();
        QString description = root.value("body").toString();

        // Условие для извлечения версии из текста
        static const QRegularExpression versionRegex("v(\\d+\\.\\d+\\.\\d+)");

        // Извлекаем версию из текста
        QRegularExpressionMatch newVersionMatch = versionRegex.match(version);
        QRegularExpressionMatch currentVersionMatch = versionRegex.match(APPLICATION_VERSION);

        // Форматируем описание обновления
        //description.replace("* ", "<li>").replace("\r\n", "<br>").replace("Version", "<b>Version</b>");
        //description.prepend("<ul>").append("</ul>");
        description = parseGithubText(description);

        QString newVersion = newVersionMatch.hasMatch() ? newVersionMatch.captured(1) : QString();
        QString currentVersion = currentVersionMatch.hasMatch() ? currentVersionMatch.captured(1) : QString();

        // Проверяем доступность новой версии сравнением с текущей "старой"
        bool isNewerVersionAvailable = false;
        QStringList currentParts = currentVersion.split(".");
        QStringList newParts = newVersion.split(".");

        for(int i = 0; i < qMin(currentParts.size(), newParts.size()); ++i){
            int currentPart = currentParts.at(i).toInt();
            int newPart = newParts.at(i).toInt();

            if(currentPart < newPart){
                isNewerVersionAvailable = true;
                break;

            }else if(currentPart > newPart){
                isNewerVersionAvailable = false;
                break;
            }
        }

        if(!isNewerVersionAvailable)
            isNewerVersionAvailable = newParts.size() > currentParts.size();

        if(isNewerVersionAvailable)
            returnData.SetVersionCheckerParam(1, "#NEW_VERSION_AVIABLE#: v" + newVersion, "v" + currentVersion, description, "https://github.com/HOMECODERNEO/Scissors/releases");
        else
            returnData.SetVersionCheckerParam(0, "", "", "", "");

    }else{
        returnData.SetVersionCheckerParam(2, "ERROR: " + reply->errorString(), "", "", "");
    }

    emit OperationEnd(returnData);
    reply->deleteLater();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Функция поиска свободных ID для названий изображений
QList<int> SaveManager::FindFreeIDs(const QSet<int> &usedIDs, int count){

    int id = 0;
    QList<int> freeIDs;

    while(freeIDs.size() < count){

        if(!usedIDs.contains(id))
            freeIDs.append(id);

        id++;
    }

    return freeIDs;
}

// Простая функция шифрования методом XOR (Работает в оба направления шифрования и расшифрования)
QByteArray SaveManager::Cipher(const QByteArray& data, const QByteArray& key){
    QByteArray encryptedData = data;

    for(int i = 0; i < data.size(); ++i)
        encryptedData[i] = data[i] ^ key[i % key.size()];

    return encryptedData;
}

// Запись информации об изображениях в файл метаданных
void SaveManager::WriteMetaDataFile(QString savePath, QByteArray data){
    QFile checkMetaDataFile(savePath);

    if(checkMetaDataFile.open(QIODevice::WriteOnly)){
        checkMetaDataFile.write(Cipher(data, QCryptographicHash::hash(ALGORITHM_KEY, QCryptographicHash::Sha256)));
        checkMetaDataFile.close();
    }
}

// Чтение информации об изображениях из файла метаданных
QList<SaveManagerFileData> SaveManager::ReadMetaDataFile(QString filePath){
    QFile checkMetaDataFile(filePath);

    if(checkMetaDataFile.open(QIODevice::ReadOnly)){
        QBuffer buffer;
        buffer.setData(Cipher(checkMetaDataFile.readAll(), QCryptographicHash::hash(ALGORITHM_KEY, QCryptographicHash::Sha256)));
        buffer.open(QIODevice::ReadOnly);
        QDataStream fileReader(&buffer);

        QList<SaveManagerFileData> dataList;

        ///////////////////////////////////
        // Читаем данные об файлах в массив

        while (!fileReader.atEnd()){
            QByteArray array;

            fileReader >> array;

            SaveManagerFileData buffer;
            buffer.DecoupleMetaData(array);

            dataList.append(buffer);
        }

        // Читаем данные об файлах в массив
        ///////////////////////////////////

        buffer.close();
        checkMetaDataFile.close();

        return dataList;
    }else
        return QList<SaveManagerFileData>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
