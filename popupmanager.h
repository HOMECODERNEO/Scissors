#ifndef POPUPMANAGER_H
#define POPUPMANAGER_H

#include <Includes.h>

class TextWindow;

class PopupManager : public QWidget{
    Q_OBJECT

public:
    struct NotificationData{
        public:
            NotificationData() = default;

            NotificationData(QString mainMessage, QString secondaryMessage, QString windowMessage, QString url, int displayTime){
                _url = url;
                _displayTime = displayTime;
                _mainMessage = mainMessage;
                _windowMessage = windowMessage;
                _secondaryMessage = secondaryMessage;
            }

            int GetDisplayTime() const {return _displayTime; }

            QString GetUrlLink() const { return _url; }
            QString GetMainMessage() const { return _mainMessage; }
            QString GetWindowMessage() const { return _windowMessage; }
            QString GetSecondaryMessage() const { return _secondaryMessage; }

        private:
            int _displayTime;

            QString _url;
            QString _mainMessage;
            QString _windowMessage;
            QString _secondaryMessage;
    };

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

public:
    explicit PopupManager(QWidget *parent = nullptr);

    void showMessage(QString mainMessage, QString secondaryMessage, QString windowMessage, QString url, int time, TranslateData _translator);

public slots:
    void Event_ChangeLanguage(TranslateData data);

private slots:
    bool Show();
    bool Hide();
    void Timer_TextAnimate();

private:
    void AnimationEnd(bool flag, int time);

signals:
    QRect GetCurrentScreenGeometry();
    ProgramSetting GetProgramSettings();
    void PlaySound(SOUNDMANAGER_SOUND_TYPE type);

private:
    float _textFade = 0;
    bool _flagVisible = false;
    QString _rightClick, _leftClick;
    AnimationsManager _animationManager;
    NotificationData _currentNotification;
    QQueue<NotificationData> _messageQueue;
    QTimer *_timerTextOpacity, *_timerClose;

    std::unique_ptr<TextWindow> _textWindow;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////// TEXTWINDOW ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class TextWindow : public QWidget{
    Q_OBJECT

    enum ACTION{
        ACTION_NONE,
        ACTION_MOVE,
        ACTION_RESIZE_HOR_RIGHT,
        ACTION_RESIZE_HOR_LEFT,
        ACTION_RESIZE_VER_UP,
        ACTION_RESIZE_VER_DOWN,
        ACTION_RESIZE_RIGHT_DOWN,
        ACTION_RESIZE_RIGHT_UP,
        ACTION_RESIZE_LEFT_DOWN,
        ACTION_RESIZE_LEFT_UP
    } _modeAction = ACTION_NONE;

public:
    TextWindow(const PopupManager::NotificationData &messageData, const QRect &screenGeometry, QWidget* parent = nullptr) : QWidget(parent){

        // Настраиваем параметры окна
        setMouseTracking(true);
        setStyleSheet("background-color: rgb(48, 56, 65);");
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

        // Основной слой
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Зона прокрутки
        QWidget *scrollWidget = new QWidget();
        QScrollArea *scrollArea = new QScrollArea(this);
        QVBoxLayout *scrollLayout = new QVBoxLayout(scrollArea);

        scrollArea->setMouseTracking(true);
        scrollWidget->setMouseTracking(true);
        scrollArea->setWidgetResizable(true);

        scrollArea->setStyleSheet(R"(
                                        QScrollBar{
                                            border: none;
                                            background: rgb(45, 45, 68);
                                            border-radius: 0px;
                                        }

                                        QScrollBar::handle{
                                            background-color: rgb(80, 80, 122);
                                            border-radius: 7px;
                                        }

                                        QScrollBar::sub-line{
                                            border: none;
                                            background-color: rgb(59, 59, 90);
                                            border-radius: 7px;
                                        }

                                        QScrollBar::add-line {
                                            border: none;
                                            background-color: rgb(59, 59, 90);
                                            border-radius: 7px;
                                        }

                                        QScrollBar::handle:vertical{ min-height: 30px; }
                                        QScrollBar::handle:horizontal{ min-width: 30px; }
                                        QScrollBar::handle:hover{ background-color: rgb(255, 0, 127); }
                                        QScrollBar::handle:pressed{ background-color: rgb(185, 0, 92); }
                                        QScrollBar::add-page, QScrollBar::sub-page { background: none; }
                                        QScrollBar::sub-line:hover{ background-color: rgb(255, 0, 127); }
                                        QScrollBar::add-line:hover{ background-color: rgb(255, 0, 127); }
                                        QScrollBar::sub-line:pressed{ background-color: rgb(185, 0, 92); }
                                        QScrollBar::add-line:pressed { background-color: rgb(185, 0, 92); }
                                        QScrollBar::up-arrow, QScrollBar::down-arrow, QScrollBar::left-arrow, QScrollBar::right-arrow { background: none; }

                                        /* VERTICAL SCROLLBAR */
                                        QScrollBar:vertical{
                                            width: 14px;
                                            margin: 15px 0;
                                        }

                                        QScrollBar::sub-line:vertical{
                                            height: 15px;
                                            border-top-left-radius: 7px;
                                            border-top-right-radius: 7px;
                                            subcontrol-position: top;
                                            subcontrol-origin: margin;
                                        }

                                        QScrollBar::add-line:vertical{
                                            height: 15px;
                                            border-bottom-left-radius: 7px;
                                            border-bottom-right-radius: 7px;
                                            subcontrol-position: bottom;
                                            subcontrol-origin: margin;
                                        }

                                        /* HORIZONTAL SCROLLBAR */
                                        QScrollBar:horizontal {
                                            height: 14px;
                                            margin: 0 15px;
                                        }

                                        QScrollBar::sub-line:horizontal {
                                            width: 15px;
                                            border-top-left-radius: 7px;
                                            border-bottom-left-radius: 7px;
                                            subcontrol-position: left;
                                            subcontrol-origin: margin;
                                        }

                                        QScrollBar::add-line:horizontal {
                                            width: 15px;
                                            border-top-right-radius: 7px;
                                            border-bottom-right-radius: 7px;
                                            subcontrol-position: right;
                                            subcontrol-origin: margin;
                                        }
                                    )");

        scrollWidget->setLayout(scrollLayout);
        scrollArea->setWidget(scrollWidget);
        mainLayout->addWidget(scrollArea);

        // Основной контент окна
        QLabel *contentLabel = new QLabel(messageData.GetWindowMessage(), scrollWidget);
        contentLabel->setMouseTracking(true);
        contentLabel->setTextFormat(Qt::RichText);
        contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        contentLabel->setStyleSheet("color: rgb(255, 255, 255);");
        scrollLayout->addWidget(contentLabel);

        // Кнопки
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *buttonGit = new QPushButton("GitHub", this);
        QPushButton *buttonClose = new QPushButton("Close", this);

        buttonGit->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 10px;
                                        background-color: rgb(0, 175, 80);
                                        padding: 5px 15px;
                                    }
                                    QPushButton:hover{
                                        background-color: rgb(65, 175, 115);
                                    }
                                )");

        buttonClose->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 10px;
                                        background-color: rgb(130, 50, 50);
                                        padding: 5px 15px;
                                    }
                                    QPushButton:hover{
                                        background-color: rgb(150, 70, 70);
                                    }
                                )");

        connect(buttonClose, &QPushButton::clicked, this, &TextWindow::close);

        connect(buttonGit, &QPushButton::clicked, this, [=](){
            if(!messageData.GetUrlLink().isEmpty())
                QDesktopServices::openUrl(QUrl(messageData.GetUrlLink()));

            close();
        });

        buttonLayout->setAlignment(Qt::AlignCenter);
        buttonLayout->addWidget(buttonClose);
        buttonLayout->addWidget(buttonGit);

        // Собираем все слои
        mainLayout->addLayout(buttonLayout);
        this->setLayout(mainLayout);

        // Событие центрирование окна
        QTimer::singleShot(5, this, [=](){
            screenWidth = screenGeometry.width();
            screenHeight = screenGeometry.height();

            int x = (screenWidth - this->geometry().width()) / 2;
            int y = (screenHeight - this->geometry().height()) / 2;

            this->move(x, y);
        });
    }

protected:
    void mousePressEvent(QMouseEvent *event) override{
        mousePressPosition = event->pos();
        mousePressGlobalPosition = event->globalPosition();
        mousePressDiffFromBorder.setWidth(width() - event->position().x());
        mousePressDiffFromBorder.setHeight(height() - event->position().y());

        if(event->buttons() == Qt::LeftButton){
            if(event->position().x() < FormBorderWidth && event->position().y() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_LEFT_UP;
                setCursor(Qt::SizeFDiagCursor);
            }else if(event->position().x() < FormBorderWidth && (height() - event->position().y()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_LEFT_DOWN;
                setCursor(Qt::SizeBDiagCursor);
            }else if(event->position().y() < FormBorderWidth && (width() - event->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_RIGHT_UP;
                setCursor(Qt::SizeBDiagCursor);
            }else if((height() - event->position().y()) < FormBorderWidth && (width() - event->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_RIGHT_DOWN;
                setCursor(Qt::SizeFDiagCursor);
            }else if(event->position().x() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_HOR_LEFT;
                setCursor(Qt::SizeHorCursor);
            }else if((width() - event->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_HOR_RIGHT;
                setCursor(Qt::SizeHorCursor);
            }else if(event->position().y() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_VER_UP;
                setCursor(Qt::SizeVerCursor);
            }else if((height() - event->position().y()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_VER_DOWN;
                setCursor(Qt::SizeVerCursor);
            }else{
                _modeAction = ACTION_MOVE;
                setCursor(Qt::ClosedHandCursor);
            }
        }else
            _modeAction = ACTION_NONE;

        if(_modeAction != ACTION_NONE && _modeAction != ACTION_MOVE)
            _areaResize = true;
        else if(_modeAction == ACTION_MOVE)
            _areaMove = true;

        repaint();
    }

    void mouseMoveEvent(QMouseEvent *event) override{
        mouseMovePosition = event->pos();

        if(_modeAction == ACTION_NONE){
            checkAndSetCursors(event);
            repaint();

        }else if(_modeAction == ACTION_MOVE){
            QPointF moveHere;
            moveHere = event->globalPosition() - mousePressPosition;

            QRect newRect = QRect(moveHere.toPoint(), geometry().size());

            if(newRect.x() < 0) newRect.setX(0);
            if(newRect.y() < 0) newRect.setY(0);

            if(newRect.x() + newRect.width() > screenWidth) newRect.setX(screenWidth - newRect.width());
            if(newRect.y() + newRect.height() > screenHeight) newRect.setY(screenHeight - newRect.height());

            move(newRect.topLeft());
            repaint();
        }else{
            QRect newRect = resizeAccordingly(event);

            // Ограничение размера объекта по размеру экрана
            newRect = newRect.intersected(QRect(0, 0, screenWidth, screenHeight));

            if(newRect.size() != geometry().size())
                resize(newRect.size());

            if(newRect.topLeft() != geometry().topLeft())
                move(newRect.topLeft());
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override{
        _areaResize = false;
        _areaMove = false;

        _modeAction = ACTION_NONE;
        checkAndSetCursors(event);
        repaint();
    }

private:
    QRect resizeAccordingly(QMouseEvent *pe){
        int newWidth = width();
        int newHeight = height();
        int newX = x();
        int newY = y();

        switch(_modeAction){
            case ACTION_RESIZE_HOR_RIGHT:{
                newWidth = pe->position().x() + mousePressDiffFromBorder.width();
                newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
                break;
            }

            case ACTION_RESIZE_VER_DOWN:{
                newHeight = pe->position().y() + mousePressDiffFromBorder.height();
                newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;
                break;
            }

            case ACTION_RESIZE_HOR_LEFT:{
                newY = pos().y();
                newHeight = height();

                newWidth = mousePressGlobalPosition.x() - pe->globalPosition().x() + mousePressPosition.x() + mousePressDiffFromBorder.width();

                if(newWidth < FormMinimumWidth){
                    newWidth = FormMinimumWidth;
                    newX = mousePressGlobalPosition.x() + mousePressDiffFromBorder.width() - FormMinimumWidth;
                }else
                    newX = pe->globalPosition().x() - mousePressPosition.x();

                break;
            }

            case ACTION_RESIZE_VER_UP:{
                newX = pos().x();
                newWidth = width();

                newHeight = mousePressGlobalPosition.y() - pe->globalPosition().y() + mousePressPosition.y() + mousePressDiffFromBorder.height();

                if (newHeight < FormMinimumHeight){
                    newHeight = FormMinimumHeight;
                    newY = mousePressGlobalPosition.y() + mousePressDiffFromBorder.height() - FormMinimumHeight;
                }else
                    newY = pe->globalPosition().y() - mousePressPosition.y();

                break;
            }


            case ACTION_RESIZE_RIGHT_DOWN:{
                newWidth = pe->position().x() + mousePressDiffFromBorder.width();
                newHeight = pe->position().y() + mousePressDiffFromBorder.height();
                newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
                newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;
                break;
            }

            case ACTION_RESIZE_RIGHT_UP:{
                newWidth = pe->position().x() + mousePressDiffFromBorder.width();

                if (newWidth < FormMinimumWidth)
                    newWidth = FormMinimumWidth;

                newX = pos().x();

                newHeight = mousePressGlobalPosition.y() - pe->globalPosition().y() + mousePressPosition.y() + mousePressDiffFromBorder.height();

                if(newHeight < FormMinimumHeight){
                    newHeight = FormMinimumHeight;
                    newY = mousePressGlobalPosition.y() + mousePressDiffFromBorder.height() - FormMinimumHeight;
                }else
                    newY = pe->globalPosition().y() - mousePressPosition.y();

                break;
            }

            case ACTION_RESIZE_LEFT_DOWN:{
                newHeight = pe->position().y() + mousePressDiffFromBorder.height();

                if(newHeight < FormMinimumHeight)
                    newHeight = FormMinimumHeight;

                newY = pos().y();

                newWidth = mousePressGlobalPosition.x() - pe->globalPosition().x() + mousePressPosition.x() + mousePressDiffFromBorder.width();

                if(newWidth < FormMinimumWidth){
                    newWidth = FormMinimumWidth;
                    newX = mousePressGlobalPosition.x() + mousePressDiffFromBorder.width() - FormMinimumWidth;
                }else
                    newX = pe->globalPosition().x() - mousePressPosition.x();

                break;
            }

            case ACTION_RESIZE_LEFT_UP:{
                newWidth = mousePressGlobalPosition.x() - pe->globalPosition().x() + mousePressPosition.x() + mousePressDiffFromBorder.width();

                if(newWidth < FormMinimumWidth){
                    newWidth = FormMinimumWidth;
                    newX = mousePressGlobalPosition.x() + mousePressDiffFromBorder.width() - FormMinimumWidth;
                }else
                    newX = pe->globalPosition().x() - mousePressPosition.x();


                newHeight = mousePressGlobalPosition.y() - pe->globalPosition().y() + mousePressPosition.y() + mousePressDiffFromBorder.height();

                if(newHeight < FormMinimumHeight){
                    newHeight = FormMinimumHeight;
                    newY = mousePressGlobalPosition.y() + mousePressDiffFromBorder.height() - FormMinimumHeight;
                }else
                    newY = pe->globalPosition().y() - mousePressPosition.y();

                break;
            }

            default: break;
        }

        return QRect(newX, newY, newWidth, newHeight);
    }

    void checkAndSetCursors(QMouseEvent *pe){
        if(pe->position().x() < FormBorderWidth && pe->position().y() < FormBorderWidth) { setCursor(Qt::SizeFDiagCursor); }
        else if(pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){ setCursor(Qt::SizeBDiagCursor); }
        else if(pe->position().y() < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeBDiagCursor);}
        else if((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeFDiagCursor); }
        else if(pe->position().x() < FormBorderWidth){ setCursor(Qt::SizeHorCursor); }
        else if((width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeHorCursor); }
        else if(pe->position().y() < FormBorderWidth){ setCursor(Qt::SizeVerCursor); }
        else if((height() - pe->position().y()) < FormBorderWidth){ setCursor(Qt::SizeVerCursor); }
        else setCursor(Qt::ArrowCursor);
    }

private:
    bool _areaMove = false;
    bool _areaResize = false;
    int screenWidth = 0, screenHeight = 0;

    QSize mousePressDiffFromBorder;
    QPointF mousePressGlobalPosition;
    QPoint mousePressPosition, mouseMovePosition;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////////// END ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // POPUPMANAGER_H
