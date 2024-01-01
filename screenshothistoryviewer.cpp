#include "screenshothistoryviewer.h"

ScreenshotHistoryViewer::ScreenshotHistoryViewer(QWidget *parent): QWidget{parent}{
    setMouseTracking(true);

    setCursor(Qt::OpenHandCursor);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
}

bool ScreenshotHistoryViewer::IsVisible(){
    return _isVisible;
}

void ScreenshotHistoryViewer::Show(QPixmap image){
    _isVisible = true;
    _currentImage = image;

    QScreen *screen = emit GetActiveScreen();

    if(screen != nullptr)
        setGeometry((screen->geometry().width() / 2) - (_currentImage.width() / 2), (screen->geometry().height() / 2) - (_currentImage.height() / 2), _currentImage.width(), _currentImage.height());

    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

void ScreenshotHistoryViewer::Hide(){
    _isVisible = false;

    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();
}

void ScreenshotHistoryViewer::mousePressEvent(QMouseEvent *pe){
    if(pe->buttons() == Qt::LeftButton){
        _isLeftClick = true;
        mousePressPosition = pe->pos();

        setCursor(Qt::ClosedHandCursor);
    }
}

void ScreenshotHistoryViewer::mouseMoveEvent(QMouseEvent *pe){
    if(_isLeftClick)
        move(QRect(pe->globalPos() - mousePressPosition, geometry().size()).topLeft());
}

void ScreenshotHistoryViewer::mouseReleaseEvent(QMouseEvent *){
    _isLeftClick = false;

    setCursor(Qt::OpenHandCursor);

    repaint();
}

void ScreenshotHistoryViewer::wheelEvent(QWheelEvent *event){
    // Получаем текущие размеры окна
    QRect oldGeometry = this->geometry();

    // Ограничиваем масштаб, чтобы окно не стало слишком маленьким или слишком большим
    double scaleChange;

    if (event->angleDelta().y() > 0) {
        scaleChange = 1.1; // Увеличиваем масштаб на 10%
    } else {
        scaleChange = 0.9; // Уменьшаем масштаб на 10%
    }

    // Вычисляем новые размеры окна с учетом текущего масштаба
    int newWidth = static_cast<int>(oldGeometry.width() * scaleChange);
    int newHeight = static_cast<int>(oldGeometry.height() * scaleChange);

    // Пересчитываем положение окна относительно курсора
    QPointF cursorPos = event->position();
    int xOffset = cursorPos.x() - cursorPos.x() * scaleChange;
    int yOffset = cursorPos.y() - cursorPos.y() * scaleChange;

    // Подгоняем новый размер что бы изображение не поплыло
    QRect newGeometry = QRect(this->x() + xOffset, this->y() + yOffset, newWidth, newHeight);
    QSize size = _currentImage.scaled(newGeometry.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
    newGeometry.setHeight(size.height());
    newGeometry.setWidth(size.width());

    // Анимация изменения масштаба
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(150);
    animation->setStartValue(oldGeometry);
    animation->setEndValue(newGeometry);
    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void ScreenshotHistoryViewer::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setRenderHint(QPainter::Antialiasing);
    paint.setRenderHint(QPainter::SmoothPixmapTransform);

    paint.drawPixmap(QRect(0, 0, width(), height()), _currentImage);
    paint.end();
}
