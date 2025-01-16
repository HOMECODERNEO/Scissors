#include "screenshotfloatingwindowviewer.h"

// Конструктор
ScreenshotFloatingWindowViewer::ScreenshotFloatingWindowViewer(QPixmap image, QList<ScreenshotFloatingWindowViewer*> *List_WindowViewer, QWidget *parent) : QWidget(parent){
    setWindowTitle("Screenshot");
    setParent(parent);

    _currentImage = image;
    m_List_WindowViewer = List_WindowViewer;

    _resizeIconTimer = new QTimer();

    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);

    connect(_resizeIconTimer, &QTimer::timeout, this, &ScreenshotFloatingWindowViewer::updateCursorPositionTimer);
    connect(this, SIGNAL(FloatingWindowClose(ScreenshotFloatingWindowViewer*)), parent, SLOT(FloatingWindowClose(ScreenshotFloatingWindowViewer*)));
    connect(this, SIGNAL(GetCurrentScreenGeometry()), parent, SLOT(GetCurrentScreenGeometry()));

    QRect screenRect = emit GetCurrentScreenGeometry();
    int centreX = (screenRect.width() / 2) - (_currentImage.width() / 2);
    int centreY = (screenRect.height() / 2) - (_currentImage.height() / 2);
    setGeometry(centreX, centreY, _currentImage.width(), _currentImage.height());

    this->setFocus();

    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();

    _resizeIconTimer->start(15);
}

// Событые перерысовки окна
void ScreenshotFloatingWindowViewer::paintEvent(QPaintEvent*){
    _painter.begin(this);

    // Рисуем главное изображение
    _painter.drawPixmap(0, 0, width(), height(), _currentImage.scaled(this->size().width() - 2, this->size().height() - 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    // Рисуем кнопки по углам для изменения размера
    _painter.drawPixmap(width() - 22, height() - 22, 18, 18, QPixmap(_resizeIconIsHover == 1 ? ":/Buttons/Resourse/Buttons/Resize_Hover.png" : ":/Buttons/Resourse/Buttons/Resize.png"));
    _painter.drawPixmap(5, height() - 22, 18, 18, QPixmap(_resizeIconIsHover == 2 ? ":/Buttons/Resourse/Buttons/Resize_Hover.png" : ":/Buttons/Resourse/Buttons/Resize.png").transformed(QTransform().rotate(90)));

    _painter.setPen(QPen(QColor(255, 238, 88), 2, Qt::SolidLine, Qt::SquareCap));
    _painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
    _painter.drawRect(0, 0, width(), height());
    _painter.end();
}

// Для создания эффекта "HOVER" на нарисованных кнопках, проверяем положение курсора над кнопками
void ScreenshotFloatingWindowViewer::updateCursorPositionTimer(){
    if(QRect(width() - 22, height() - 22, 22, 22).contains(mapFromGlobal(QCursor::pos()))){
        _resizeIconIsHover = 1;
        repaint();
    }else if(QRect(0, height() - 18, 18, 18).contains(mapFromGlobal(QCursor::pos()))){
        _resizeIconIsHover = 2;
        repaint();
    }else if(_resizeIconIsHover == 1 || _resizeIconIsHover == 2){
        _resizeIconIsHover = 0;
        repaint();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////[-------Событие перетаскивания--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotFloatingWindowViewer::wheelEvent(QWheelEvent *event){
    double scaleValue = 0;
    const double scaleFactor = 0.1;

    if (event->angleDelta().y() > 0)
        scaleValue = 1.0 + scaleFactor;
    else
        scaleValue = 1.0 / (1.0 + scaleFactor);

    int newWidth = static_cast<int>(width() * scaleValue);
    int newHeight = static_cast<int>(height() * scaleValue);

    float aspectRatio = static_cast<float>(newWidth) / newHeight;

    if(newWidth / aspectRatio <= newHeight)
        newHeight = newWidth / aspectRatio;
    else
        newWidth = newHeight * aspectRatio;

    int newX = x() - (newWidth - width()) / 2;
    int newY = y() - (newHeight - height()) / 2;

    if(newHeight > FormMinimumHeight)
        _animationManager.Create_ObjectGeometry(this, nullptr, 75, this->geometry(), QRect(newX, newY, newWidth, newHeight)).Start();

    event->accept();
}

void ScreenshotFloatingWindowViewer::mousePressEvent(QMouseEvent *pe){
    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPosition();
    mousePressDiffFromBorder.setWidth(width() - pe->position().x());
    mousePressDiffFromBorder.setHeight(height() - pe->position().y());

    if (pe->buttons() == Qt::LeftButton){

        if ((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
            _action = ACTION_RESIZE_RIGHT_DOWN;
            setCursorOnAll(Qt::SizeFDiagCursor);
        }else if (pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){
            _action = ACTION_RESIZE_LEFT_DOWN;
            setCursorOnAll(Qt::SizeBDiagCursor);
        }else{
            _action = ACTION_MOVE;
            setCursorOnAll(Qt::ClosedHandCursor);
        }
    }else if(pe->buttons() == Qt::RightButton){
        _resizeIconTimer->stop();
        emit FloatingWindowClose(this);

    }else{ _action = ACTION_NONE; }

    repaint();
}

void ScreenshotFloatingWindowViewer::mouseMoveEvent(QMouseEvent *pe){
    QRect screen = QGuiApplication::primaryScreen()->geometry();

    mouseMovePosition = pe->pos();

    if(_action == ACTION_NONE){
        checkAndSetCursors(pe);

    }else if(_action == ACTION_MOVE){
        QPointF moveHere = QPointF(pe->globalPosition() - mousePressPosition);
        QRect newRect = QRect(moveHere.toPoint(), geometry().size());

        //////////////////////////////////////////////////////////////////////

        if(!snapEdgeToScreenOrClosestFellow(newRect, screen, [](QRect& r, int v) { r.moveLeft(v); }, [](const QRect& r) { return r.left(); }, [](const QRect& r) { return r.right() + 1; } ))
            snapEdgeToScreenOrClosestFellow(newRect, screen, [](QRect& r, int v) { r.moveRight(v); }, [](const QRect& r) { return r.right(); }, [](const QRect& r) { return r.left() - 1; } );

        if(!snapEdgeToScreenOrClosestFellow(newRect, screen, [](QRect& r, int v) { r.moveTop(v); }, [](const QRect& r) { return r.top(); }, [](const QRect& r) { return r.bottom() + 1; } ))
            snapEdgeToScreenOrClosestFellow(newRect, screen, [](QRect& r, int v) { r.moveBottom(v); }, [](const QRect& r) { return r.bottom(); }, [](const QRect& r) { return r.top() - 1; } );

        //////////////////////////////////////////////////////////////////////

        move(newRect.topLeft());
    }else{
        QRect newRect = resizeAccordingly(pe);

        // Проверка соотношения сторон изображения
        if(_action == ACTION_RESIZE_RIGHT_DOWN || _action == ACTION_RESIZE_LEFT_DOWN){

            float aspectRatio = static_cast<float>(_currentImage.width()) / _currentImage.height();

            if(newRect.width() / aspectRatio <= newRect.height())
                newRect.setHeight(newRect.width() / aspectRatio);
            else
                newRect.setWidth(newRect.height() * aspectRatio);

            snapEdgeToScreenOrClosestFellow(newRect, screen, [](QRect& r, int v) { r.setBottom(v); }, [](const QRect& r) { return r.bottom(); }, [](const QRect& r) { return r.top() - 1; } );
        }

        // Устанавливаем новую позицию (Фикс сьезжающего окна при изменении размера за левый угол)
        if(_action == ACTION_RESIZE_LEFT_DOWN){
            int dx = newRect.width() - geometry().width();
            newRect.moveLeft(geometry().left() - dx);
        }

        // Изменяем размер
        resize(newRect.size());

        // Изменяем положение
        move(newRect.topLeft());
    }
}

void ScreenshotFloatingWindowViewer::mouseReleaseEvent(QMouseEvent *pe){
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
        case ACTION_MOVE:{ break; }

        case ACTION_RESIZE_RIGHT_DOWN:{
            newWidth = pe->position().x() + mousePressDiffFromBorder.width();
            newHeight = pe->position().y() + mousePressDiffFromBorder.height();
            newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
            newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;

            break;
        }

        case ACTION_RESIZE_LEFT_DOWN:{
            newHeight = pe->position().y() + mousePressDiffFromBorder.height();

            if (newHeight < FormMinimumHeight)
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

        default:{ break; }
    }

    return QRect(newX, newY, newWidth, newHeight);
}

void ScreenshotFloatingWindowViewer::checkAndSetCursors(QMouseEvent *pe){
    if ((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth)
        setCursorOnAll(Qt::SizeFDiagCursor); // Правый нижний угол
    else if (pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth)
        setCursorOnAll(Qt::SizeBDiagCursor); // Левый нижний угол
    else
        setCursorOnAll(Qt::OpenHandCursor); // Открытая ладошка
}

// Эффект магнита к краям экрана и других окон
bool ScreenshotFloatingWindowViewer::snapEdgeToScreenOrClosestFellow(
    QRect& newRect,
    const QRect& screen,
    std::function<void(QRect&, int)> setter,
    std::function<int(const QRect&)> getter,
    std::function<int(const QRect&)> oppositeGetter){

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
