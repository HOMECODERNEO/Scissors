#include "screenshotprocess.h"

ScreenshotProcess::ScreenshotProcess(QWidget *parent): QWidget(parent){

    _screenshotHighlightArea = new ScreenshotProcessHighlightedArea(this);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    connect(this, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));
    connect(this, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(this, SIGNAL(ScreenshotProcessEnd(QPixmap)), parent, SLOT(ScreenshotProcessEnd(QPixmap)));
    connect(_screenshotHighlightArea, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));

    setGeometry(emit GetActiveScreen()->geometry());
    setFocus();
}

void ScreenshotProcess::Show(){
    show();
    _animationManager.Create_WindowOpacity(this, [this](){ repaint(); }, 100, 0, 1).Start();
}

void ScreenshotProcess::Hide(){
    _animationManager.Create_WindowOpacity(this, [this](){
                                                            _flagStopFrame = false;
                                                            ClearBorderLines();
                                                            _screenshotHighlightArea->Hide();
                                                            hide();
                                                        }, 100, 1, 0).Start();
}

void ScreenshotProcess::SetStopFrameImage(QPixmap image){
    _bufferStopFrame = image;
    _flagStopFrame = true;
    repaint();
}

void ScreenshotProcess::CreateScreenshot(QRect rect){
    QRect convert = ConvertGlobalCoords(QPoint(rect.x(), rect.y()), QPoint(rect.x() + rect.width(), rect.y() + rect.height()));

    _animationManager.Create_WindowOpacity(this, [this, convert](){
                                                                    ClearBorderLines();
                                                                    _screenshotHighlightArea->Hide();
                                                                    hide();

                                                                    if(emit GetProgramSettings().Get_StopFrame())
                                                                        emit ScreenshotProcessEnd(_bufferStopFrame.copy(convert));
                                                                    else
                                                                        emit ScreenshotProcessEnd(emit GetActiveScreen()->grabWindow(0, convert.x(), convert.y(), convert.width(), convert.height()));
                                                                }, 100, 1, 0).Start();
}

void ScreenshotProcess::AreaMove(QRect rect){
    int newW = rect.x() + rect.width();
    int newH = rect.y() + rect.height();

    ///////////////////////////////////////////////////////////
    _lineTop.setRect(rect.x(), rect.y(), newW, rect.y());    // TOP
    _lineDown.setRect(rect.x(), rect.y(), rect.x(), newH);   // LEFT
    _lineLeft.setRect(newW, rect.y(), newW, newH);           // RIGHT
    _lineRight.setRect(rect.x(), newH, newW, newH);          // DOWN
    ///////////////////////////////////////////////////////////

    repaint();
}

void ScreenshotProcess::paintEvent(QPaintEvent *){
    QPainter paint(this);

    if(emit GetProgramSettings().Get_StopFrame() && _flagStopFrame)
        paint.drawPixmap(QRect(0, 0, _bufferStopFrame.width(), _bufferStopFrame.height()), _bufferStopFrame);

    paint.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 50)));
    paint.drawRect(0, 0, width(), height());

    paint.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::SquareCap));
    paint.drawLine(_lineTop.x(), _lineTop.y(), _lineTop.width(), _lineTop.height()); // TOP
    paint.drawLine(_lineDown.x(), _lineDown.y(), _lineDown.width(), _lineDown.height()); // DOWN
    paint.drawLine(_lineLeft.x(), _lineLeft.y(), _lineLeft.width(), _lineLeft.height()); // LEFT
    paint.drawLine(_lineRight.x(), _lineRight.y(), _lineRight.width(), _lineRight.height()); // RIGHT
}

void ScreenshotProcess::mousePressEvent(QMouseEvent *event){
    if(!_mouseLeftPressed && event->buttons() == Qt::LeftButton){
        ClearBorderLines();
        _screenshotHighlightArea->Hide();

        _mouseLeftPressed = true;
        _mousePressGlobalPositionStart = event->globalPos();
    }
}

void ScreenshotProcess::mouseMoveEvent(QMouseEvent *event){
    if(_mouseLeftPressed){
        float x = event->globalPos().x();
        float y = event->globalPos().y();

        _lineTop.setRect(_mousePressGlobalPositionStart.x(), _mousePressGlobalPositionStart.y(), x, _mousePressGlobalPositionStart.y());
        _lineDown.setRect(_mousePressGlobalPositionStart.x(), y, x, y);
        _lineLeft.setRect(_mousePressGlobalPositionStart.x(), _mousePressGlobalPositionStart.y(), _mousePressGlobalPositionStart.x(), y);
        _lineRight.setRect(x, _mousePressGlobalPositionStart.y(), x, y);

        repaint();
    }
}

void ScreenshotProcess::mouseReleaseEvent(QMouseEvent *event){    
    if(_mouseLeftPressed){
        QRect convertArea = ConvertGlobalCoords(_mousePressGlobalPositionStart, event->globalPos());

        if(emit GetProgramSettings().Get_ShowModificationArea())
            _screenshotHighlightArea->Show(convertArea);
        else
            CreateScreenshot(convertArea);
    }

    _mouseLeftPressed = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcess::ClearBorderLines(){
    _lineTop.setRect(0, 0, 0, 0);
    _lineDown.setRect(0, 0, 0, 0);
    _lineLeft.setRect(0, 0, 0, 0);
    _lineRight.setRect(0, 0, 0, 0);

    repaint();
}

QRect ScreenshotProcess::ConvertGlobalCoords(QPoint start, QPoint end){
    float x1, y1;
    float x2, y2;

    if(end.x() > start.x()){
        x1 = start.x();
        x2 = end.x() - start.x();
    }else{
        x1 = end.x();
        x2 = start.x() - end.x();
    }

    if(end.y() > start.y()){
        y1 = start.y();
        y2 = end.y() - start.y();
    }else{
        y1 = end.y();
        y2 = start.y() - end.y();
    }

    return QRect(x1, y1, x2, y2);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
