#ifndef SCREENSHOTPROCESSHIGHLIGHTEDAREA_H
#define SCREENSHOTPROCESSHIGHLIGHTEDAREA_H

#include <Includes.h>

class ScreenshotProcessHighlightedArea : public QWidget{
    Q_OBJECT

    enum{
        ACTION_NONE,
        ACTION_MOVE,
        ACTION_RESIZE_HOR_RIGHT,
        ACTION_RESIZE_HOR_LEFT,
        ACTION_RESIZE_VER_UP,
        ACTION_RESIZE_VER_DOWN,
        ACTION_RESIZE_RIGHT_DOWN,
        ACTION_RESIZE_RIGHT_UP,
        ACTION_RESIZE_LEFT_DOWN,
        ACTION_RESIZE_LEFT_UP
    } _modeAction = ACTION_NONE;

    QPoint mousePressPosition;
    QPointF mousePressGlobalPosition;
    QSize mousePressDiffFromBorder;

    void setCursorOnAll(Qt::CursorShape cursor);
    void checkAndSetCursors(QMouseEvent *pe);
    QRect resizeAccordingly(QMouseEvent *pe);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *pe);
    virtual void mousePressEvent(QMouseEvent *pe);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

signals:
    void AreaMove(QRect data);
    void CreateScreenshot(QRect data);
    ProgramSetting GetProgramSettings();

public:
    explicit ScreenshotProcessHighlightedArea(QWidget *parent = nullptr);

    void Show(QRect geometry);
    void Hide();

private:
    long MapValue(long x, long in_min, long in_max, long out_min, long out_max);

private:
    QPainter _painter;

    bool _areaMove = false;
    bool _areaResize = false;
    bool _areaInfoShow = false;
};

#endif // SCREENSHOTPROCESSHIGHLIGHTEDAREA_H
