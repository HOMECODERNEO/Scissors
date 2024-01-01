#ifndef SCREENSHOTFLOATINGWINDOWVIEWER_H
#define SCREENSHOTFLOATINGWINDOWVIEWER_H

#include <Includes.h>

class ScreenshotFloatingWindowViewer : public QWidget{
    Q_OBJECT

    enum {
        ACTION_NONE,
        ACTION_MOVE,
        //ACTION_PAINT,
        ACTION_RESIZE_RIGHT_DOWN,
    } _action = ACTION_NONE;

    enum{
        MOUSE_NONE,
        MOUSE_LEFT,
        MOUSE_MIDDLE,
        MOUSE_RIGHT
    } _mousePress = MOUSE_NONE;

public:
    explicit ScreenshotFloatingWindowViewer(int id, QPixmap image, QList<ScreenshotFloatingWindowViewer*> *List_WindowViewer = NULL, QWidget *parent = nullptr);

    void setCursorOnAll(Qt::CursorShape);
    void checkAndSetCursors(QMouseEvent*);

public slots:
    void updateCursorPositionTimer();

protected:
    //virtual void closeEvent(QCloseEvent*);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    //virtual void keyPressEvent(QKeyEvent *event);
    //virtual bool eventFilter(QObject *object, QEvent *event);

signals:
    void FloatingWindowClose(ScreenshotFloatingWindowViewer* window, int id, QPixmap image);

private:
    QPainter _painter;

    bool _resizeMove = false;
    bool _resizeIconIsHover = false;

    QTimer *_resizeIconTimer;
    QRect *_resizeIcon;

    int _screenListID = 0;
    QPixmap _currentImage;
    QList <ScreenshotFloatingWindowViewer*> *m_List_WindowViewer;

    /////////////////////////////// MOUSE WINDOW MOVE
    QString sizeBuffer;
    QSize mousePressDiffFromBorder;
    QRect resizeAccordingly(QMouseEvent*);
    QPoint mousePressGlobalPosition, mousePressPosition, mouseMovePosition;
    bool snapEdgeToScreenOrClosestFellow(QRect& newRect, const QRect& screen, std::function<void(QRect&,int)> setter, std::function<int(const QRect&)> getter, std::function<int(const QRect&)> oppositeGetter);
    /////////////////////////////// MOUSE WINDOW MOVE

    AnimationsManager _animationManager;
};

#endif // SCREENSHOTFLOATINGWINDOWVIEWER_H
