#ifndef SCREENSHOTPROCESSTOOLPOPUP_H
#define SCREENSHOTPROCESSTOOLPOPUP_H

#include <DrawData.h>

namespace Ui { class ScreenshotProcessToolPopup; }

class ScreenshotProcessToolPopup : public QWidget{
    Q_OBJECT

    enum{
        ACTION_NONE,
        ACTION_MOVE,
    } _action = ACTION_NONE;

    enum{
        COLOR_FROM_NONE,
        COLOR_FROM_PEN,
        COLOR_FROM_LINE,
        COLOR_FROM_SECONDARY_LINE,
        COLOR_FROM_MAIN_FIGURE,
        COLOR_FROM_SECONDARY_FIGURE
    } _selectColor = COLOR_FROM_NONE;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

signals:
    void ClearDrawArea();
    PAINTING_MODE GetDrawMode();
    QRect GetCurrentScreenGeometry();
    void SetDrawMode(PAINTING_MODE mode);
    QBitmap LoadBrushPatternAtIndex(int index);
    void LineCapInit(QComboBox *start, QComboBox *end);
    std::unique_ptr<FigureObjectClass> GetFigure(FIGURE_TYPE type, QWidget *parent);

    DrawPenData GetPenData();
    DrawLineData GetLineData();
    DrawEraserData GetEraserData();
    DrawFigureData GetFigureData();

    void Update_PenData(DrawPenData data);
    void Update_LineData(DrawLineData data);
    void Update_EraserData(DrawEraserData data);
    void Update_FigureData(DrawFigureData data);

public slots:
    bool GetScreenshotDisplay();

private slots:
    void on_button_clear_released();
    void on_button_displays_released();

    void currentColorChanged(QColor color);

    // PEN
    void on_mode_pen_button_released();
    void on_pen_color_released();
    void on_pen_size_valueChanged(int arg);

    // ERASER
    void on_mode_eraser_button_released();
    void on_eraser_size_valueChanged(int arg);

    // LINE
    void on_line_color_released();
    void on_mode_line_button_released();
    void on_line_size_valueChanged(int arg1);
    void on_line_style_currentIndexChanged(int index);
    void on_line_style_capend_currentIndexChanged(int index);
    void on_line_style_capstart_currentIndexChanged(int index);

    // FIGURE
    void on_figure_color_released();
    void on_mode_figure_button_released();
    void on_figure_secondary_color_released();
    void on_figure_burder_size_valueChanged(int arg1);
    void on_figure_type_currentIndexChanged(int index);
    void on_figure_patern_currentIndexChanged(int index);
    void on_figure_border_rounding_toggled(bool checked);
    void on_figure_style_border_currentIndexChanged(int index);

    void on_line_secondary_color_released();

public:
    explicit ScreenshotProcessToolPopup(QWidget *parent = nullptr); ~ScreenshotProcessToolPopup();

    void Show();
    void Hide();

    QComboBox *GetBrushPatternsBox();
    void PenDashLinesInit(QMap<QString, QVector<QVector<qreal>>> blockPatterns);

private:
    void FigureTypeInit();
    void ChangeMode(PAINTING_MODE mode);
    void ModePaintingExit(PAINTING_MODE mode);
    void ChangingScreenshotDisplay(bool show);
    void ModePaintingEnter(PAINTING_MODE mode);

private:
    QColorDialog *_colorDialog;
    Ui::ScreenshotProcessToolPopup *ui;
    AnimationsManager _animationManager;
    QMap<QString, QVector<QVector<qreal>>> _penLinesPatterns;

    QRect popupGeometryData;
    bool screenshot_hiden = false;
    QPointF mousePressGlobalPosition, mousePressPosition, mouseMovePosition;
    QColor pen_color = Qt::white, line_main_color = Qt::white, line_secondary_color = Qt::white, figure_main_color = Qt::white, figure_secondary_color = Qt::white;
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////// CUSTOMSTACKESWIDGET //////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomStackedWidget : public QStackedWidget{
    Q_OBJECT

public:
    explicit CustomStackedWidget(QWidget *parent = nullptr): QStackedWidget(parent){}

protected:
    QSize sizeHint() const override{ return currentWidget()->sizeHint(); }
    QSize minimumSizeHint() const override{ return currentWidget()->minimumSizeHint(); }
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////// END //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // SCREENSHOTPROCESSTOOLPOPUP_H
