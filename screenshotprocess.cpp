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

    _animationManager.Create_WindowOpacity(this, [this, convert]() {
                            ClearBorderLines();
                            _screenshotHighlightArea->Hide();
                            hide();

                            if(GetProgramSettings().Get_StopFrame()){
                                qreal scaleX = static_cast<qreal>(width()) / _bufferStopFrame.width();
                                qreal scaleY = static_cast<qreal>(height()) / _bufferStopFrame.height();

                                emit ScreenshotProcessEnd(_bufferStopFrame.copy(QRect(convert.x() / scaleX, convert.y() / scaleY, convert.width() / scaleX, convert.height() / scaleY)));
                            }else
                                emit ScreenshotProcessEnd(GetActiveScreen()->grabWindow(0, convert.x(), convert.y(), convert.width(), convert.height()));
                         }, 100, 1, 0).Start();
}

void ScreenshotProcess::AreaMove(QRect rect){
    _hightlightAreaGeometry = rect;
    repaint();
}

void ScreenshotProcess::paintEvent(QPaintEvent *){
    QPainter paint(this);

    if (GetProgramSettings().Get_StopFrame() && _flagStopFrame) {
        // Explicitly set the transformation matrix to handle scaling
        QTransform transform;
        transform.scale(static_cast<qreal>(width()) / _bufferStopFrame.width(),
                        static_cast<qreal>(height()) / _bufferStopFrame.height());
        paint.setTransform(transform);

        paint.drawPixmap(0, 0, _bufferStopFrame);
    }

    // Reset the transformation matrix for subsequent drawing
    paint.resetTransform();

    paint.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 50)));
    paint.drawRect(0, 0, width(), height());

    paint.setPen(QPen(Qt::red, 1, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 0)));
    paint.drawRect(_hightlightAreaGeometry);
}

void ScreenshotProcess::mousePressEvent(QMouseEvent *event){
    if(!_mouseLeftPressed && event->buttons() == Qt::LeftButton){
        ClearBorderLines();
        _screenshotHighlightArea->Hide();

        _mouseLeftPressed = true;
        _mousePressGlobalPositionStart = event->globalPosition();
    }
}

void ScreenshotProcess::mouseMoveEvent(QMouseEvent *event){
    if(_mouseLeftPressed){
        _hightlightAreaGeometry.setX(_mousePressGlobalPositionStart.x());
        _hightlightAreaGeometry.setY(_mousePressGlobalPositionStart.y());
        _hightlightAreaGeometry.setWidth(event->globalPosition().x() - _hightlightAreaGeometry.x());
        _hightlightAreaGeometry.setHeight(event->globalPosition().y() - _hightlightAreaGeometry.y());

        repaint();
    }
}

void ScreenshotProcess::mouseReleaseEvent(QMouseEvent *event){    
    if(_mouseLeftPressed){
        QRect convertArea = ConvertGlobalCoords(_mousePressGlobalPositionStart.toPoint(), event->globalPosition().toPoint());

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
    _hightlightAreaGeometry = QRect();
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
