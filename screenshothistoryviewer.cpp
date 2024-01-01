#include "screenshothistoryviewer.h"

// Конструктор
ScreenshotHistoryViewer::ScreenshotHistoryViewer(QWidget *parent): QGraphicsView{parent}{
    setRenderHint(QPainter::Antialiasing, false);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setMouseTracking(true);

    _scene = new QGraphicsScene(this);
    _item = new QGraphicsPixmapItem();

    _scene->addItem(_item);
    setScene(_scene);

    setStyleSheet("background-color: rgba(1, 1, 1, 200);");
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
}

// Видимое ли окно
bool ScreenshotHistoryViewer::IsVisible(){
    return _isVisible;
}

// Отображаем просмотрщик
void ScreenshotHistoryViewer::Show(QPixmap image){
    // Изменяем флаг отображения и сбрасываем предыдущий масштаб
    _isVisible = true;
    _oldZoomLevel = 0;
    _currentZoomLevel = 0;

    // Устанавливаем изображение
    _item->setPixmap(image);
    _scene->setSceneRect(_item->boundingRect());

    // Центруем изображение
    centerOn(mapFromScene(_scene->sceneRect().center().x(), _scene->sceneRect().center().y()));

    // Устанавливаем размер окна на весь экран
    QScreen *size = emit GetActiveScreen();
    setGeometry(0, 0, size->geometry().width(), size->geometry().height());

    // Отображаем
    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

// Прячем просмотрщика
void ScreenshotHistoryViewer::Hide(){
    _isVisible = false;

    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();
}

// Событие прокрутки колесика мыши
void ScreenshotHistoryViewer::wheelEvent(QWheelEvent *event){
    const auto d = event->angleDelta();

    if (event->modifiers() == Qt::NoModifier) {
        auto dm = abs(d.x()) > abs(d.y()) ? d.x() : d.y();

        if (dm > 0 && _currentZoomLevel + 4.7 < 100)
            Zoom(4.7);
        else if (dm < 0 && _currentZoomLevel - 4.7 > -30)
            Zoom(-4.7);

        event->accept();
    }
}

// Зумим
void ScreenshotHistoryViewer::Zoom(float level){
    _currentZoomLevel += level;
    SetMatrix();
}

// Обновляем масштаб изображения
void ScreenshotHistoryViewer::UpdateZoom(){
    qreal newScale = std::pow(2.0, _oldZoomLevel / 10.0);

    QTransform mat;
    mat.scale(newScale, newScale);
    mat.rotateRadians(RotationRadians());
    setTransform(mat);
}

void ScreenshotHistoryViewer::SetMatrix(){

    QVariantAnimation *animation = new QVariantAnimation;
    animation->setStartValue(_oldZoomLevel);
    animation->setEndValue(_currentZoomLevel);
    animation->setDuration(175);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(animation, &QVariantAnimation::valueChanged, this, [=](const QVariant &value) {
        _oldZoomLevel = value.toInt();

        UpdateZoom();
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal ScreenshotHistoryViewer::RotationRadians() const{
    QPointF p = transform().map(QPointF(1., 0.));
    return std::atan2(p.y(), p.x());
}
