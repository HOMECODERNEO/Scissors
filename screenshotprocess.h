#ifndef SCREENSHOTPROCESS_H
#define SCREENSHOTPROCESS_H

#include <screenshotprocesshighlightedarea.h>

class ScreenshotProcess : public QWidget{
    Q_OBJECT

protected:
    virtual void mouseReleaseEvent(QMouseEvent *pe);
    virtual void mousePressEvent(QMouseEvent *pe);
    virtual void mouseMoveEvent(QMouseEvent *pe);
    virtual void paintEvent(QPaintEvent *event);

private slots:
    void AreaMove(QRect rect);
    void CreateScreenshot(QRect rect);

signals:
    QScreen* GetActiveScreen();
    ProgramSetting GetProgramSettings();
    void ScreenshotProcessEnd(QPixmap image);

public:
    explicit ScreenshotProcess(QWidget *parent = nullptr);

    void Show();
    void Hide();

    void ClearBorderLines();
    void SetStopFrameImage(QPixmap image);
    QRect ConvertGlobalCoords(QPoint start, QPoint end);

private:
    QRect _hightlightAreaGeometry;
    QPixmap _bufferStopFrame;
    QPointF _mousePressGlobalPositionStart;
    bool _mouseLeftPressed = false, _flagStopFrame = false;

    AnimationsManager _animationManager;
    ScreenshotProcessHighlightedArea *_screenshotHighlightArea;
};

#endif // SCREENSHOTPROCESS_H
