#include "popupmanager.h"

// Конструктор
PopupManager::PopupManager(QWidget *parent): QWidget{parent}{

    setParent(parent);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    _timerClose = new QTimer(this);
    _timerTextOpacity = new QTimer(this);

    connect(_timerClose, &QTimer::timeout, this, &PopupManager::Hide);
    connect(_timerTextOpacity, &QTimer::timeout, this, &PopupManager::Timer_TextAnimate);

    hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// MAIN ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Функция вызова отображения уведомления на экране
void PopupManager::showMessage(QString mainMessage, QString secondaryMessage, QString windowMessage, QString url, int time, TranslateData _translator){

    static QRegularExpression regex("#([^#]+)#");

    QString _mainMessage, _secondaryMessage;
    QRegularExpressionMatch match;
    QList<QPair<int, int>> placeholderIndices;
    QRegularExpressionMatchIterator matchIterator;

    if(!mainMessage.isEmpty()){
        placeholderIndices.clear();

        matchIterator = regex.globalMatch(mainMessage);

        // Перебираем в строке все слова в спец. символах
        while(matchIterator.hasNext()){
            match = matchIterator.next();
            placeholderIndices.append(qMakePair(match.capturedStart(1), match.capturedEnd(1)));
        }

        // Изменяем найденное слово, и вставляем обратно в текст без спец. символа "#"
        for(int i = placeholderIndices.size() - 1; i >= 0; --i){
            int start = placeholderIndices[i].first;
            int end = placeholderIndices[i].second;

            mainMessage.replace(start - 1, end  - start + 2, _translator.translate(mainMessage.mid(start, end - start)));
        }

        _mainMessage = mainMessage;
    }

    if(!secondaryMessage.isEmpty()){
        placeholderIndices.clear();

        matchIterator = regex.globalMatch(secondaryMessage);

        while(matchIterator.hasNext()){
            match = matchIterator.next();
            placeholderIndices.append(qMakePair(match.capturedStart(1), match.capturedEnd(1)));
        }

        // Изменяем найденное слово, и вставляем обратно в текст без спец. символа "#"
        for(int i = placeholderIndices.size() - 1; i >= 0; --i){
            int start = placeholderIndices[i].first;
            int end = placeholderIndices[i].second;

            secondaryMessage.replace(start - 1, end  - start + 2, _translator.translate(secondaryMessage.mid(start, end - start)));
        }

        _secondaryMessage = secondaryMessage;
    }

    // Добавляем в очередь на отображение
    NotificationData notification(_mainMessage, _secondaryMessage, windowMessage, url, time);
    _messageQueue.enqueue(notification);

    // Если никаких уведомлений сейчас не отображаеться - отображаем
    if(!_flagVisible)
        Show();
}

// Функция проверки, формирования размера окна, и отображения сообщения
bool PopupManager::Show(){

    if(_messageQueue.isEmpty())
        return false;

    // Получаем данные о текущем сообщении
    _currentNotification = _messageQueue.dequeue();

    // Расчитываем ширину окна
    QRect geometry((emit GetCurrentScreenGeometry()).width(), 40, QFontMetrics(QFont("Arial", 12)).horizontalAdvance(_currentNotification.GetMainMessage()) + 35, 90);

    // Запускаем анимацию появления
    _flagVisible = true;

    _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(true, _currentNotification.GetDisplayTime()); }, POPUP_ANIMATION_SPEED, geometry, QRect(geometry.x() - geometry.width() - 10, geometry.y(), geometry.width(), geometry.height())).Start();
    _timerTextOpacity->start(3);

    show();

    // Воспроизводим звук
    emit PlaySound(SOUND_TYPE_POPUP);

    return true;
}

bool PopupManager::Hide(){

    // Останавливаем таймер закрытия сообщения
    if(_timerClose->isActive())
        _timerClose->stop();

    _flagVisible = false;

    // Запускаем анимацию закрытия
    _animationManager.Create_ObjectGeometry(this, [this](){ AnimationEnd(false, 0); }, POPUP_ANIMATION_SPEED, this->geometry(), QRect((emit GetCurrentScreenGeometry()).width(), 40, 50, 90)).Start();
    _timerTextOpacity->start(3);

    return true;
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
    paint.drawText(rect(), Qt::AlignCenter, _currentNotification.GetMainMessage());

    // Заголовок сообщения
    font.setPointSize(9);
    paint.setFont(font);
    paint.setPen(QColor(255, 124, 112, 255 * _textFade));
    paint.drawText(rect().adjusted(0, 6, 0, 0), Qt::AlignHCenter | Qt::AlignTop, APPLICATION_NAME);

    // Если в сообщении есть ссылка, выводим кнопки управления
    if(!_currentNotification.GetUrlLink().isEmpty()){
        paint.setPen(QColor(255, 237, 122, 255 * _textFade));
        paint.drawText(rect().adjusted(6, 0, -6, -6), Qt::AlignLeft | Qt::AlignBottom, _leftClick);
        paint.drawText(rect().adjusted(6, 0, -6, -6), Qt::AlignRight | Qt::AlignBottom, _rightClick);
    }

    // Если дополнительный текст есть - отображаем его
    if(!_currentNotification.GetSecondaryMessage().isEmpty()){
        paint.setPen(QColor(255, 124, 112, 255 * _textFade));
        paint.drawText(rect().adjusted(0, 0, 0, -6), Qt::AlignHCenter | Qt::AlignBottom, _currentNotification.GetSecondaryMessage());
    }

    paint.end();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// MOUSE ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Событие отпускания кнопки мыши
void PopupManager::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton){

        // Если мы имеем сообщение для окна то открываем его в окне
        if(!_currentNotification.GetWindowMessage().isEmpty()){

            // Освобождаем память если окно не удалилось
            if(_textWindow){
                _textWindow->close();
                _textWindow.reset();
            }

            if(!_textWindow){
                _textWindow = std::make_unique<TextWindow>( _currentNotification, emit GetCurrentScreenGeometry() );
                _textWindow->setWindowModality(Qt::ApplicationModal);
                _textWindow->show();

                // Освобождение памяти при закрытии окна
                connect(_textWindow.get(), &TextWindow::destroyed, this, [this](){
                    _textWindow.reset();
                });
            }

        // Если сообщения для окна не было, но была ссылка, то открываем ее если она не пустая
        }else if(!_currentNotification.GetUrlLink().isEmpty())
            QDesktopServices::openUrl(QUrl(_currentNotification.GetUrlLink()));
    }

    // Закрываем основное окно с сообщением
    Hide();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// SIGNAL //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Функция анимации прозрачности текста через таймер
void PopupManager::Timer_TextAnimate(){
    if(!_flagVisible && _textFade - 0.04 > 0){
        _textFade -= 0.04;
    }else if(_flagVisible && _textFade + 0.01 < 1){
        _textFade += 0.01;
    }

    // Перерысовываем окно для изменения прозрачности текста
    repaint();
}

// Событие окончания анимаций открытия и закрытия
void PopupManager::AnimationEnd(bool flag, int time){

    _timerTextOpacity->stop();

    if(flag){ // Открытие

        // Если отображение какое-то время то закрываем спустя время
        if(time != 0)
            _timerClose->start(time);

    }else{ // Закрытие
        Show();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// TRANSLATOR ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Событие смены языка всей программы
void PopupManager::Event_ChangeLanguage(TranslateData data){
    _leftClick = data.translate("POPUP_LEFT");
    _rightClick = data.translate("POPUP_RIGHT");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// END ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
