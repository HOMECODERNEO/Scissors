#ifndef POPUP_H
#define POPUP_H

#include <Includes.h>

class Popup : public QWidget{
    Q_OBJECT

    struct NotificationData{
        QString _text, _url;
        int _displayTime;

        NotificationData(QString text, int displayTime, QString url){
            _text = text;
            _displayTime = displayTime;
            _url = url;
        }
    };

public:
    explicit Popup(QWidget *parent = nullptr);

    void showMessage(QString text, int time, QString url);

    void ClearLast();
    void ClearAll();

private:
    bool Show();
    void AnimationEnd(bool flag, int time);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

signals:
    QScreen* GetActiveScreen();

private:
    bool _flagShow = false;
    QString _currentMessage, _url;
    QQueue<NotificationData> _messageQueue;

    AnimationsManager _animationManager;
};

#endif // POPUP_H
