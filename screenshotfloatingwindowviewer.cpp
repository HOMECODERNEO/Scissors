#include "screenshotfloatingwindowviewer.h"

ScreenshotFloatingWindowViewer::ScreenshotFloatingWindowViewer(int id, QPixmap image, QList<ScreenshotFloatingWindowViewer*> *List_WindowViewer, QWidget *parent) : QWidget(parent){
    setWindowTitle("Screenshot");
    setParent(parent);

    _resizeIconTimer = new QTimer();

    //////
    QScreen *screen = QGuiApplication::primaryScreen();
    int centreX = (screen->geometry().width() / 2) - (image.width() / 2);
    int centreY = (screen->geometry().height() / 2) - (image.height() / 2);
    setGeometry(centreX, centreY, image.width(), image.height());
    //////

    _screenListID = id;
    _currentImage = image;
    m_List_WindowViewer = List_WindowViewer;

    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_TranslucentBackground,true);
    setMouseTracking(true);

    connect(_resizeIconTimer, &QTimer::timeout, this, &ScreenshotFloatingWindowViewer::updateCursorPositionTimer);
    connect(this, SIGNAL(FloatingWindowClose(ScreenshotFloatingWindowViewer*, int, QPixmap)), parent, SLOT(FloatingWindowClose(ScreenshotFloatingWindowViewer*, int, QPixmap)));

    _resizeIconTimer->start(25);

    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

void ScreenshotFloatingWindowViewer::resizeEvent(QResizeEvent *){
    _resizeIcon = new QRect(width() - 22, height() - 22, 18, 18);
}

void ScreenshotFloatingWindowViewer::paintEvent(QPaintEvent*){
    _painter.begin(this);

    // DRAW FRONT IMAGE
    _painter.drawPixmap(0, 0, width(), height(), _currentImage.scaled(this->size().width() - 2, this->size().height() - 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    _painter.drawPixmap(width() - 22, height() - 22, 18, 18, QPixmap(_resizeIconIsHover ? ":/Buttons/Resourse/Buttons/Resize.png" : ":/Buttons/Resourse/Buttons/Resize_Transparent.png"));

    _painter.setPen(QPen(QColor(255, 238, 88), 2, Qt::SolidLine, Qt::SquareCap));
    _painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
    _painter.drawRect(0, 0, width(), height());
    _painter.end();
}

void ScreenshotFloatingWindowViewer::updateCursorPositionTimer(){
    if(_resizeIcon->contains(mapFromGlobal(QCursor::pos()))){
        _resizeIconIsHover = true;
        repaint();
    }else if(_resizeIconIsHover){
        _resizeIconIsHover = false;
        repaint();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////[-------Событие перетаскивания--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotFloatingWindowViewer::mousePressEvent(QMouseEvent *pe){
    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPos();
    mousePressDiffFromBorder.setWidth(width() - pe->x());
    mousePressDiffFromBorder.setHeight(height() - pe->y());

    if (pe->buttons() == Qt::LeftButton){
        _mousePress = MOUSE_LEFT;

        if ((height() - pe->y()) < FormBorderWidth && (width() - pe->x()) < FormBorderWidth){
            _action = ACTION_RESIZE_RIGHT_DOWN;
            setCursorOnAll(Qt::SizeFDiagCursor);
        }else{
            _action = ACTION_MOVE;
            setCursorOnAll(Qt::ClosedHandCursor);
        }
    }else if(pe->buttons() == Qt::RightButton){
        _resizeIconTimer->stop();
        emit FloatingWindowClose(this, _screenListID, QPixmap());

        _animationManager.Create_WindowOpacity(this, [this](){ close(); }, 100, 1, 0).Start();

    }else{ _action = ACTION_NONE; }

    repaint();
}

void ScreenshotFloatingWindowViewer::mouseMoveEvent(QMouseEvent *pe){
    QRect screen = QApplication::desktop()->screenGeometry(this);

    mouseMovePosition = pe->pos();

    if(_action == ACTION_NONE){
        checkAndSetCursors(pe);

    }else if(_mousePress == MOUSE_LEFT && !_resizeMove){
        _resizeMove = true;
    }else if(_action == ACTION_MOVE){
        QPoint moveHere;
        moveHere = pe->globalPos() - mousePressPosition;
        QRect newRect = QRect(moveHere, geometry().size());
        bool snapped;

        snapped = snapEdgeToScreenOrClosestFellow(
            newRect, screen,
            [](QRect& r, int v) { r.moveLeft(v); },
            [](const QRect& r) { return r.left(); },
            [](const QRect& r) { return r.right() + 1; });
        if(!snapped){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) { r.moveRight(v); },
                [](const QRect& r) { return r.right(); },
                [](const QRect& r) { return r.left() - 1; }
                );
        }

        snapped = snapEdgeToScreenOrClosestFellow(
            newRect, screen,
            [](QRect& r, int v) { r.moveTop(v); },
            [](const QRect& r) { return r.top(); },
            [](const QRect& r) { return r.bottom() + 1; }
            );

        if(!snapped){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) { r.moveBottom(v); },
                [](const QRect& r) { return r.bottom(); },
                [](const QRect& r) { return r.top() - 1; }
                );
        }

        move(newRect.topLeft());
    }else{
        QRect newRect = resizeAccordingly(pe);

        if(_action == ACTION_RESIZE_RIGHT_DOWN){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) {	r.setBottom(v); },
                [](const QRect& r) { return r.bottom(); },
                [](const QRect& r) { return r.top() - 1; }
                );
        }

        if(newRect.size() != geometry().size())
            resize(_currentImage.scaled(newRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation).size());

        if(newRect.topLeft() != geometry().topLeft())
            move(newRect.topLeft());
    }
}

void ScreenshotFloatingWindowViewer::mouseReleaseEvent(QMouseEvent *pe){
    _resizeMove = false;
    _mousePress = MOUSE_NONE;
    _action = ACTION_NONE;

    checkAndSetCursors(pe);
    repaint();
}

void ScreenshotFloatingWindowViewer::setCursorOnAll(Qt::CursorShape cursor){
    setCursor(cursor);
}

QRect ScreenshotFloatingWindowViewer::resizeAccordingly(QMouseEvent *pe){
    int newWidth = width();
    int newHeight = height();
    int newX = x();
    int newY = y();

    switch (_action){
        case ACTION_MOVE:
            break;


        case ACTION_RESIZE_RIGHT_DOWN:
            newWidth = pe->x() + mousePressDiffFromBorder.width();
            newHeight = pe->y() + mousePressDiffFromBorder.height();
            newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
            newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;
            break;

        default:
            break;
    }

    return QRect(newX, newY, newWidth, newHeight);
}

void ScreenshotFloatingWindowViewer::checkAndSetCursors(QMouseEvent *pe){
    if ((height() - pe->y()) < FormBorderWidth && (width() - pe->x()) < FormBorderWidth)
        setCursorOnAll(Qt::SizeFDiagCursor);
    else
        setCursorOnAll(Qt::OpenHandCursor);
}

bool ScreenshotFloatingWindowViewer::snapEdgeToScreenOrClosestFellow(
    QRect& newRect,
    const QRect& screen,
    std::function<void(QRect&, int)> setter,
    std::function<int(const QRect&)> getter,
    std::function<int(const QRect&)> oppositeGetter) {

    if(abs(getter(newRect) - getter(screen)) < FORM_STIKY_CLOSER_PIXELS){
        setter(newRect, getter(screen));
        return true;
    }

    if(m_List_WindowViewer){
        int candidate = INT_MIN;
        bool snapped = false;

        for(int i = 0; i < m_List_WindowViewer->size(); ++i){
            if (m_List_WindowViewer->at(i) != this){
                int currentDistance = abs(getter(newRect) - getter(m_List_WindowViewer->at(i)->geometry()));
                int oppositeDistance = abs(getter(newRect) - oppositeGetter(m_List_WindowViewer->at(i)->geometry()));

                if(currentDistance < FORM_STIKY_CLOSER_PIXELS && currentDistance < abs(candidate - getter(m_List_WindowViewer->at(i)->geometry()))) {
                    candidate = getter(m_List_WindowViewer->at(i)->geometry());
                    snapped = true;
                }

                if(oppositeDistance < FORM_STIKY_CLOSER_PIXELS && oppositeDistance < abs(candidate - oppositeGetter(m_List_WindowViewer->at(i)->geometry()))) {
                    candidate = oppositeGetter(m_List_WindowViewer->at(i)->geometry());
                    snapped = true;
                }
            }
        }

        if(snapped){
            setter(newRect, candidate);
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////[-------Конец--------]/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
