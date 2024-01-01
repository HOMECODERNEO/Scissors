#include "translatormanager.h"

TranslatorManager::TranslatorManager(QObject *parent) : QObject{parent}{
}

void TranslatorManager::TranslatorManager::LoadTranslate(QString lang, QComboBox *box){
    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + LANGUAGE_DIR;

    // Проверяем по пути загрузки лежащие файлы с расширением *.tr и формируем список количества файлов
    QDir folder(loadPath);
    folder.setNameFilters(QStringList("*.tr"));

    // Проходимся по каждому файлу для его добавления в элемент выбора языка
    foreach(QString fileName, folder.entryList())
        box->addItem(fileName.left(fileName.lastIndexOf('.')));

    // Выбираем сохранённый язык с настроек
    box->setCurrentText(lang);
}

void TranslatorManager::ChangeProgramLanguage(TranslateData* container, QString lang){
    container->Clear();

    // Путь загрузки
    QString loadPath = QApplication::applicationDirPath() + LANGUAGE_DIR;

    // Загружаем и читаем файл
    QSettings _lang(loadPath + lang + ".tr", QSettings::IniFormat);
    _lang.setIniCodec("UTF-8");

    QStringList keys = _lang.allKeys();

    // Заполняем список переводов    
    for(const QString &key : keys)
        container->Set(key, _lang.value(key).toString());

    // Вызываем применение перевода программы
    emit Event_ChangeLanguage(*container);
}
