#ifndef SCREENSHOTPROCESS_H
#define SCREENSHOTPROCESS_H

#include <screenshotprocesstoolpopup.h>
#include <screenshotprocesshighlightedarea.h>

class ScreenshotProcess : public QWidget{
    Q_OBJECT

protected:
    virtual void mouseReleaseEvent(QMouseEvent *pe);
    virtual void mousePressEvent(QMouseEvent *pe);
    virtual void mouseMoveEvent(QMouseEvent *pe);
    virtual void paintEvent(QPaintEvent *event);

public:
    explicit ScreenshotProcess(QWidget *parent = nullptr);

    void Show();
    bool Hide();

    void ClearFigure();
    void ClearBorderLines();
    void SetStopFrameImage(QPixmap image);
    void SetCursorPositionData(QPoint pos);
    QRect ConvertGlobalCoords(QPoint start, QPoint end);

    void PenDashLinesInit(QMap<QString, QVector<QVector<qreal>>> blockPatterns);

    ScreenshotProcessToolPopup* GetToolPopup() const;
    ScreenshotProcessHighlightedArea* GetHighlightedArea() const;

private slots:
    void AreaMove(QRect rect);
    void CreateScreenshot(QRect rect, QPixmap mask);

signals:
    void ProcessEvent_HideEnd();
    void ProcessEvent_ShowStart();
    void ProcessEvent_HideStart();
    void ProcessEvent_ShowEnd(QWidget *widget);

    void PlaySound(SOUNDMANAGER_SOUND_TYPE type);

    QRect GetCurrentScreenGeometry();
    ProgramSetting GetProgramSettings();
    void ScreenshotProcess_CreateScreenshot(QPixmap image, bool hidden);

private:
    QRect _hightlightAreaGeometry;
    QPixmap _bufferStopFrame;
    QPointF _mousePressGlobalPositionStart;
    bool _mouseLeftPressed = false, _flagStopFrame = false, _clearZone = false;

    AnimationsManager _animationManager;
    ScreenshotProcessToolPopup *_screenshotProcessTools;
    ScreenshotProcessHighlightedArea *_screenshotHighlightArea;
};

#endif // SCREENSHOTPROCESS_H
