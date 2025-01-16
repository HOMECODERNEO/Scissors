#include "screenshotprocess.h"

ScreenshotProcess::ScreenshotProcess(QWidget *parent): QWidget(parent){

    _screenshotProcessTools = new ScreenshotProcessToolPopup(this);
    _screenshotHighlightArea = new ScreenshotProcessHighlightedArea(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    connect(_screenshotHighlightArea, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(_screenshotHighlightArea, SIGNAL(GetCurrentScreenGeometry()), parent, SLOT(GetCurrentScreenGeometry()));
    connect(_screenshotHighlightArea, &ScreenshotProcessHighlightedArea::AreaMove, this, &ScreenshotProcess::AreaMove);
    connect(_screenshotHighlightArea, &ScreenshotProcessHighlightedArea::CreateScreenshot, this, &ScreenshotProcess::CreateScreenshot);

    connect(_screenshotProcessTools, SIGNAL(GetCurrentScreenGeometry()), parent, SLOT(GetCurrentScreenGeometry()));
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::GetPenData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::GetPenData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::GetLineData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::GetLineData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::SetDrawMode, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::SetDrawMode);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::GetDrawMode, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::GetDrawMode);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::ClearDrawArea, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::ClearDrawArea);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::GetEraserData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::GetEraserData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::GetFigureData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::GetFigureData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::Update_PenData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::UpdateToolPanel_PenData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::Update_LineData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::UpdateToolPanel_LineData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::Update_EraserData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::UpdateToolPanel_EraserData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::Update_FigureData, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::UpdateToolPanel_FigureData);
    connect(_screenshotProcessTools, &ScreenshotProcessToolPopup::LineCapInit, _screenshotHighlightArea, &ScreenshotProcessHighlightedArea::LineCapInit);

    _screenshotHighlightArea->Hide();
    setFocus();
}

ScreenshotProcessToolPopup* ScreenshotProcess::GetToolPopup() const { return _screenshotProcessTools; }
ScreenshotProcessHighlightedArea* ScreenshotProcess::GetHighlightedArea() const { return _screenshotHighlightArea; }

void ScreenshotProcess::Show(){
    emit ProcessEvent_ShowStart();

    setGeometry(emit GetCurrentScreenGeometry());

    show();
    _animationManager.Create_WindowOpacity(this, [this](){

        repaint();
        emit ProcessEvent_ShowEnd(this);
    }, 100, 0, 1).Start();

    _screenshotProcessTools->Show();

    activateWindow();
}

bool ScreenshotProcess::Hide(){
    emit ProcessEvent_HideStart();

    _screenshotProcessTools->Hide();

    _animationManager.Create_WindowOpacity(this, [this](){
                                                            _flagStopFrame = false;
                                                            ClearBorderLines();

                                                            _screenshotHighlightArea->Hide();
                                                            hide();

                                                            emit ProcessEvent_HideEnd();
                                                        }, 100, 1, 0).Start();

    return true;
}

void ScreenshotProcess::SetStopFrameImage(QPixmap image){
    _bufferStopFrame = image;
    _flagStopFrame = true;
    repaint();
}

void ScreenshotProcess::CreateScreenshot(QRect rect, QPixmap draw){
    QRect convert = ConvertGlobalCoords(QPoint(rect.x(), rect.y()), QPoint(rect.x() + rect.width(), rect.y() + rect.height()));

    _screenshotProcessTools->Hide();
    _screenshotHighlightArea->Hide();

    _animationManager.Create_WindowOpacity(this, [this, convert, draw](){
        ClearBorderLines();

        hide();
        emit ProcessEvent_HideEnd();

        QPixmap screenshot;
        QPixmap drawBuffer = draw;

        if(GetProgramSettings().Get_StopFrame()){
            qreal scaleX = static_cast<qreal>(width()) / _bufferStopFrame.width();
            qreal scaleY = static_cast<qreal>(height()) / _bufferStopFrame.height();

            QRect rectBuffer = QRect(convert.x() / scaleX, convert.y() / scaleY, convert.width() / scaleX, convert.height() / scaleY);

            screenshot = _bufferStopFrame.copy(rectBuffer);
            drawBuffer = drawBuffer.scaled(rectBuffer.size());
        }else
            screenshot = QGuiApplication::primaryScreen()->grabWindow(0, convert.x(), convert.y(), convert.width(), convert.height());

        // Смешиваем скриншот с полем рисования
        if(!drawBuffer.isNull()){
            QPainter painter(&screenshot);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawPixmap(0, 0, drawBuffer);
        }

        bool history_hide = _screenshotProcessTools->GetScreenshotDisplay();
        emit ScreenshotProcess_CreateScreenshot(screenshot, history_hide);

        // Проигрываем звук создания скриншота
        emit PlaySound(SOUND_TYPE_PRINTSCREEN);

    }, 100, 1, 0).Start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcess::PenDashLinesInit(QMap<QString, QVector<QVector<qreal>>> blockPatterns){
    _screenshotProcessTools->PenDashLinesInit(blockPatterns);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcess::AreaMove(QRect rect){
    _hightlightAreaGeometry = rect;
    repaint();
}

void ScreenshotProcess::paintEvent(QPaintEvent *){
    QPainter paint(this);

    if (GetProgramSettings().Get_StopFrame() && _flagStopFrame){
        QTransform transform;

        transform.scale(static_cast<qreal>(width()) / _bufferStopFrame.width(),
                        static_cast<qreal>(height()) / _bufferStopFrame.height());

        paint.setTransform(transform);

        paint.drawPixmap(0, 0, _bufferStopFrame);
    }

    paint.resetTransform();

    paint.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 50)));
    paint.drawRect(0, 0, width(), height());

    paint.setPen(QPen(Qt::red, 2, Qt::DashLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 0)));
    paint.drawRect(_hightlightAreaGeometry);
}

void ScreenshotProcess::mousePressEvent(QMouseEvent *event){
    if(!_mouseLeftPressed && event->buttons() == Qt::LeftButton){

        if(_screenshotHighlightArea->GetDrawMode() == MODE_NONE){
            ClearBorderLines();
            _screenshotHighlightArea->Hide();
        }

        _mouseLeftPressed = true;
        _mousePressGlobalPositionStart = event->globalPosition();
    }
}

void ScreenshotProcess::mouseMoveEvent(QMouseEvent *event){
    if(_mouseLeftPressed && _screenshotHighlightArea->GetDrawMode() == MODE_NONE){
        _hightlightAreaGeometry.setX(_mousePressGlobalPositionStart.x());
        _hightlightAreaGeometry.setY(_mousePressGlobalPositionStart.y());
        _hightlightAreaGeometry.setWidth(event->globalPosition().x() - _hightlightAreaGeometry.x());
        _hightlightAreaGeometry.setHeight(event->globalPosition().y() - _hightlightAreaGeometry.y());

        repaint();
    }
}

void ScreenshotProcess::mouseReleaseEvent(QMouseEvent *event){
    if(_mouseLeftPressed && _screenshotHighlightArea->GetDrawMode() == MODE_NONE){
        QRect convertArea = ConvertGlobalCoords(_mousePressGlobalPositionStart.toPoint(), event->globalPosition().toPoint());

        // Устанавливаем минимальный допустимый размер
        QRect minSize = convertArea;
        minSize.setWidth(FormMinimumWidth);
        minSize.setHeight(FormMinimumHeight);

        // Если созданная область больше или равно минимальной - то создаем выделение
        if((convertArea & minSize) == minSize){
            if((emit GetProgramSettings()).Get_ShowModificationArea()){
                _screenshotHighlightArea->Show(convertArea);
            }else
                CreateScreenshot(convertArea, QPixmap());
        }else
            ClearBorderLines();
    }

    _mouseLeftPressed = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcess::ClearFigure(){
    _screenshotHighlightArea->DeleteFigure(nullptr, false);
    _screenshotHighlightArea->DeleteLine(nullptr, false);
}

void ScreenshotProcess::ClearBorderLines(){
    _hightlightAreaGeometry = QRect(0, 0, 0, 0);
    repaint();
}

QRect ScreenshotProcess::ConvertGlobalCoords(QPoint start, QPoint end){

    int x = qMin(start.x(), end.x());
    int y = qMin(start.y(), end.y());

    int width = qAbs(end.x() - start.x());
    int height = qAbs(end.y() - start.y());

    return QRect(x, y, width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
