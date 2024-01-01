#ifndef SCREENSHOTHISTORYVIEWER_H
#define SCREENSHOTHISTORYVIEWER_H

#include <Includes.h>

#include <QMainWindow>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QGraphicsView>
#include <cmath>

class ScreenshotHistoryViewer : public QGraphicsView{
    Q_OBJECT

signals:
    QScreen* GetActiveScreen();

protected:
    void wheelEvent(QWheelEvent *event) override;

public:
    explicit ScreenshotHistoryViewer(QWidget *parent = nullptr);

    bool IsVisible();
    void Show(QPixmap image);
    void Hide();

private:
    void Zoom(float level);
    void UpdateZoom();
    void SetMatrix();
    qreal RotationRadians() const;

private:
    bool _isVisible = false;
    float _currentZoomLevel = 0, _oldZoomLevel = 0;

    QGraphicsScene *_scene;
    QGraphicsPixmapItem* _item;
    AnimationsManager _animationManager;
};

#endif // SCREENSHOTHISTORYVIEWER_H
