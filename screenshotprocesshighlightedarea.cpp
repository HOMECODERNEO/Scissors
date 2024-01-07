#include "screenshotprocesshighlightedarea.h"

ScreenshotProcessHighlightedArea::ScreenshotProcessHighlightedArea(QWidget *parent) : QWidget{parent}{
    setParent(parent);
    setMouseTracking(true);
    setCursorOnAll(Qt::OpenHandCursor);

    connect(this, SIGNAL(AreaMove(QRect)), parent, SLOT(AreaMove(QRect)));
    connect(this, SIGNAL(CreateScreenshot(QRect)), parent, SLOT(CreateScreenshot(QRect)));
}

void ScreenshotProcessHighlightedArea::Show(QRect geometry){
    show();
    setGeometry(geometry);
    _areaInfoShow = GetProgramSettings().Get_ShowScreenshotZoneGeometry();
}

void ScreenshotProcessHighlightedArea::Hide(){
    hide();
}

void ScreenshotProcessHighlightedArea::paintEvent(QPaintEvent *){
    _painter.begin(this);
    
    QFont font = _painter.font();
    QRect rectWidthHeight = rect();

    int fontWidth = MapValue(rect().width(), 0, 1920, 5, 70);
    int fontHeight = MapValue(rect().height(), 0, 1080, 10, 70);

    int fontSize = qMin(fontWidth, fontHeight);

    if(_areaInfoShow){
        if(_areaResize){
            font.setPointSize(fontSize); // TEXT SIZE
            _painter.setPen(QColor(255, 255, 255, 150)); // TEXT COLOR
            _painter.setFont(font);

            _painter.drawText(rectWidthHeight, QString::number(width()) + " x " + QString::number(height()) + " px", QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        }else if(_areaMove){
            font.setPointSize(fontSize); // TEXT SIZE
            _painter.setPen(QColor(255, 255, 255, 150)); // TEXT COLOR
            _painter.setFont(font);

            _painter.drawText(rectWidthHeight, "X: [" + QString::number(x()) + "] Y: [" + QString::number(y()) + "]", QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        }
    }

    _painter.setBrush(QBrush(QColor(0, 0, 0, 25)));
    _painter.drawRect(-1, -1, width() + 1, height() + 1);

    _painter.end();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////[-------Событие перетаскивания--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcessHighlightedArea::setCursorOnAll(Qt::CursorShape cursor){
    setCursor(cursor);
}

void ScreenshotProcessHighlightedArea::mousePressEvent(QMouseEvent *pe){

    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPosition();
    mousePressDiffFromBorder.setWidth(width() - pe->position().x());
    mousePressDiffFromBorder.setHeight(height() - pe->position().y());

    if(pe->buttons() == Qt::RightButton){

        emit CreateScreenshot(QRect(pos().x(), pos().y(), width(), height()));

        return;
    }

    if(pe->buttons() == Qt::LeftButton){
        if(pe->position().x() < FormBorderWidth && pe->position().y() < FormBorderWidth){
            _modeAction = ACTION_RESIZE_LEFT_UP;
            setCursorOnAll(Qt::SizeFDiagCursor);
        }else if(pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){
            _modeAction = ACTION_RESIZE_LEFT_DOWN;
            setCursorOnAll(Qt::SizeBDiagCursor);
        }else if(pe->position().y() < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
            _modeAction = ACTION_RESIZE_RIGHT_UP;
            setCursorOnAll(Qt::SizeBDiagCursor);
        }else if((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
            _modeAction = ACTION_RESIZE_RIGHT_DOWN;
            setCursorOnAll(Qt::SizeFDiagCursor);
        }else if(pe->position().x() < FormBorderWidth){
            _modeAction = ACTION_RESIZE_HOR_LEFT;
            setCursorOnAll(Qt::SizeHorCursor);
        }else if((width() - pe->position().x()) < FormBorderWidth){
            _modeAction = ACTION_RESIZE_HOR_RIGHT;
            setCursorOnAll(Qt::SizeHorCursor);
        }else if(pe->position().y() < FormBorderWidth){
            _modeAction = ACTION_RESIZE_VER_UP;
            setCursorOnAll(Qt::SizeVerCursor);
        }else if((height() - pe->position().y()) < FormBorderWidth){
            _modeAction = ACTION_RESIZE_VER_DOWN;
            setCursorOnAll(Qt::SizeVerCursor);
        }else{
            _modeAction = ACTION_MOVE;
            setCursorOnAll(Qt::ClosedHandCursor);
        }
    }else{ _modeAction = ACTION_NONE; }

    if(_modeAction != ACTION_NONE && _modeAction != ACTION_MOVE)
        _areaResize = true;
    else if(_modeAction == ACTION_MOVE)
        _areaMove = true;

    repaint();
}

QRect ScreenshotProcessHighlightedArea::resizeAccordingly(QMouseEvent *pe){
    int newWidth = width();
    int newHeight = height();
    int newX = x();
    int newY = y();

    switch(_modeAction){
        case ACTION_MOVE:
            break;

        case ACTION_RESIZE_HOR_RIGHT:
            newWidth = pe->position().x() + mousePressDiffFromBorder.width();
            newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
            break;

        case ACTION_RESIZE_VER_DOWN:
            newHeight = pe->position().y() + mousePressDiffFromBorder.height();
            newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;
            break;

        case ACTION_RESIZE_HOR_LEFT:
            newY = pos().y();
            newHeight = height();

            newWidth = mousePressGlobalPosition.x() - pe->globalPosition().x() + mousePressPosition.x() + mousePressDiffFromBorder.width();

            if(newWidth < FormMinimumWidth){
                newWidth = FormMinimumWidth;
                newX = mousePressGlobalPosition.x() + mousePressDiffFromBorder.width() - FormMinimumWidth;
            }else
                newX = pe->globalPosition().x() - mousePressPosition.x();

            break;

        case ACTION_RESIZE_VER_UP:
            newX = pos().x();
            newWidth = width();

            newHeight = mousePressGlobalPosition.y() - pe->globalPosition().y() + mousePressPosition.y() + mousePressDiffFromBorder.height();

            if (newHeight < FormMinimumHeight){
                newHeight = FormMinimumHeight;
                newY = mousePressGlobalPosition.y() + mousePressDiffFromBorder.height() - FormMinimumHeight;
            }else
                newY = pe->globalPosition().y() - mousePressPosition.y();

            break;


        case ACTION_RESIZE_RIGHT_DOWN:
            newWidth = pe->position().x() + mousePressDiffFromBorder.width();
            newHeight = pe->position().y() + mousePressDiffFromBorder.height();
            newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
            newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;
            break;

        case ACTION_RESIZE_RIGHT_UP:
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

        case ACTION_RESIZE_LEFT_DOWN:
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

        case ACTION_RESIZE_LEFT_UP:
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

        default:
            break;
    }

    return QRect(newX, newY, newWidth, newHeight);
}

void ScreenshotProcessHighlightedArea::mouseMoveEvent(QMouseEvent *pe){
    if(_modeAction == ACTION_NONE)
        checkAndSetCursors(pe);

    else if(_modeAction == ACTION_MOVE){
        QPointF moveHere;
        moveHere = pe->globalPosition() - mousePressPosition;
        QRect newRect = QRect(moveHere.toPoint(), geometry().size());

        repaint();
        move(newRect.topLeft());
    }else{
        QRect newRect = resizeAccordingly(pe);

        if(newRect.size() != geometry().size())
            resize(newRect.size());

        if(newRect.topLeft() != geometry().topLeft())
            move(newRect.topLeft());
    }

    emit AreaMove(QRect(pos().x(), pos().y(), width(), height()));
}

void ScreenshotProcessHighlightedArea::mouseReleaseEvent(QMouseEvent *pe){
    _areaResize = false;
    _areaMove = false;

    _modeAction = ACTION_NONE;
    checkAndSetCursors(pe);
    repaint();
}

void ScreenshotProcessHighlightedArea::checkAndSetCursors(QMouseEvent *pe){
    if (pe->position().x() < FormBorderWidth && pe->position().y() < FormBorderWidth) { setCursorOnAll(Qt::SizeFDiagCursor); }
    else if (pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){ setCursorOnAll(Qt::SizeBDiagCursor); }
    else if (pe->position().y() < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursorOnAll(Qt::SizeBDiagCursor);}
    else if ((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursorOnAll(Qt::SizeFDiagCursor); }
    else if (pe->position().x() < FormBorderWidth){ setCursorOnAll(Qt::SizeHorCursor); }
    else if ((width() - pe->position().x()) < FormBorderWidth){ setCursorOnAll(Qt::SizeHorCursor); }
    else if (pe->position().y() < FormBorderWidth){ setCursorOnAll(Qt::SizeVerCursor); }
    else if ((height() - pe->position().y()) < FormBorderWidth){ setCursorOnAll(Qt::SizeVerCursor); }
    else setCursorOnAll(Qt::OpenHandCursor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////[-------Конец--------]/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

long ScreenshotProcessHighlightedArea::MapValue(long x, long in_min, long in_max, long out_min, long out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
