#ifndef SCREENSHOTHISTORYVIEWER_H
#define SCREENSHOTHISTORYVIEWER_H

#include <Includes.h>

class ScreenshotHistoryViewer : public QWidget{
    Q_OBJECT

protected:
    virtual void mousePressEvent(QMouseEvent *pe);
    virtual void mouseMoveEvent(QMouseEvent *pe);
    virtual void mouseReleaseEvent(QMouseEvent *pe);
    virtual void wheelEvent(QWheelEvent * event);
    virtual void paintEvent(QPaintEvent *);

signals:
    QScreen* GetActiveScreen();

public:
    explicit ScreenshotHistoryViewer(QWidget *parent = nullptr);

    bool IsVisible();
    void Show(QPixmap image);
    void Hide();

private:
    QPixmap _currentImage;
    QPoint mousePressPosition;
    bool _isLeftClick = false, _isVisible = false;

    AnimationsManager _animationManager;

};

#endif // SCREENSHOTHISTORYVIEWER_H
