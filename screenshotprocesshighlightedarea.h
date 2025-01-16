#ifndef SCREENSHOTPROCESSHIGHLIGHTEDAREA_H
#define SCREENSHOTPROCESSHIGHLIGHTEDAREA_H

#include <DrawData.h>

class ScreenshotProcessHighlightedArea : public QWidget{
    Q_OBJECT

public:
    enum ACTION{
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

protected:
    virtual void leaveEvent(QEvent *);
    virtual void enterEvent(QEnterEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

signals:
    void AreaMove(QRect data);
    QRect GetCurrentScreenGeometry();
    ProgramSetting GetProgramSettings();
    void CreateScreenshot(QRect data, QPixmap mask);

public:
    explicit ScreenshotProcessHighlightedArea(QWidget *parent = nullptr);

    void Hide();
    void Show(QRect geometry);
    void DeleteLine(QPixmap *pixmap = nullptr, bool delete_permanently = false);
    void DeleteFigure(QPixmap *pixmap = nullptr, bool delete_permanently = false);

private:
    void DrawMouseEvent();
    void DrawPaintEvent(QPainter &);
    void checkAndSetCursors(QMouseEvent *pe);
    QRect resizeAccordingly(QMouseEvent *pe);
    QRect ConvertCoords(QPoint start, QPoint end);
    void UpdateFigure(FIGURE_TYPE type, QWidget *parent);
    void CreateFigure(FIGURE_TYPE type, QWidget *parent);
    long MapValue(long x, long in_min, long in_max, long out_min, long out_max);

public slots:
    PAINTING_MODE GetDrawMode();
    void SetDrawMode(PAINTING_MODE mode);

    void LineCapInit(QComboBox *start, QComboBox *end);

    void ClearDrawArea();
    void PreCreateScreenshot();

    void LineDeleteRequest();

    DrawPenData GetPenData();
    DrawLineData GetLineData();
    DrawEraserData GetEraserData();
    DrawFigureData GetFigureData();
    void UpdateToolPanel_PenData(DrawPenData data);
    void UpdateToolPanel_LineData(DrawLineData data);
    void UpdateToolPanel_EraserData(DrawEraserData data);
    void UpdateToolPanel_FigureData(DrawFigureData data);

private:
    std::unique_ptr<LineObjectClass> _Line;
    std::unique_ptr<FigureObjectClass> _Figure;

    QPixmap _drawArea;
    QPainter _painter;
    PAINTING_MODE _drawMode = MODE_NONE;

    DrawPenData _penData;
    DrawLineData _lineData;
    DrawEraserData _eraserData;
    DrawFigureData _figureData;

    bool _mousePress = false;
    bool _areaInfoShow = false;
    bool _areaMouseActive = false;

    QSize mousePressDiffFromBorder;
    QPointF mousePressGlobalPosition;
    QPoint mousePressPosition, mouseMovePosition;
    QPoint mouseDrawPressPosition, mouseDrawMovePosition;
};

#endif // SCREENSHOTPROCESSHIGHLIGHTEDAREA_H
