#ifndef SCREENSHOTHISTORYVIEWER_H
#define SCREENSHOTHISTORYVIEWER_H

#include <Includes.h>

#include <QMainWindow>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QGraphicsView>

// Кастомные классы которые описаные в этом заголовочном файле ниже основного класа
class Percent;

class ScreenshotHistoryViewer : public QGraphicsView{
    Q_OBJECT

signals:
    QRect GetCurrentScreenGeometry();

protected:
    void wheelEvent(QWheelEvent *event);

public:
    explicit ScreenshotHistoryViewer(QWidget *parent = nullptr);

    bool IsVisible();
    void Show(QPixmap image, ProgramSetting settings);
    void ChangePixmap(QPixmap image);
    void Hide();

private slots:
    void ZoomAnimationStep();

private:
    void UpdateZoom();
    void Zoom(float level);
    bool FloatCompare(float f1, float f2) const;
    qreal CalculateInitialScaleFactor(const QPixmap &image);

private:
    bool _isVisible = false;
    float _currentZoomLevel = 0, _oldZoomLevel = 0;

    QTimer *_zoomTimer;
    float _zoomStepSize;
    QPointF _lastCursorPosition;

    Percent *_percent;

    QGraphicsScene *_scene;
    QGraphicsPixmapItem* _item;
    AnimationsManager _animationManager;
};

//////////////////////////////////////////////////////////////////////////
///////////////////////////////// PERCENT ////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class Percent : public QWidget{
    Q_OBJECT

public:

    void SetActive(bool use){
        _active = use;
    }

    void setValue(float newValue){
        if(!_active)
            return;

        value = newValue;

        if(_showTimer->isActive()){
            _showTimer->stop();
            _showTimer->setInterval(500);
        }else{
            show();
            _animationManager.Create_WindowOpacity(this, nullptr, 150, 0, 0.75).Start();
        }

        _showTimer->start();
        repaint();
    }

    explicit Percent(QWidget *parent = nullptr) : QWidget(parent){
        setParent(parent);

        _showTimer = new QTimer(this);

        connect(_showTimer, &QTimer::timeout, this, [this](){
            _animationManager.Create_WindowOpacity(this, [this](){ _showTimer->stop(); hide(); }, 150, 0.75, 0).Start();
        });

        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool | Qt::WindowTransparentForInput);
    }

protected:
    void paintEvent(QPaintEvent *) {
        QPainter painter(this);

        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize((height() / 4 < width() / 4) ? height() / 4 : width() / 4);
        painter.setFont(font);

        QString text = (QString::number(value, 'f', 2)) + "%";

        // Получаем ограничивающий прямоугольник для текста
        QRectF textRect = painter.fontMetrics().boundingRect(text);

        qreal rectWidth = textRect.width() * 1.2;  // Увеличиваем ширину прямоугольника на некоторый запас
        qreal rectHeight = textRect.height();
        qreal rectX = (width() - rectWidth) / 2;
        qreal rectY = (height() - rectHeight) / 2;

        QRectF backgroundRect(rectX, rectY, rectWidth, rectHeight);

        painter.setPen(QColor(100, 100, 100, 200));
        painter.setBrush(QBrush(QColor(100, 100, 100, 200)));
        painter.drawRoundedRect(backgroundRect, 7, 7);

        painter.setPen(Qt::white);
        painter.drawText(backgroundRect, Qt::AlignCenter, text);
    }

private:
    float value = 0;
    bool _active = false;

    QTimer *_showTimer;
    AnimationsManager _animationManager;
};

#endif // SCREENSHOTHISTORYVIEWER_H
