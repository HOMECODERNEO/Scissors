#ifndef TRANSLATORMANAGER_H
#define TRANSLATORMANAGER_H

#include <Includes.h>

class TranslatorManager : public QObject{
    Q_OBJECT

public:
    explicit TranslatorManager(QObject *parent = nullptr);

    QString translate(const QString &key);
    void LoadTranslate(QString lang, QComboBox *box);
    void ChangeProgramLanguage(TranslateData* container, QString lang);

signals:
    void Event_ChangeLanguage(TranslateData data);

private:
    bool _startProgramFlag = false;

};

#endif // TRANSLATORMANAGER_H
