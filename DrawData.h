#ifndef DRAWDATA_H
#define DRAWDATA_H

#include <Includes.h>

enum PAINTING_MODE{
    MODE_NONE,
    MODE_PEN,
    MODE_ERASER,
    MODE_LINE,
    MODE_FIGURE
};

enum FIGURE_TYPE{
    FIGURE_TYPE_RECT,
    FIGURE_TYPE_CIRCLE,
    FIGURE_TYPE_TRIANGLE,
    FIGURE_TYPE_DIAMOND,
    FIGURE_TYPE_STAR,
    FIGURE_TYPE_COUNT
};

enum LINESTYLE_CAP{
    LINESTYLE_CAP_NONE,
    LINESTYLE_CAP_ARROW,
    LINESTYLE_CAP_SPLIT_ARROW,
    LINESTYLE_CAP_CROSS,
    LINESTYLE_CAP_SQUARE,
    LINESTYLE_CAP_CIRCLE,
    LINESTYLE_TYPE_COUNT
};

//////////////////////////////////////////////////////////////////////////
////////////////////////////// DRAWDATA //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct DrawPenData{

public:
    DrawPenData(int width = DEFFAULT_SIZE_PEN, QColor color = Qt::white){
        SetWidth(width);
        SetColor(color);
    }

    // SET
    void SetWidth(int width){ _width = width; }
    void SetColor(QColor color){ _color = color; }

    // GET
    int GetWidth(){ return _width; }
    QColor GetColor(){ return _color; }

private:
    QColor _color = Qt::white;
    int _width = DEFFAULT_SIZE_PEN;
};

struct DrawEraserData{

public:
    DrawEraserData(int width = DEFFAULT_SIZE_ERASER){
        SetWidth(width);
    }

    // SET
    void SetWidth(int width){ _width = width; }

    // GET
    int GetWidth(){ return _width; }

private:
    int _width = DEFFAULT_SIZE_ERASER;
};

struct DrawLineData{

public:
    DrawLineData(int width = DEFFAULT_SIZE_LINE, QColor colorMain = Qt::white, QColor colorSecondary = Qt::white, QPen lineStyle = Qt::NoPen, LINESTYLE_CAP capStartType = LINESTYLE_CAP_NONE, LINESTYLE_CAP capEndType = LINESTYLE_CAP_NONE){
        SetWidth(width);
        SetMainColor(colorMain);
        SetSecondaryColor(colorSecondary);
        SetLineStyle(lineStyle);
        SetLineCapEnd(capEndType);
        SetLineCapStart(capStartType);
    }

    void SetWidth(int width){ _width = width; }
    void SetMainColor(QColor color){ _colorMain = color; }
    void SetSecondaryColor(QColor color){ _colorSecondary = color; }
    void SetLineStyle(QPen lineStyle){ _lineStyle = lineStyle; }
    void SetLineCapEnd(LINESTYLE_CAP capEndType){ _capEndType = capEndType; }
    void SetLineCapStart(LINESTYLE_CAP capStartType){ _capStartType = capStartType; }

    int GetWidth() const { return _width; }
    QColor GetMainColor() const { return _colorMain; }
    QColor GetSecondaryColor() const { return _colorSecondary; }
    QPen GetLineStyle() const { return _lineStyle; }
    LINESTYLE_CAP GetLineCapEnd() const { return _capEndType; }
    LINESTYLE_CAP GetLineCapStart() const { return _capStartType; }

private:
    int _width = DEFFAULT_SIZE_LINE;
    QColor _colorMain = Qt::white, _colorSecondary = Qt::white;
    QPen _lineStyle = Qt::NoPen;
    LINESTYLE_CAP _capEndType = LINESTYLE_CAP_NONE, _capStartType = LINESTYLE_CAP_NONE;
};

struct DrawFigureData{

public:
    DrawFigureData(FIGURE_TYPE figureType = FIGURE_TYPE_RECT, QColor figureMainColor = Qt::white, QColor figureSecondaryColor = Qt::white, int figureBorderSize = DEFFAULT_SIZE_FIGURE_BORDER, QBrush figurePattern = Qt::NoBrush, QPen figureBorderStyle = Qt::NoPen, bool rounding = false){
        SetType(figureType);
        SetPattern(figurePattern);
        SetBorderRounding(rounding);
        SetMainColor(figureMainColor);
        SetBorderSize(figureBorderSize);
        SetBorderStyle(figureBorderStyle);
        SetSecondaryColor(figureSecondaryColor);
    }

    void SetType(FIGURE_TYPE figureType){ _figureType = figureType; }
    void SetPattern(QBrush figurePattern){ _figurePattern = figurePattern; }
    void SetBorderRounding(bool rounding){ _figureBorderRounding = rounding; }
    void SetMainColor(QColor figureMainColor){ _figureMainColor = figureMainColor; }
    void SetBorderSize(int figureBorderSize){ _figureBorderSize = figureBorderSize; }
    void SetBorderStyle(QPen figureBorderStyle){ _figureBorderStyleID = figureBorderStyle; }
    void SetSecondaryColor(QColor figureSecondaryColor){ _figureSecondaryColor = figureSecondaryColor; }

    FIGURE_TYPE GetType(){ return _figureType; }
    QBrush GetPattern(){ return _figurePattern; }
    int GetBorderSize(){ return _figureBorderSize; }
    QColor GetMainColor(){ return _figureMainColor; }
    bool GetBorderRounding(){ return _figureBorderRounding; }
    QPen GetBorderStyle(){ return _figureBorderStyleID; }
    QColor GetSecondaryColor(){ return _figureSecondaryColor; }

private:
    QBrush _figurePattern;
    FIGURE_TYPE _figureType;
    QPen _figureBorderStyleID;
    bool _figureBorderRounding = false;
    int _figureBorderSize = DEFFAULT_SIZE_FIGURE_BORDER;
    QColor _figureMainColor = Qt::white, _figureSecondaryColor = Qt::white;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////// LINEOBJECTCLASS ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

class LineObjectClass : public QWidget{
    Q_OBJECT

public:
    explicit LineObjectClass(QWidget *parent = nullptr){
        _isDisplayed = false;

        if(parent != nullptr){
            setParent(parent);
            resize(parent->size());
        }

        setMouseTracking(true);
        setFocusPolicy(Qt::ClickFocus);

        _isMoving = false;
        _isDraggingPointEnd = false;
        _isDraggingPointStart = false;
        _isDraggingControlPoint1 = false;
        _isDraggingControlPoint2 = false;

        // Плавное мигание маркеров фигуры
        _animationProgress = 0.0;
        _animationSpeed = 0.075; // Скорость смены

        connect(&_timerHightlightLine, &QTimer::timeout, this, &LineObjectClass::changeHightlightColor);
    }

    void VisualUpdate(){ update(); }
    void DrawAreaPaintLine(QPainter *painter){ DrawLine(painter); }

    /// GET
    bool isFocused() const { return _isFocused; }
    bool isDisplayed() const { return _isDisplayed; }
    
    void Show(){
        show();
        _isDisplayed = true;
        _timerHightlightLine.start(10);
    }

    void Hide(){
        hide();
       _isDisplayed = false;
        _timerHightlightLine.stop();
    }

    void SetData(DrawLineData data){
        _lineData = data;

        int avgRed = (_lineData.GetMainColor().red() + _lineData.GetSecondaryColor().red()) / 2;
        int avgGreen = (_lineData.GetMainColor().green() + _lineData.GetSecondaryColor().green()) / 2;
        int avgBlue = (_lineData.GetMainColor().blue() + _lineData.GetSecondaryColor().blue()) / 2;

        _lineHighlightColor.setRgb(255 - avgRed, 255 - avgGreen, 255 - avgBlue);

        update();
    }

    void SetVector(QPoint start, QPoint end){
        _pointVector[0] = start;
        _pointVector[1] = end;

        _controlPoint[0] = _controlPoint[1] = (start + end) / 2;

        UpdateVirtualControlPoints();
    }

signals:
    void LineDeleteRequest();
    void RightClick();

protected:
    void paintEvent(QPaintEvent *paintEvent) override {
        Q_UNUSED(paintEvent)

        QPainter painter(this);
        DrawLine(&painter);

        // Рисуем маркеры изменения размеров
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Рисуем точки начала и конца линии, а так же контрольные
        QBrush linePointBrush(_lineHighlightColor);
        painter.setPen(_lineHighlightColor);
        painter.setBrush(linePointBrush);

        int size = _lineData.GetWidth() * 2;

        painter.drawEllipse(_pointVector[0], size, size);
        painter.drawEllipse(_pointVector[1], size, size);

        painter.drawEllipse(_controlPoint[0], size, size);
        painter.drawEllipse(_controlPoint[1], size, size);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    void focusInEvent(QFocusEvent *event) override{
        QWidget::focusInEvent(event);
        _isFocused = true;
    }

    void focusOutEvent(QFocusEvent *event) override{
        QWidget::focusOutEvent(event);
        _isFocused = false;
    }

    void mousePressEvent(QMouseEvent *event) override {
        QPoint pos = event->pos();

        if(event->buttons() == Qt::LeftButton){
            if((pos - _controlPoint[0]).manhattanLength() < 15)
                _isDraggingControlPoint1 = true;

            else if((pos - _controlPoint[1]).manhattanLength() < 15)
                _isDraggingControlPoint2 = true;

            else if((pos - _pointVector[0]).manhattanLength() < 15)
                _isDraggingPointStart = true;

            else if((pos - _pointVector[1]).manhattanLength() < 15)
                _isDraggingPointEnd = true;

            else if(isPointOnLine(pos))
                _isMoving = true;
            else
                emit LineDeleteRequest();

        }else if(event->buttons() == Qt::RightButton)
            emit RightClick();

        _lastMousePosition = pos;
    }

    void mouseMoveEvent(QMouseEvent *event) override{
        QPoint pos = event->pos();

        if(_isDraggingControlPoint1) _controlPoint[0] = pos;
        else if(_isDraggingControlPoint2) _controlPoint[1] = pos;
        else if(_isDraggingPointStart) _pointVector[0] = pos;
        else if(_isDraggingPointEnd) _pointVector[1] = pos;

        else if(_isMoving){
            QPoint delta = pos - _lastMousePosition;

            _pointVector[0] += delta;
            _pointVector[1] += delta;

            _controlPoint[0] += delta;
            _controlPoint[1] += delta;
        }

        _lastMousePosition = pos;

        UpdateVirtualControlPoints();

        update();
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        Q_UNUSED(event)

        _isMoving = false;
        _isDraggingPointEnd = false;
        _isDraggingPointStart = false;
        _isDraggingControlPoint1 = false;
        _isDraggingControlPoint2 = false;
    }

private slots:
    void changeHightlightColor(){

        _animationProgress += _animationSpeed;

        int alpha = qBound(0, static_cast<int>( ( sin(_animationProgress) + 1.0 ) * 127.5) , 255);

        _lineHighlightColor.setAlpha(alpha);

        update();
    }

private:
    void DrawLine(QPainter *painter) {
        if((_pointVector[1] - _pointVector[0]).manhattanLength() < 3)
            return;

        int size = _lineData.GetWidth() * 2;

        // Устанавливаем кисть и перо
        painter->setBrush(_lineData.GetMainColor());
        painter->setPen(QPen(_lineData.GetMainColor(), size, Qt::SolidLine));

        // Создаём кривую Безье
        QPainterPath drawLine;
        drawLine.moveTo(_pointVector[0]);
        drawLine.cubicTo(_virtualControlPoint[0], _virtualControlPoint[1], _pointVector[1]);

        // Рисуем основную линию
        QPen pen(_lineData.GetLineStyle());
        pen.setWidth(size);
        pen.setColor(_lineData.GetMainColor());
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(drawLine);

        // Направления для начала и конца линии
        QLineF startLine(_pointVector[0], drawLine.pointAtPercent(0.01));
        QLineF endLine(drawLine.pointAtPercent(0.99), _pointVector[1]);

        // Рисуем конци линии
        pen.setStyle(Qt::SolidLine);
        pen.setColor(_lineData.GetSecondaryColor());

        painter->setBrush(_lineData.GetSecondaryColor());
        painter->setPen(pen);

        // Основной угол поворота линии
        float offset = -13;
        double angleStart = std::atan2(-startLine.dy(), startLine.dx());
        double angleEnd = std::atan2(-endLine.dy(), endLine.dx());

        // Рисуем начало линии
        switch(_lineData.GetLineCapStart()){
            case LINESTYLE_CAP_NONE: break;

            case LINESTYLE_CAP_ARROW:{

                QLineF directionLine(startLine.p1(), startLine.p2());
                QPointF adjustedStart = startLine.p1() + (directionLine.unitVector().pointAt(1) - directionLine.pointAt(0)) * offset;

                QPointF arrowP1 = adjustedStart + QPointF(sin(angleStart + M_PI / 3) * size * 3, cos(angleStart + M_PI / 3) * size * 3);
                QPointF arrowP2 = adjustedStart + QPointF(sin(angleStart + M_PI - M_PI / 3) * size * 3, cos(angleStart + M_PI - M_PI / 3) * size * 3);

                painter->drawPolygon(QPolygonF() << adjustedStart << arrowP1 << arrowP2);

                break;
            }

            case LINESTYLE_CAP_SPLIT_ARROW:{
                // Коректируем положение
                angleStart = (angleStart - M_PI_2 + M_PI);

                QPointF adjustedStart = startLine.p1() + QPointF(sin(angleStart) * offset, cos(angleStart) * offset);

                // Угол развёрнут на 90 градусов против часовой стрелки
                QPointF arrowP1 = adjustedStart + QPointF(sin(angleStart + M_PI / 4) * size * 3, cos(angleStart + M_PI / 4) * size * 3);
                QPointF arrowP2 = adjustedStart + QPointF(sin(angleStart - M_PI / 4) * size * 3, cos(angleStart - M_PI / 4) * size * 3);
                QPointF arrowP3 = adjustedStart + QPointF(sin(angleStart) * size, cos(angleStart) * size);

                // Рисуем треугольник с разрезом
                painter->drawPolygon(QPolygonF() << adjustedStart << arrowP1 << arrowP3 << arrowP2);

                break;
            }

            case LINESTYLE_CAP_CROSS:{
                QPointF crossP1 = startLine.p1() + QPointF(sin(angleStart + M_PI / 2) * size, cos(angleStart + M_PI / 2) * size);
                QPointF crossP2 = startLine.p1() + QPointF(sin(angleStart - M_PI / 2) * size, cos(angleStart - M_PI / 2) * size);
                QPointF crossP3 = startLine.p1() + QPointF(sin(angleStart) * size, cos(angleStart) * size);
                QPointF crossP4 = startLine.p1() + QPointF(-sin(angleStart) * size, -cos(angleStart) * size);

                painter->drawLine(crossP1, crossP2);
                painter->drawLine(crossP3, crossP4);

                break;
            }

            case LINESTYLE_CAP_SQUARE:{
                QPointF squareP1 = startLine.p1() + QPointF(sin(angleStart + M_PI / 4) * size, cos(angleStart + M_PI / 4) * size);
                QPointF squareP2 = startLine.p1() + QPointF(sin(angleStart - M_PI / 4) * size, cos(angleStart - M_PI / 4) * size);
                QPointF squareP3 = startLine.p1() + QPointF(sin(angleStart - M_PI + M_PI / 4) * size, cos(angleStart - M_PI + M_PI / 4) * size);
                QPointF squareP4 = startLine.p1() + QPointF(sin(angleStart + M_PI - M_PI / 4) * size, cos(angleStart + M_PI - M_PI / 4) * size);

                painter->drawPolygon(QPolygonF() << squareP1 << squareP2 << squareP3 << squareP4);

                break;
            }

            case LINESTYLE_CAP_CIRCLE:{
                painter->drawEllipse(startLine.p1(), size, size);
                break;
            }

            default: break;
        }

        // Рисуем конец линии
        switch(_lineData.GetLineCapEnd()){
            case LINESTYLE_CAP_NONE: break;

            case LINESTYLE_CAP_ARROW:{
                QLineF directionLine(endLine.p2(), endLine.p1());
                QPointF adjustedEnd = endLine.p2() + (directionLine.unitVector().pointAt(1) - directionLine.pointAt(0)) * offset;

                QPointF arrowP1 = adjustedEnd + QPointF(sin(angleEnd - M_PI / 3) * size * 3, cos(angleEnd - M_PI / 3) * size * 3);
                QPointF arrowP2 = adjustedEnd + QPointF(sin(angleEnd - M_PI + M_PI / 3) * size * 3, cos(angleEnd - M_PI + M_PI / 3) * size * 3);

                painter->drawPolygon(QPolygonF() << adjustedEnd << arrowP1 << arrowP2);

                break;
            }

            case LINESTYLE_CAP_SPLIT_ARROW:{
                angleEnd = (angleEnd - M_PI_2);
                QPointF adjustedEnd = endLine.p2() + QPointF(sin(angleEnd) * offset, cos(angleEnd) * offset);

                // Угол развёрнут на 90 градусов против часовой стрелки
                QPointF arrowP1 = adjustedEnd + QPointF(sin(angleEnd + M_PI / 4) * size * 3, cos(angleEnd + M_PI / 4) * size * 3); // Левая сторона
                QPointF arrowP2 = adjustedEnd + QPointF(sin(angleEnd - M_PI / 4) * size * 3, cos(angleEnd - M_PI / 4) * size * 3); // Правая сторона
                QPointF arrowP3 = adjustedEnd + QPointF(sin(angleEnd) * size, cos(angleEnd) * size);                               // Центральная точка

                // Рисуем треугольник с разрезом
                painter->drawPolygon(QPolygonF() << adjustedEnd << arrowP1 << arrowP3 << arrowP2);

                break;
            }

            case LINESTYLE_CAP_CROSS:{
                QPointF crossP1 = endLine.p2() + QPointF(sin(angleEnd + M_PI / 2) * size, cos(angleEnd + M_PI / 2) * size); // Верх
                QPointF crossP2 = endLine.p2() + QPointF(sin(angleEnd - M_PI / 2) * size, cos(angleEnd - M_PI / 2) * size); // Низ
                QPointF crossP3 = endLine.p2() + QPointF(sin(angleEnd) * size, cos(angleEnd) * size);                      // Вперёд
                QPointF crossP4 = endLine.p2() + QPointF(-sin(angleEnd) * size, -cos(angleEnd) * size);                    // Назад

                painter->drawLine(crossP1, crossP2);
                painter->drawLine(crossP3, crossP4);

                break;
            }

            case LINESTYLE_CAP_SQUARE:{
                QPointF squareP1 = endLine.p2() + QPointF(sin(angleEnd + M_PI / 4) * size, cos(angleEnd + M_PI / 4) * size);
                QPointF squareP2 = endLine.p2() + QPointF(sin(angleEnd - M_PI / 4) * size, cos(angleEnd - M_PI / 4) * size);
                QPointF squareP3 = endLine.p2() + QPointF(sin(angleEnd - M_PI + M_PI / 4) * size, cos(angleEnd - M_PI + M_PI / 4) * size);
                QPointF squareP4 = endLine.p2() + QPointF(sin(angleEnd + M_PI - M_PI / 4) * size, cos(angleEnd + M_PI - M_PI / 4) * size);

                painter->drawPolygon(QPolygonF() << squareP1 << squareP2 << squareP3 << squareP4);

                break;
            }

            case LINESTYLE_CAP_CIRCLE:{
                painter->drawEllipse(endLine.p2(), size, size);
                break;
            }

            default: break;
        }
    }


    void UpdateVirtualControlPoints(){
        _virtualControlPoint[0] = _controlPoint[0] + (_controlPoint[0] - (_pointVector[0] + _pointVector[1]) / 2) * (LINE_BEZIER_INTENSITY - 1);
        _virtualControlPoint[1] = _controlPoint[1] + (_controlPoint[1] - (_pointVector[0] + _pointVector[1]) / 2) * (LINE_BEZIER_INTENSITY - 1);
    }

    bool isPointOnLine(const QPoint& point){
        QPainterPath path;
        path.moveTo(_pointVector[0]);
        path.cubicTo(_virtualControlPoint[0], _virtualControlPoint[1], _pointVector[1]);

        // Устанавливаем толщину линии для проверки
        QPen pen(QBrush(), _lineData.GetWidth() * 2);
        QPainterPathStroker stroker;
        stroker.setWidth(pen.widthF());
        QPainterPath strokedPath = stroker.createStroke(path);

        // Проверяем попадание точки в область линии
        return strokedPath.contains(point);
    }

private:
    DrawLineData _lineData;
    QPoint _pointVector[2], _controlPoint[2], _virtualControlPoint[2], _lastMousePosition;

    bool _isDisplayed = false, _isFocused = false;
    bool _isMoving = false, _isDraggingControlPoint1 = false, _isDraggingControlPoint2 = false, _isDraggingPointStart, _isDraggingPointEnd;

    QColor _lineHighlightColor;
    QTimer _timerHightlightLine;
    qreal _animationSpeed, _animationProgress;
};

//////////////////////////////////////////////////////////////////////////
///////////////////////// FIGUREOBJECTCLASS //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class FigureObjectClass : public QWidget{
    Q_OBJECT

private:
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

    QBrush _figurePatern;
    QRect _originalGeometry;
    QPen _figureBorderStyle;
    QColor _colorMain = Qt::black;
    QColor _colorSecondary = Qt::black;
    int _markerSize = 0, _borderSize = 0;
    QPoint _lastMousePosition, _dragStartPosition;
    bool _isFocused = false, _figureBorderRounding = false, _isDisplayed = false;

    QColor _figureHighlightColor;
    QTimer _timerHightlightFigure;
    qreal _animationSpeed, _animationProgress;

protected:
    virtual void paintFigure(QPainter *painter) = 0;

public:
    virtual FIGURE_TYPE getType() = 0;

signals:
    void RightClick();

public:
    FigureObjectClass(QWidget *parent = nullptr, int markersSize = 7) : QWidget(parent), _modeAction(ACTION_NONE), _colorMain(Qt::black), _colorSecondary(Qt::black){
        setParent(parent);
        SetMarkerSize(markersSize);
        setFocusPolicy(Qt::ClickFocus);
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

        // Плавное мигание маркеров фигуры
        _animationProgress = 0.0;
        _animationSpeed = 0.075; // Скорость смены

        connect(&_timerHightlightFigure, &QTimer::timeout, this, &FigureObjectClass::changeHightlightColor);
    }

    /// SET
    void SetData(DrawFigureData data){
        SetColor(0, data.GetMainColor());
        SetFigurePatern(data.GetPattern());
        SetBorderSize(data.GetBorderSize());
        SetColor(1, data.GetSecondaryColor());
        SetBorderRounding(data.GetBorderRounding());
        SetFigureBorderStyle(data.GetBorderStyle());
    }


    void SetPos(int x, int y){ move(x, y); }
    void SetBorderSize(int size){ _borderSize = size; }
    void SetGeometry(QRect data){ this->setGeometry(data); }
    void SetSize(int width, int height){ resize(width, height); }
    void SetFigurePatern(QBrush patern){ _figurePatern = patern; }
    void SetFigureBorderStyle(QPen style){ _figureBorderStyle = style; }
    void SetBorderRounding(bool rounding){ _figureBorderRounding = rounding; }

    void SetMarkerSize(int size){
        if(size >= 0)
            _markerSize = size;
    }

    void SetColor(int type, QColor color){
        bool flag = false;

        switch(type){
            case 0:{
                if(_colorMain != color){
                    _colorMain = color;
                    flag = true;
                }
                break;
            }

            case 1:{
                if(_colorSecondary != color){
                    _colorSecondary = color;
                    flag = true;
                }
                break;
            }
        }

        if(flag){
            int avgRed = (_colorMain.red() + _colorSecondary.red()) / 2;
            int avgGreen = (_colorMain.green() + _colorSecondary.green()) / 2;
            int avgBlue = (_colorMain.blue() + _colorSecondary.blue()) / 2;

            _figureHighlightColor.setRgb(255 - avgRed, 255 - avgGreen, 255 - avgBlue);

            update();
        }
    }

    /// GET
    QColor GetColor(int type){
        switch(type){
            case 0: return _colorMain;
            case 1: return _colorSecondary;
            default: return Qt::black;
        }
    }

    bool isFocused() const { return _isFocused; }
    QRect getGeometry(){ return this->geometry(); }
    bool isDisplayed() const {return _isDisplayed; }
    int GetBorderSize() const { return _borderSize; }
    int GetMarkerSize() const { return _markerSize; }
    QBrush GetFigurePattern() const { return _figurePatern; }
    bool GetBorderRounding() const { return _figureBorderRounding; }
    QPen GetFigureBorderStyle() const { return _figureBorderStyle; }

    void Show(){
        show();
        _isFocused = true;
        _isDisplayed = true;
        _timerHightlightFigure.start(10);
    }

    void Hide(){
        hide();
        _isFocused = false;
        _isDisplayed = false;
        _timerHightlightFigure.stop();
    }

    void DrawAreaPaintFigure(QPainter *painter){
        painter->save();
        painter->translate(mapToParent(QPoint(0, 0)));

        paintFigure(painter);

        painter->restore();
    }

private slots:
    void changeHightlightColor(){

        _animationProgress += _animationSpeed;

        int alpha = qBound(0, static_cast<int>( ( sin(_animationProgress) + 1.0 ) * 127.5) , 255);

        _figureHighlightColor.setAlpha(alpha);

        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override{
        Q_UNUSED(event);

        QPainter painter(this);

        painter.setPen(QPen(QColor(0, 0, 0, 0), 0));
        painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
        painter.drawRect(0, 0, width(), height());

        paintFigure(&painter);

        // Рисуем маркеры изменения размеров
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(isFocused()){
            painter.setPen(_figureHighlightColor);
            painter.setBrush(_figureHighlightColor);

            painter.drawEllipse(0, 0, GetMarkerSize(), GetMarkerSize());                                                            // Top left marker
            painter.drawEllipse(0, height() - GetMarkerSize(), GetMarkerSize(), GetMarkerSize());                                   // Bottom left marker
            painter.drawEllipse(width() - GetMarkerSize(), 0, GetMarkerSize(), GetMarkerSize());                                    // Top right marker
            painter.drawEllipse(width() - GetMarkerSize(), height() - GetMarkerSize(), GetMarkerSize(), GetMarkerSize());           // Bottom right marker
            painter.drawEllipse(width() / 2 - (GetMarkerSize() / 2), 0, GetMarkerSize(), GetMarkerSize());                            // Top center
            painter.drawEllipse(width() / 2 - (GetMarkerSize() / 2), height() - GetMarkerSize(), GetMarkerSize(), GetMarkerSize());   // Bottom center
            painter.drawEllipse(0, height() / 2 - (GetMarkerSize() / 2), GetMarkerSize(), GetMarkerSize());                           // Left center
            painter.drawEllipse(width() - GetMarkerSize(), height() / 2 - (GetMarkerSize() / 2), GetMarkerSize(), GetMarkerSize());   // Right center
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    void focusInEvent(QFocusEvent *event) override {
        QWidget::focusInEvent(event);
        _isFocused = true;
    }

    void focusOutEvent(QFocusEvent *event) override {
        QWidget::focusOutEvent(event);
        _isFocused = false;
    }

    void mousePressEvent(QMouseEvent *event) override {
        if(event->button() == Qt::LeftButton){
            _lastMousePosition = event->pos();
            _originalGeometry = geometry();
            _dragStartPosition = event->globalPosition().toPoint();

            CheckAction(event->pos());
        }else if(event->button() == Qt::RightButton){
            emit RightClick();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if(_modeAction != ACTION_NONE){
            QRect newGeometry = _originalGeometry;
            QPoint delta = event->globalPosition().toPoint() - _dragStartPosition;

            switch(_modeAction){

            case ACTION_MOVE:
                newGeometry.moveTopLeft(_originalGeometry.topLeft() + delta);
                break;

            case ACTION_RESIZE_HOR_RIGHT:
                newGeometry.setWidth(qMax(_originalGeometry.width() + delta.x(), FigureMinimumWidth));
                break;

            case ACTION_RESIZE_HOR_LEFT:
                newGeometry.setLeft(qMin(_originalGeometry.left() + delta.x(), _originalGeometry.right() - FigureMinimumWidth));
                newGeometry.setWidth(qMax(_originalGeometry.width() - delta.x(), FigureMinimumWidth));
                break;

            case ACTION_RESIZE_VER_UP:
                newGeometry.setTop(qMin(_originalGeometry.top() + delta.y(), _originalGeometry.bottom() - FigureMinimumHeight));
                newGeometry.setHeight(qMax(_originalGeometry.height() - delta.y(), FigureMinimumHeight));
                break;

            case ACTION_RESIZE_VER_DOWN:
                newGeometry.setHeight(qMax(_originalGeometry.height() + delta.y(), FigureMinimumHeight));
                break;

            case ACTION_RESIZE_RIGHT_DOWN:
                newGeometry.setWidth(qMax(_originalGeometry.width() + delta.x(), FigureMinimumWidth));
                newGeometry.setHeight(qMax(_originalGeometry.height() + delta.y(), FigureMinimumHeight));
                break;

            case ACTION_RESIZE_RIGHT_UP:
                newGeometry.setWidth(qMax(_originalGeometry.width() + delta.x(), FigureMinimumWidth));
                newGeometry.setTop(qMin(_originalGeometry.top() + delta.y(), _originalGeometry.bottom() - FigureMinimumHeight));
                newGeometry.setHeight(qMax(_originalGeometry.height() - delta.y(), FigureMinimumHeight));
                break;

            case ACTION_RESIZE_LEFT_DOWN:
                newGeometry.setLeft(qMin(_originalGeometry.left() + delta.x(), _originalGeometry.right() - FigureMinimumWidth));
                newGeometry.setWidth(qMax(_originalGeometry.width() - delta.x(), FigureMinimumWidth));
                newGeometry.setHeight(qMax(_originalGeometry.height() + delta.y(), FigureMinimumHeight));
                break;

            case ACTION_RESIZE_LEFT_UP:
                newGeometry.setLeft(qMin(_originalGeometry.left() + delta.x(), _originalGeometry.right() - FigureMinimumWidth));
                newGeometry.setTop(qMin(_originalGeometry.top() + delta.y(), _originalGeometry.bottom() - FigureMinimumHeight));
                newGeometry.setWidth(qMax(_originalGeometry.width() - delta.x(), FigureMinimumWidth));
                newGeometry.setHeight(qMax(_originalGeometry.height() - delta.y(), FigureMinimumHeight));
                break;

            default:
                break;
            }

            setGeometry(newGeometry);
        }else
            CheckAction(event->pos());
    }

    void mouseReleaseEvent(QMouseEvent *event) override{
        if(event->button() == Qt::LeftButton)
            _modeAction = ACTION_NONE;
    }

    void CheckAction(const QPoint &pos){
        if (pos.x() < FigureBorderWidth && pos.y() < FigureBorderWidth)
            _modeAction = ACTION_RESIZE_LEFT_UP;

        else if (pos.x() < FigureBorderWidth && pos.y() > height() - FigureBorderWidth)
            _modeAction = ACTION_RESIZE_LEFT_DOWN;

        else if (pos.x() > width() - FigureBorderWidth && pos.y() < FigureBorderWidth)
            _modeAction = ACTION_RESIZE_RIGHT_UP;

        else if (pos.x() > width() - FigureBorderWidth && pos.y() > height() - FigureBorderWidth)
            _modeAction = ACTION_RESIZE_RIGHT_DOWN;

        else if (pos.x() < FigureBorderWidth)
            _modeAction = ACTION_RESIZE_HOR_LEFT;

        else if (pos.x() > width() - FigureBorderWidth)
            _modeAction = ACTION_RESIZE_HOR_RIGHT;

        else if (pos.y() < FigureBorderWidth)
            _modeAction = ACTION_RESIZE_VER_UP;

        else if (pos.y() > height() - FigureBorderWidth)
            _modeAction = ACTION_RESIZE_VER_DOWN;

        else
            _modeAction = ACTION_MOVE;
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// ----------- //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class SquareObject : public FigureObjectClass{
    Q_OBJECT

public:
    SquareObject(QWidget *parent = nullptr) : FigureObjectClass(parent){}

protected:
    FIGURE_TYPE getType() override{
        return FIGURE_TYPE_RECT;
    }


    void paintFigure(QPainter *painter) override {

        QBrush brush = GetFigurePattern();
        brush.setColor(GetColor(0));

        painter->fillRect(0 + (GetMarkerSize() / 2) + GetBorderSize() - 1,          // X
                          0 + (GetMarkerSize() / 2) + GetBorderSize() - 1,          // Y
                          width() - GetMarkerSize() - (GetBorderSize() * 2) + 1,    // Width
                          height() - GetMarkerSize() - (GetBorderSize() * 2) + 1,   // Height
                          brush                                                     // Brush
                          );

        QPen borderLinePen(GetFigureBorderStyle());
        borderLinePen.setColor(GetColor(1));
        borderLinePen.setWidth(GetBorderSize());

        if(GetBorderRounding())
            borderLinePen.setCapStyle(Qt::RoundCap);

        painter->setPen(borderLinePen);

        painter->drawRect(0 + (GetMarkerSize() / 2) + (GetBorderSize() / 2),        // X
                          0 + (GetMarkerSize() / 2) + (GetBorderSize() / 2),        // Y
                          width() - GetMarkerSize() - GetBorderSize(),              // Width
                          height() - GetMarkerSize() - GetBorderSize()              // Height
                          );
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// ----------- //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CircleObject : public FigureObjectClass{
    Q_OBJECT

public:
    CircleObject(QWidget *parent = nullptr) : FigureObjectClass(parent){}

protected:
    FIGURE_TYPE getType() override{
        return FIGURE_TYPE_CIRCLE;
    }

    void paintFigure(QPainter *painter) override {

        QPen borderLinePen(GetFigureBorderStyle());
        borderLinePen.setColor(GetColor(1));
        borderLinePen.setWidth(GetBorderSize());

        if(GetBorderRounding())
            borderLinePen.setCapStyle(Qt::RoundCap);

        painter->setPen(borderLinePen);

        // Рисуем границу круга
        painter->drawEllipse(
                            GetMarkerSize() / 2 + GetBorderSize() / 2,           // X
                            GetMarkerSize() / 2 + GetBorderSize() / 2,           // Y
                            width() - GetMarkerSize() - GetBorderSize(),         // Width
                            height() - GetMarkerSize() - GetBorderSize()         // Height
                            );

        QBrush brush = GetFigurePattern();
        brush.setColor(GetColor(0));

        // Рисуем внутренний патерн круга
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(
                            GetMarkerSize() / 2 + GetBorderSize(),               // X
                            GetMarkerSize() / 2 + GetBorderSize(),               // Y
                            width() - GetMarkerSize() - (GetBorderSize() * 2),   // Width
                            height() - GetMarkerSize() - (GetBorderSize() * 2)   // Height
                            );
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// ----------- //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class TriangleObject : public FigureObjectClass {
    Q_OBJECT

public:
    TriangleObject(QWidget *parent = nullptr) : FigureObjectClass(parent) {}

protected:
    FIGURE_TYPE getType() override{
        return FIGURE_TYPE_TRIANGLE;
    }

    void paintFigure(QPainter *painter) override {
        int margin = GetMarkerSize() / 2 + GetBorderSize();
        int borderOffset = GetBorderSize() / 2;

        // Внешний треугольник
        QPolygon outerTriangle;
        outerTriangle << QPoint(width() / 2, margin);                 // Верхняя точка
        outerTriangle << QPoint(margin, height() - margin);           // Левая точка
        outerTriangle << QPoint(width() - margin, height() - margin); // Правая точка

        // Внутренний треугольник
        QPolygon innerTriangle;
        innerTriangle << QPoint(width() / 2, margin + borderOffset);                                // Верхняя точка
        innerTriangle << QPoint(margin + borderOffset, height() - margin - borderOffset);           // Левая точка
        innerTriangle << QPoint(width() - margin - borderOffset, height() - margin - borderOffset); // Правая точка

        // Отрисовка внутреннего паттерна
        QBrush brush = GetFigurePattern();
        brush.setColor(GetColor(0));
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(innerTriangle);

        // Отрисовка границы треугольника
        QPen borderLinePen(GetFigureBorderStyle());
        borderLinePen.setColor(GetColor(1));
        borderLinePen.setWidth(GetBorderSize());

        if(GetBorderRounding())
            borderLinePen.setCapStyle(Qt::RoundCap);

        painter->setPen(borderLinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(outerTriangle);
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// ----------- //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class DiamondObject : public FigureObjectClass{
    Q_OBJECT

public:
    DiamondObject(QWidget *parent = nullptr) : FigureObjectClass(parent){}

protected:
    FIGURE_TYPE getType() override{
        return FIGURE_TYPE_DIAMOND;
    }

    void paintFigure(QPainter *painter) override {
        int margin = GetMarkerSize() / 2 + GetBorderSize();
        int borderOffset = GetBorderSize() / 2;

        // Внешний ромб
        QPolygon outerRhombus;
        outerRhombus << QPoint(width() / 2, margin);            // Верхняя вершина
        outerRhombus << QPoint(width() - margin, height() / 2); // Правая вершина
        outerRhombus << QPoint(width() / 2, height() - margin); // Нижняя вершина
        outerRhombus << QPoint(margin, height() / 2);           // Левая вершина

        // Внутренний ромб
        QPolygon innerRhombus;
        innerRhombus << QPoint(width() / 2, margin + borderOffset);            // Верхняя вершина
        innerRhombus << QPoint(width() - margin - borderOffset, height() / 2); // Правая вершина
        innerRhombus << QPoint(width() / 2, height() - margin - borderOffset); // Нижняя вершина
        innerRhombus << QPoint(margin + borderOffset, height() / 2);           // Левая вершина

        QBrush brush = GetFigurePattern();
        brush.setColor(GetColor(0));
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(innerRhombus);

        QPen borderLinePen(GetFigureBorderStyle());
        borderLinePen.setColor(GetColor(1));
        borderLinePen.setWidth(GetBorderSize());

        if(GetBorderRounding())
            borderLinePen.setCapStyle(Qt::RoundCap);

        painter->setPen(borderLinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(outerRhombus);
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// ----------- //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class StarObject : public FigureObjectClass {
    Q_OBJECT

public:
    StarObject(QWidget *parent = nullptr) : FigureObjectClass(parent) {}

protected:
    FIGURE_TYPE getType() override{
        return FIGURE_TYPE_STAR;
    }

    void paintFigure(QPainter *painter) override {
        int margin = GetMarkerSize() / 2 + GetBorderSize();
        int borderOffset = GetBorderSize() / 2;

        // Параметры звезды
        int numPoints = 5; // Количество лучей у звезды
        int availableWidth = width() - margin * 2;
        int availableHeight = height() - margin * 2;

        double outerRadius = qMin(availableWidth, availableHeight) / 2;
        double innerRadius = outerRadius / 2.0;

        // Центр звезды
        QPoint center(width() / 2, height() / 2);

        QPolygon outerStar = createStar(center, outerRadius, innerRadius, numPoints);
        QPolygon innerStar = createStar(center, outerRadius - borderOffset, innerRadius - borderOffset, numPoints);

        QBrush brush = GetFigurePattern();
        brush.setColor(GetColor(0));
        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(innerStar);

        QPen borderLinePen(GetFigureBorderStyle());
        borderLinePen.setColor(GetColor(1));
        borderLinePen.setWidth(GetBorderSize());

        if(GetBorderRounding())
            borderLinePen.setCapStyle(Qt::RoundCap);

        painter->setPen(borderLinePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPolygon(outerStar);
    }

private:
    QPolygon createStar(const QPoint &center, double outerRadius, double innerRadius, int numPoints){
        QPolygon star;
        double angleStep = 2 * M_PI / numPoints;

        for(int i = 0; i < 2 * numPoints; ++i){
            double angle = i * angleStep / 2;
            double radius = (i % 2 == 0) ? outerRadius : innerRadius;
            int x = center.x() + static_cast<int>(radius * cos(angle - M_PI / 2));
            int y = center.y() + static_cast<int>(radius * sin(angle - M_PI / 2));

            star << QPoint(x, y);
        }

        return star;
    }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////// END //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // DRAWDATA_H
