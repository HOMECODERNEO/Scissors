#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include <Includes.h>

class PopupManager : public QWidget{
    Q_OBJECT

    struct NotificationData{
        QString _text, _url, _secondary;
        int _displayTime;

        NotificationData(QString text, QString secondary, int displayTime, QString url){
            _url = url;
            _text = text;
            _secondary = secondary;
            _displayTime = displayTime;
        }
    };

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

public:
    explicit PopupManager(QWidget *parent = nullptr);

    void showMessage(QString text, QString secondary, int time, QString url, TranslateData _translator);

public slots:
    void Event_ChangeLanguage(TranslateData data);

private slots:
    void Timer_TextAnimate();

private:
    bool Show();
    void AnimationEnd(bool flag, int time);

signals:
    QScreen* GetActiveScreen();

private:
    float _textFade = 0;
    QTimer *_timerTextOpacity;

    bool _flagShow = false, _flagClosed = false;
    QString _currentMessage, _url, _secondaryMessage, _rightClick, _leftClick;
    QQueue<NotificationData> _messageQueue;

    AnimationsManager _animationManager;
};

#endif // POPUPMANAGER_H
