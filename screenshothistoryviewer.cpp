#include "screenshothistoryviewer.h"

// Конструктор
ScreenshotHistoryViewer::ScreenshotHistoryViewer(QWidget *parent): QGraphicsView{parent}{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
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

    _percent = new Percent(this);
    _zoomTimer = new QTimer(this);
}

// Видимое ли окно
bool ScreenshotHistoryViewer::IsVisible(){
    return _isVisible;
}

// Отображаем просмотрщик
void ScreenshotHistoryViewer::Show(QPixmap image, ProgramSetting settings){
    // Изменяем флаг отображения
    _isVisible = true;
    _percent->SetActive(settings.Get_ViewerShowPercent());

    _item->setPixmap(image);
    _scene->setSceneRect(_item->boundingRect());

    // Центруем изображение
    centerOn(mapFromScene(_scene->sceneRect().center().x(), _scene->sceneRect().center().y()));

    // Устанавливаем размер окна на весь экран
    QScreen *size = emit GetActiveScreen();
    setGeometry(0, 0, size->geometry().width(), size->geometry().height());
    _percent->setGeometry((geometry().width() / 2) - 50, (geometry().height() / 2) - 50, 100, 100);

    // Сбрасываем предыдущий масштаб
    _currentZoomLevel = 0;
    _oldZoomLevel = 0;
    UpdateZoom();

    // Отображаем
    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

// Прячем просмотрщика
void ScreenshotHistoryViewer::Hide(){
    _isVisible = false;

    _percent->hide();
    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();
}

// Событие прокрутки колесика мыши
void ScreenshotHistoryViewer::wheelEvent(QWheelEvent *event){
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QPoint delta = event->angleDelta();
    _lastCursorPosition = mapToScene(event->position().toPoint());

    if (event->modifiers() == Qt::NoModifier){
        int result = abs(delta.x()) > abs(delta.y()) ? delta.x() : delta.y();

        if(result > 0)
            Zoom(_currentZoomLevel + 1.5);
        else if (result < 0)
            Zoom(_currentZoomLevel - 1.5);

        event->accept();
    }
}

// Зумим
void ScreenshotHistoryViewer::Zoom(float level){
    if(_zoomTimer){
        if(_zoomTimer->isActive()){
            _zoomTimer->stop();
            _zoomTimer->deleteLater();
            disconnect(_zoomTimer, &QTimer::timeout, this, &ScreenshotHistoryViewer::ZoomAnimationStep);
            _zoomTimer = nullptr;
        }
    }

    if(!_zoomTimer){
        _zoomTimer = new QTimer(this);
        connect(_zoomTimer, &QTimer::timeout, this, &ScreenshotHistoryViewer::ZoomAnimationStep);
    }

    _currentZoomLevel = level;

    // Необходимое количество шагов
    _zoomStepSize = (_currentZoomLevel - _oldZoomLevel) / 50;

    _zoomTimer->start(1);
}

// Обновляем масштаб изображения
void ScreenshotHistoryViewer::UpdateZoom(){
    qreal newScale = std::pow(2.0, _oldZoomLevel / 10.0);

    QTransform mat;
    mat.scale(newScale, newScale);
    mat.translate(_lastCursorPosition.x() * (1 - newScale), _lastCursorPosition.y() * (1 - newScale));
    setTransform(mat);
}

void ScreenshotHistoryViewer::ZoomAnimationStep(){
    // Удаляем если достигли необходимого значения
    if (FloatCompare(_oldZoomLevel, _currentZoomLevel) || (_oldZoomLevel + _zoomStepSize) >= 100 || (_oldZoomLevel + _zoomStepSize <= -30)){
        _zoomTimer->stop();
        disconnect(_zoomTimer, &QTimer::timeout, this, &ScreenshotHistoryViewer::ZoomAnimationStep);
        _zoomTimer->deleteLater();
        _zoomTimer = nullptr;

        _currentZoomLevel = _oldZoomLevel;
        return;
    }

    // Пошагово обновляем значение
    _oldZoomLevel += _zoomStepSize;
    _percent->setValue(_oldZoomLevel);
    UpdateZoom();
}

// Коректно сравниваем 2 float2 числа
bool ScreenshotHistoryViewer::FloatCompare(float f1, float f2) const{
    static constexpr auto epsilon = 1.0e-05f;
    if (qAbs(f1 - f2) <= epsilon)
        return true;
    return qAbs(f1 - f2) <= epsilon * qMax(qAbs(f1), qAbs(f2));
}
