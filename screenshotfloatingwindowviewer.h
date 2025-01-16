#ifndef SCREENSHOTFLOATINGWINDOWVIEWER_H
#define SCREENSHOTFLOATINGWINDOWVIEWER_H

#include <Includes.h>

class ScreenshotFloatingWindowViewer : public QWidget{
    Q_OBJECT

    enum {
        ACTION_NONE,
        ACTION_MOVE,
        ACTION_RESIZE_RIGHT_DOWN,
        ACTION_RESIZE_LEFT_DOWN
    } _action = ACTION_NONE;

public:
    explicit ScreenshotFloatingWindowViewer(QPixmap image, QList<ScreenshotFloatingWindowViewer*> *List_WindowViewer = NULL, QWidget *parent = nullptr);

    void setCursorOnAll(Qt::CursorShape);
    void checkAndSetCursors(QMouseEvent*);

public slots:
    void updateCursorPositionTimer();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent*);

signals:
    QRect GetCurrentScreenGeometry();
    void FloatingWindowClose(ScreenshotFloatingWindowViewer* window);

private:
    QPainter _painter;
    QTimer *_resizeIconTimer;

    byte _resizeIconIsHover = 0;

    QPixmap _currentImage;
    QList <ScreenshotFloatingWindowViewer*> *m_List_WindowViewer;

    /////////////////////////////// MOUSE WINDOW MOVE
    QString sizeBuffer;
    QSize mousePressDiffFromBorder;
    QRect resizeAccordingly(QMouseEvent*);
    QPointF mousePressGlobalPosition, mousePressPosition, mouseMovePosition;
    bool snapEdgeToScreenOrClosestFellow(QRect& newRect, const QRect& screen, std::function<void(QRect&,int)> setter, std::function<int(const QRect&)> getter, std::function<int(const QRect&)> oppositeGetter);
    /////////////////////////////// MOUSE WINDOW MOVE

    AnimationsManager _animationManager;
};

#endif // SCREENSHOTFLOATINGWINDOWVIEWER_H
