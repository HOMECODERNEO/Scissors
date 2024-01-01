#include "popupmanager.h"

// Конструктор
PopupManager::PopupManager(QWidget *parent): QWidget{parent}{

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    _timerTextOpacity = new QTimer(this);
    connect(_timerTextOpacity, &QTimer::timeout, this, &PopupManager::Timer_TextAnimate);

    connect(this, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));

    hide();
}

// Событие отрисовки
void PopupManager::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setBrush(QBrush(QColor(33, 33, 33, 180))); // Background
    paint.setPen(QPen(QColor(99, 99, 99, 180), 2)); // Border

    paint.drawRoundedRect(0, 0, width(), height(), 10, 10);

    QFont font = paint.font();

    // Текущее сообщение
    font.setPointSize(12);
    paint.setFont(font);
    paint.setPen(QColor(255, 255, 255, 255 * _textFade));
    paint.drawText(rect(), Qt::AlignCenter, _currentMessage);

    // Заголовок сообщения
    font.setPointSize(7);
    paint.setFont(font);
    paint.setPen(QColor(255, 124, 112, 255 * _textFade));
    paint.drawText(rect().adjusted(0, 6, 0, 0), Qt::AlignHCenter | Qt::AlignTop, APPLICATION_NAME);

    // Если в сообщении есть ссылка, выводим кнопки управления
    if(!_url.isEmpty()){
        paint.setPen(QColor(255, 237, 122, 255 * _textFade));
        paint.drawText(rect().adjusted(6, 0, -6, -6), Qt::AlignLeft | Qt::AlignBottom, _leftClick);
        paint.drawText(rect().adjusted(6, 0, -6, -6), Qt::AlignRight | Qt::AlignBottom, _rightClick);
    }

    // Если дополнительный текст есть - отображаем его
    if(!_secondaryMessage.isEmpty()){
        paint.setPen(QColor(255, 124, 112, 255 * _textFade));
        paint.drawText(rect().adjusted(0, 0, 0, -6), Qt::AlignHCenter | Qt::AlignBottom, _secondaryMessage);
    }

    paint.end();
}

// Событие отпускания кнопки мыши
void PopupManager::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){
        // Открываем ссылку
        QDesktopServices::openUrl(QUrl(_url));
    }

    // Закрываем окно
    _flagClosed = false;
    _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(false, 0); }, POPUP_ANIMATION_SPEED, this->geometry(), QRect(emit GetActiveScreen()->geometry().width(), 40, 50, 90)).Start();
    _timerTextOpacity->start(3);
}

// Функция вызова отображения уведомления на экране
void PopupManager::showMessage(QString text, QString secondary, int time, QString url, TranslateData _translator){

    static QRegularExpression regex("#([^#]+)#");

    QString _text, _secondary;
    QRegularExpressionMatch match;
    QList<QPair<int, int>> placeholderIndices;
    QRegularExpressionMatchIterator matchIterator;

    if(!text.isEmpty()){
        placeholderIndices.clear();

        matchIterator = regex.globalMatch(text);

        // Перебираем в строке все слова в спец. символах
        while(matchIterator.hasNext()){
            match = matchIterator.next();
            placeholderIndices.append(qMakePair(match.capturedStart(1), match.capturedEnd(1)));
        }

        // Изменяем найденное слово, и вставляем обратно в текст без спец. символа "#"
        for(int i = placeholderIndices.size() - 1; i >= 0; --i){
            int start = placeholderIndices[i].first;
            int end = placeholderIndices[i].second;

            text.replace(start - 1, end  - start + 2, _translator.translate(text.mid(start, end - start)));
        }

        _text = text;
    }

    if(!secondary.isEmpty()){
        placeholderIndices.clear();

        matchIterator = regex.globalMatch(secondary);

        while(matchIterator.hasNext()){
            match = matchIterator.next();
            placeholderIndices.append(qMakePair(match.capturedStart(1), match.capturedEnd(1)));
        }

        // Изменяем найденное слово, и вставляем обратно в текст без спец. символа "#"
        for(int i = placeholderIndices.size() - 1; i >= 0; --i){
            int start = placeholderIndices[i].first;
            int end = placeholderIndices[i].second;

            secondary.replace(start - 1, end  - start + 2, _translator.translate(secondary.mid(start, end - start)));
        }

        _secondary = secondary;
    }

    // Добавляем в очередь за отображение
    NotificationData notification(_text, _secondary, time, url);
    _messageQueue.enqueue(notification);

    // Если никаких уведомлений сейчас не отображаеться - отображаем
    if(!_flagShow){
        if(Show())
            show();
    }
}

// Функция анимации прозрачности текста через таймер
void PopupManager::Timer_TextAnimate(){
    if(!_flagClosed && _textFade - 0.04 > 0){
        _textFade -= 0.04;
    }else if(_flagClosed && _textFade + 0.01 < 1){
        _textFade += 0.01;
    }

    // Перерысовываем окно для изменения прозрачности текста
    repaint();
}

// Событие окончания анимаций открытия и закрытия
void PopupManager::AnimationEnd(bool flag, int time){
    _timerTextOpacity->stop();

    if(flag){ // OPENED
        if(time != 0){
            QTimer::singleShot(time, this, [&](){
                _flagClosed = false;
                _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(false, 0); }, POPUP_ANIMATION_SPEED, this->geometry(), QRect(emit GetActiveScreen()->geometry().width(), 40, 50, 90)).Start();
                _timerTextOpacity->start(3);
            });
        }

    }else{ // CLOSED
        _flagShow = false;
        Show();
    }
}

// Функция проверки, формирования размера окна, и отображения сообщения
bool PopupManager::Show(){
    if(_messageQueue.isEmpty())
        return false;

    NotificationData notification = _messageQueue.dequeue();

    // Устанавливаем текущее сообщение и размер окна
    _url = notification._url;
    _currentMessage = notification._text;
    _secondaryMessage = notification._secondary;

    QFontMetrics fontMetrics(QFont("Arial", 12));

    QRect geometry(emit GetActiveScreen()->geometry().width(), 40, fontMetrics.horizontalAdvance(_currentMessage) + 35, 90);

    _flagShow = true;
    _flagClosed = true;
    _animationManager.Create_ObjectGeometry(this, [this, notification, geometry](){ AnimationEnd(true, notification._displayTime); }, POPUP_ANIMATION_SPEED, geometry, QRect(geometry.x() - geometry.width() - 10, geometry.y(), geometry.width(), geometry.height())).Start();
    _timerTextOpacity->start(3);

    return true;
}

// Событие смены языка всей программы
void PopupManager::Event_ChangeLanguage(TranslateData data){
    _leftClick = data.translate("POPUP_LEFT");
    _rightClick = data.translate("POPUP_RIGHT");
}
