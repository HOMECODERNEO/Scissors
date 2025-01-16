#include "screenshotprocesshighlightedarea.h"

ScreenshotProcessHighlightedArea::ScreenshotProcessHighlightedArea(QWidget *parent) : QWidget{parent}{
    setParent(parent);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);

    _drawArea = QPixmap(1, 1);
}

void ScreenshotProcessHighlightedArea::Show(QRect geometry){
    _areaInfoShow = GetProgramSettings().Get_ShowScreenshotZoneGeometry();

    _drawArea = _drawArea.scaled(geometry.width(), geometry.height());
    ClearDrawArea();

    setGeometry(geometry);    show();
}

void ScreenshotProcessHighlightedArea::Hide(){
    hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////[-------Область алгоритма для рисования--------]///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawPenData ScreenshotProcessHighlightedArea::GetPenData(){ return _penData; }
DrawLineData ScreenshotProcessHighlightedArea::GetLineData(){ return _lineData; }
DrawEraserData ScreenshotProcessHighlightedArea::GetEraserData(){ return _eraserData; }
DrawFigureData ScreenshotProcessHighlightedArea::GetFigureData(){ return _figureData; }

void ScreenshotProcessHighlightedArea::UpdateToolPanel_PenData(DrawPenData data){ _penData = data; }
void ScreenshotProcessHighlightedArea::UpdateToolPanel_EraserData(DrawEraserData data){ _eraserData = data; }

void ScreenshotProcessHighlightedArea::UpdateToolPanel_LineData(DrawLineData data){
    _lineData = data;

    // Присваиваем изменённые данные классу линии
    if(_Line != nullptr)
        _Line->SetData(_lineData);
}

void ScreenshotProcessHighlightedArea::UpdateToolPanel_FigureData(DrawFigureData data){
    _figureData = data;

    // Если тип фигуры изменился то затираем старый класс и создаем новую фигуру
    if(_Figure != nullptr){
        if(_Figure->getType() != _figureData.GetType())
            UpdateFigure(_figureData.GetType(), this);

        // Присваиваем изменённые данные классу фигуры
        _Figure->SetData(_figureData);
    }
}

PAINTING_MODE ScreenshotProcessHighlightedArea::GetDrawMode(){
    return _drawMode;
}

void ScreenshotProcessHighlightedArea::SetDrawMode(PAINTING_MODE mode){

    switch(mode){
        case MODE_NONE: setCursor(Qt::ArrowCursor); break;
        case MODE_PEN: setCursor(Qt::CrossCursor); break;
        case MODE_ERASER: setCursor(Qt::CrossCursor); break;

        case MODE_LINE:{

            if(_Line == nullptr){
                _Line = std::make_unique<LineObjectClass>(this);

                connect(_Line.get(), &LineObjectClass::LineDeleteRequest, this, &ScreenshotProcessHighlightedArea::LineDeleteRequest);
                connect(_Line.get(), &LineObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);

                _Line->SetData(_lineData);
            }

            setCursor(Qt::CrossCursor);
            break;
        }

        case MODE_FIGURE:{

            if(_Figure == nullptr){
                CreateFigure(_figureData.GetType(), this);

                _Figure->SetData(_figureData);
            }

            setCursor(Qt::CrossCursor);
            break;
        }
    }

    // Удаляем обьекты если произошло переключение режима
    if(_drawMode == MODE_LINE && mode != MODE_LINE && _Line->isDisplayed())
        DeleteLine(&_drawArea, true);

    // Удаляем обьекты если произошло переключение режима
    if(_drawMode == MODE_FIGURE && mode != MODE_FIGURE && _Figure->isDisplayed())
        DeleteFigure(&_drawArea, true);

    // Присваиваем новый режим
    _drawMode = mode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////[-------Событие отрысовки--------]////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Событие отрисовщика
void ScreenshotProcessHighlightedArea::paintEvent(QPaintEvent *){
    _painter.begin(this);

    if(_areaInfoShow && _drawMode == MODE_NONE){
        QFont font = _painter.font();
        QRect rectWidthHeight = rect();

        int fontWidth = MapValue(rect().width(), 0, ETALON_WIDTH, 5, 70);
        int fontHeight = MapValue(rect().height(), 0, ETALON_HEIGHT, 10, 70);

        int fontSize = qMin(fontWidth, fontHeight);

        if(_modeAction != ACTION_NONE && _modeAction != ACTION_MOVE){
            font.setPointSize(fontSize);
            _painter.setPen(QColor(255, 255, 255, 150));
            _painter.setFont(font);

            _painter.drawText(rectWidthHeight, "[" + QString::number(width()) + "] x [" + QString::number(height()) + "] px", QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        }else if(_modeAction == ACTION_MOVE){
            font.setPointSize(fontSize);
            _painter.setPen(QColor(255, 255, 255, 150));
            _painter.setFont(font);

            _painter.drawText(rectWidthHeight, "X: [" + QString::number(x()) + "] Y: [" + QString::number(y()) + "]", QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        }
    }

    // Отрысовываем основной слой где будут рисунки
    _painter.drawPixmap(rect(), _drawArea);

    ////////////////////////////////////////////////////// Рисуем обводку для наглядности размеров
    if(_areaMouseActive && (GetDrawMode() == MODE_PEN || GetDrawMode() == MODE_ERASER || GetDrawMode() == MODE_LINE)){
        QColor color = Qt::white;
        int size = 2;

        if(GetDrawMode() == MODE_PEN){
            color = _penData.GetColor();
            size = _penData.GetWidth();
        }else if(GetDrawMode() == MODE_ERASER){
            color = Qt::white;
            size = _eraserData.GetWidth();
        }else if(GetDrawMode() == MODE_LINE){
            color = _lineData.GetMainColor();
            size = _lineData.GetWidth();
        }

        _painter.setPen(color);
        _painter.setBrush(Qt::NoBrush);

        _painter.drawEllipse(mouseMovePosition, size, size);
    }
    ////////////////////////////////////////////////////// Рисуем обводку для наглядности размеров

    if(GetDrawMode() != MODE_NONE)
        DrawPaintEvent(_painter);

    _painter.end();
}

void ScreenshotProcessHighlightedArea::DrawPaintEvent(QPainter &painter){
    Q_UNUSED(painter);

    switch(GetDrawMode()){
        case MODE_NONE: break;
        case MODE_PEN: break;
        case MODE_ERASER: break;

        case MODE_LINE:{

            // Процес создания линии
            if(_mousePress && _Line != nullptr && (mouseDrawMovePosition - mouseDrawPressPosition).manhattanLength() >= 1){

                if(!_Line->isDisplayed())
                    _Line->Show();

                _Line->SetVector(mouseDrawPressPosition, mouseDrawMovePosition);
                _Line->update();
            }

            break;
        }

        case MODE_FIGURE:{

            // Процес создания фигуры
            if(_mousePress && _Figure != nullptr && (mouseDrawMovePosition - mouseDrawPressPosition).manhattanLength() >= 1){
                QRect data = ConvertCoords(mouseDrawPressPosition, mouseDrawMovePosition);

                if(!_Figure->isDisplayed())
                    _Figure->Show();

                _Figure->SetPos(data.x(), data.y());
                _Figure->SetSize(data.width(), data.height());
            }

            break;
        }
    }
}

void ScreenshotProcessHighlightedArea::DrawMouseEvent(){
    switch(GetDrawMode()){
        case MODE_NONE: break;

        case MODE_PEN:{
            QPainter painter(&_drawArea);
            painter.setPen(QPen(_penData.GetColor(), _penData.GetWidth()));
            painter.drawEllipse(mouseDrawMovePosition, _penData.GetWidth() / 2, _penData.GetWidth() / 2);

            // Проходимся по вектору движения мышки (устранения точнечного рисования)
            if(!mouseDrawPressPosition.isNull()){

                QPoint posStart = mouseDrawPressPosition;
                QPoint delta = mouseDrawMovePosition - posStart;
                float length = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

                for(float t = 0; t < 1; (t += 0.1f / length))
                    painter.drawEllipse(posStart + delta * t, _penData.GetWidth() / 2, _penData.GetWidth() / 2);
            }

            mouseDrawPressPosition = mouseDrawMovePosition;
            break;
        }

        case MODE_ERASER:{
            QPainter painter(&_drawArea);

            painter.setPen(Qt::transparent);
            painter.setBrush(Qt::transparent);

            painter.setCompositionMode(QPainter::CompositionMode_Clear);
            painter.drawEllipse(mouseDrawMovePosition, _eraserData.GetWidth(), _eraserData.GetWidth());

            // Проходимся по вектору движения мышки (устранения точнечного рисования)
            if(!mouseDrawPressPosition.isNull()){

                QPoint posStart = mouseDrawPressPosition;
                QPoint delta = mouseMovePosition - posStart;
                float length = sqrt(delta.x() * delta.x() + delta.y() * delta.y());

                for (float t = 0; t < 1; (t += 0.1f / length) )
                    painter.drawEllipse(posStart + delta * t, _eraserData.GetWidth(), _eraserData.GetWidth());
            }

            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

            mouseDrawPressPosition = mouseDrawMovePosition;
            break;
        }

        case MODE_LINE: break;
        case MODE_FIGURE: break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////[-------Дополнительные функции--------]//////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Очистка области рисования
void ScreenshotProcessHighlightedArea::ClearDrawArea(){
    _drawArea.fill(QColor(0, 0, 0, 0));
    repaint();
}

// Запрос от класса линии на ее удаление с холста
void ScreenshotProcessHighlightedArea::LineDeleteRequest(){
    DeleteLine(&_drawArea, false);
}

// Создание фигуры
void ScreenshotProcessHighlightedArea::UpdateFigure(FIGURE_TYPE type, QWidget *parent){
    if(_Figure == nullptr)
        return;

    bool displaying = _Figure->isDisplayed();
    QRect geometryData = _Figure->getGeometry();
    disconnect(_Figure.get(), &FigureObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);

    switch(type){
        case FIGURE_TYPE_COUNT: return;

        case FIGURE_TYPE_RECT: _Figure.reset(new SquareObject(parent)); break;
        case FIGURE_TYPE_CIRCLE: _Figure.reset(new CircleObject(parent)); break;
        case FIGURE_TYPE_TRIANGLE: _Figure.reset(new TriangleObject(parent)); break;
        case FIGURE_TYPE_DIAMOND: _Figure.reset(new DiamondObject(parent)); break;
        case FIGURE_TYPE_STAR: _Figure.reset(new StarObject(parent)); break;
    }

    _Figure->SetData(_figureData);
    _Figure->SetGeometry(geometryData);
    _Figure->SetMarkerSize(FIGURE_MARKER_SIZE);
    connect(_Figure.get(), &FigureObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);

    if(displaying)
        _Figure->Show();
}

// Создание фигуры
void ScreenshotProcessHighlightedArea::CreateFigure(FIGURE_TYPE type, QWidget *parent){
    if(_Figure != nullptr)
        return;

    switch(type){
        case FIGURE_TYPE_COUNT: return;

        case FIGURE_TYPE_RECT: _Figure = std::make_unique<SquareObject>(parent); break;
        case FIGURE_TYPE_CIRCLE: _Figure = std::make_unique<CircleObject>(parent); break;
        case FIGURE_TYPE_TRIANGLE: _Figure = std::make_unique<TriangleObject>(parent); break;
        case FIGURE_TYPE_DIAMOND: _Figure = std::make_unique<DiamondObject>(parent); break;
        case FIGURE_TYPE_STAR: _Figure = std::make_unique<StarObject>(parent); break;
    }

    _Figure->SetMarkerSize(FIGURE_MARKER_SIZE);
    connect(_Figure.get(), &FigureObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);
}

// Удаление нарисованной активной фигуры
void ScreenshotProcessHighlightedArea::DeleteFigure(QPixmap *drawArea, bool delete_permanently){
    if(_Figure != nullptr){
        if(drawArea != nullptr){
            QPainter painter(drawArea);
            _Figure->DrawAreaPaintFigure(&painter);
        }

        _Figure->Hide();

        if(delete_permanently){
            disconnect(_Figure.get(), &FigureObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);
            _Figure.reset();
        }
    }
}

// Удаление нарисованной активной линии
void ScreenshotProcessHighlightedArea::DeleteLine(QPixmap *drawArea, bool delete_permanently){
    if(_Line != nullptr){
        if(drawArea != nullptr){
            QPainter painter(drawArea);
            _Line->DrawAreaPaintLine(&painter);
        }

        _Line->Hide();

        if(delete_permanently){
            disconnect(_Line.get(), &LineObjectClass::LineDeleteRequest, this, &ScreenshotProcessHighlightedArea::LineDeleteRequest);
            disconnect(_Line.get(), &LineObjectClass::RightClick, this, &ScreenshotProcessHighlightedArea::FigureCreateScreenshot);

            _Line.reset();
        }
    }
}

// Инициализация создания скриншота
void ScreenshotProcessHighlightedArea::FigureCreateScreenshot(){
    // Удаляем фигуру если она была, и переносим ее проекцию на холст
    DeleteFigure(&_drawArea, true);
    DeleteLine(&_drawArea, true);

    // Создаем скриншот
    emit CreateScreenshot(QRect(pos().x(), pos().y(), width(), height()), _drawArea);
}

void ScreenshotProcessHighlightedArea::LineCapInit(QComboBox* start, QComboBox* end) {
    const int imageWidth = 75;
    const int imageHeight = 25;

    end->setIconSize(QSize(imageWidth, imageHeight));
    start->setIconSize(QSize(imageWidth, imageHeight));

    // Добавляем элементы в оба QComboBox
    for(int i = 0; i < LINESTYLE_TYPE_COUNT; i++){        
        QPixmap pixmap(imageWidth, imageHeight);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        // Создаём временный объект линии
        LineObjectClass tempLine;
        DrawLineData tempLineData;
        tempLineData.SetWidth(3);
        tempLineData.SetMainColor(Qt::white);
        tempLineData.SetSecondaryColor(Qt::red);
        tempLineData.SetLineStyle(Qt::SolidLine);
        tempLineData.SetLineCapStart(static_cast<LINESTYLE_CAP>(i));

        tempLine.SetData(tempLineData);
        tempLine.SetVector(QPoint(20, imageHeight / 2), QPoint(imageWidth, imageHeight / 2));

        tempLine.DrawAreaPaintLine(&painter);

        start->addItem(QIcon(pixmap), "");
        end->addItem(QIcon(pixmap.transformed(QTransform().rotate(180))), "");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////[-------События мыши--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcessHighlightedArea::enterEvent(QEnterEvent *){
    _areaMouseActive = true;
    update();
}

void ScreenshotProcessHighlightedArea::leaveEvent(QEvent *){
    _areaMouseActive = false;
    update();
}

void ScreenshotProcessHighlightedArea::mousePressEvent(QMouseEvent *pe){

    // Program
    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPosition();
    mousePressDiffFromBorder.setWidth(width() - pe->position().x());
    mousePressDiffFromBorder.setHeight(height() - pe->position().y());
    // Draw
    mouseDrawPressPosition.setX(MapValue(mousePressPosition.x(), 0, geometry().width(), 0, _drawArea.width()));
    mouseDrawPressPosition.setY(MapValue(mousePressPosition.y(), 0, geometry().height(), 0, _drawArea.height()));

    // Reset move point
    mouseDrawMovePosition = mouseDrawPressPosition;

    // Right button
    if(pe->buttons() == Qt::RightButton){
        // Создаем скриншот
        emit CreateScreenshot(QRect(pos().x(), pos().y(), width(), height()), _drawArea);

        return;
    }

    // Left button
    if(pe->buttons() == Qt::LeftButton){

        // Удаляем фигуру если она была, и переносим ее проекцию на холст
        if(_Figure != nullptr && _Figure->isDisplayed())
            DeleteFigure(&_drawArea, false);

        // Удаляем линию если она была, и переносим ее проекцию на холст
        if(_Line != nullptr && _Line->isDisplayed())
            DeleteLine(&_drawArea, false);

        _mousePress = true;

        if(_drawMode == MODE_NONE){
            if(pe->position().x() < FormBorderWidth && pe->position().y() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_LEFT_UP;
                setCursor(Qt::SizeFDiagCursor);
            }else if(pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_LEFT_DOWN;
                setCursor(Qt::SizeBDiagCursor);
            }else if(pe->position().y() < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_RIGHT_UP;
                setCursor(Qt::SizeBDiagCursor);
            }else if((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_RIGHT_DOWN;
                setCursor(Qt::SizeFDiagCursor);
            }else if(pe->position().x() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_HOR_LEFT;
                setCursor(Qt::SizeHorCursor);
            }else if((width() - pe->position().x()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_HOR_RIGHT;
                setCursor(Qt::SizeHorCursor);
            }else if(pe->position().y() < FormBorderWidth){
                _modeAction = ACTION_RESIZE_VER_UP;
                setCursor(Qt::SizeVerCursor);
            }else if((height() - pe->position().y()) < FormBorderWidth){
                _modeAction = ACTION_RESIZE_VER_DOWN;
                setCursor(Qt::SizeVerCursor);
            }else{
                _modeAction = ACTION_MOVE;
                setCursor(Qt::ClosedHandCursor);
            }
        }

    }else
        _modeAction = ACTION_NONE;

    _areaMouseActive = true;

    repaint();
}

QRect ScreenshotProcessHighlightedArea::resizeAccordingly(QMouseEvent *pe){
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

void ScreenshotProcessHighlightedArea::mouseMoveEvent(QMouseEvent *pe){
    // Program
    mouseMovePosition = pe->pos();

    // Draw
    mouseDrawMovePosition.setX(MapValue(mouseMovePosition.x(), 0, geometry().width(), 0, _drawArea.width()));
    mouseDrawMovePosition.setY(MapValue(mouseMovePosition.y(), 0, geometry().height(), 0, _drawArea.height()));

    if(!_areaMouseActive)
        return;

    if(GetDrawMode() == MODE_NONE){

        if(_modeAction == ACTION_NONE){
            checkAndSetCursors(pe);
            repaint();
            return;
        }

    }else{

        if(_mousePress)
            DrawMouseEvent();
    }

    if(_modeAction == ACTION_MOVE){
        QPointF moveHere;
        moveHere = pe->globalPosition() - mousePressPosition;

        QRect screenRect = emit GetCurrentScreenGeometry();
        QRect newRect = QRect(moveHere.toPoint(), geometry().size());

        if(newRect.x() < 0) newRect.setX(0);
        if(newRect.y() < 0) newRect.setY(0);

        if(newRect.x() + newRect.width() > screenRect.width()) newRect.setX(screenRect.width() - newRect.width());
        if(newRect.y() + newRect.height() > screenRect.height()) newRect.setY(screenRect.height() - newRect.height());

        move(newRect.topLeft());
        repaint();

    }else{

        QRect newRect = resizeAccordingly(pe);

        // Ограничение размера объекта по размеру экрана
        QRect screenRect = emit GetCurrentScreenGeometry();
        newRect = newRect.intersected(screenRect);

        if(newRect.size() != geometry().size())
            resize(newRect.size());

        if(newRect.topLeft() != geometry().topLeft())
            move(newRect.topLeft());
    }

    emit AreaMove(QRect(pos().x(), pos().y(), width(), height()));
}

void ScreenshotProcessHighlightedArea::mouseReleaseEvent(QMouseEvent *pe){
    _mousePress = false;
    _areaMouseActive = true;

    _modeAction = ACTION_NONE;
    checkAndSetCursors(pe);
    repaint();
}

void ScreenshotProcessHighlightedArea::checkAndSetCursors(QMouseEvent *pe){
    if(_drawMode != MODE_NONE)
        return;

    if(pe->position().x() < FormBorderWidth && pe->position().y() < FormBorderWidth) { setCursor(Qt::SizeFDiagCursor); }
    else if(pe->position().x() < FormBorderWidth && (height() - pe->position().y()) < FormBorderWidth){ setCursor(Qt::SizeBDiagCursor); }
    else if(pe->position().y() < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeBDiagCursor);}
    else if((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeFDiagCursor); }
    else if(pe->position().x() < FormBorderWidth){ setCursor(Qt::SizeHorCursor); }
    else if((width() - pe->position().x()) < FormBorderWidth){ setCursor(Qt::SizeHorCursor); }
    else if(pe->position().y() < FormBorderWidth){ setCursor(Qt::SizeVerCursor); }
    else if((height() - pe->position().y()) < FormBorderWidth){ setCursor(Qt::SizeVerCursor); }
    else setCursor(Qt::OpenHandCursor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////[-------Вспомогательные функции--------]//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Линейная интерполяция
long ScreenshotProcessHighlightedArea::MapValue(long x, long in_min, long in_max, long out_min, long out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Конвертирование координат, исключение отрицательных значений
QRect ScreenshotProcessHighlightedArea::ConvertCoords(QPoint start, QPoint end){

    int x = qMin(start.x(), end.x());
    int y = qMin(start.y(), end.y());

    int width = qAbs(end.x() - start.x());
    int height = qAbs(end.y() - start.y());

    return QRect(x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////[-------Конец--------]/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
