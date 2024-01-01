#include "popup.h"

Popup::Popup(QWidget *parent): QWidget{parent}{
    // Атрибуты окна
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    connect(this, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));

    hide();
}

void Popup::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setBrush(QBrush(QColor(38, 46, 49)));
    paint.drawRoundedRect(0, 0, width(), height(), 10, 10);

    // Добавляем текст по центру
    QFont font = paint.font();

    font.setPointSize(12);
    paint.setFont(font);
    paint.setPen(QColor(Qt::white));
    paint.drawText(rect(), Qt::AlignCenter, _currentMessage);

    font.setPointSize(7);
    paint.setFont(font);
    paint.setPen(QColor(Qt::white));
    paint.drawText(rect(), Qt::AlignHCenter | Qt::AlignTop, APPLICATION_NAME);

    if(!_url.isEmpty()){
        paint.drawText(rect().adjusted(2, 0, -2, -5), Qt::AlignLeft | Qt::AlignBottom, "  Right - Open");
        paint.drawText(rect().adjusted(2, 0, -2, -5), Qt::AlignRight | Qt::AlignBottom, "Left - Close  ");
    }

    paint.end();
}

void Popup::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        // Открываем ссылку
        QDesktopServices::openUrl(QUrl(_url));
    }

    // Закрываем окно
    _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(false, 0); }, 200, this->geometry(), QRect(emit GetActiveScreen()->geometry().width(), 40, 50, 70)).Start();
}

void Popup::showMessage(QString text, int time, QString url){

    // Добавляем в очередь за отображение
    NotificationData notification(text, time, url);
    _messageQueue.enqueue(notification);

    // Если никаких уведомлений сейчас не отображаеться - отображаем
    if(!_flagShow){
        if(Show())
            show();
    }
}

void Popup::AnimationEnd(bool flag, int time){
    if(flag){ // OPENED
        if(time != 0){
            QTimer::singleShot(time, this, [&](){
                _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(false, 0); }, 200, this->geometry(), QRect(emit GetActiveScreen()->geometry().width(), 40, 50, 70)).Start();
            });
        }

    }else{ // CLOSED
        _flagShow = false;
        Show();
    }
}

void Popup::ClearLast(){

}

void Popup::ClearAll(){

}

bool Popup::Show(){
    if(_messageQueue.isEmpty())
        return false;

    NotificationData notification = _messageQueue.dequeue();

    // Устанавливаем текущее сообщение и размер окна
    _url = notification._url;
    _currentMessage = notification._text;

    QFontMetrics fontMetrics(QFont("Arial", 12));

    QRect geometry(emit GetActiveScreen()->geometry().width(), 40, fontMetrics.horizontalAdvance(_currentMessage) + 35, 90);

    _flagShow = true;
    _animationManager.Create_ObjectGeometry(this, [this, notification, geometry](){ AnimationEnd(true, notification._displayTime); }, 200, geometry, QRect(geometry.x() - geometry.width() - 10, geometry.y(), geometry.width(), geometry.height())).Start();

    return true;
}
