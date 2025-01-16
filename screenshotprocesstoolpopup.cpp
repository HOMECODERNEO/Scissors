#include "screenshotprocesstoolpopup.h"
#include "ui_screenshotprocesstoolpopup.h"

ScreenshotProcessToolPopup::ScreenshotProcessToolPopup(QWidget *parent) : QWidget(parent), ui(new Ui::ScreenshotProcessToolPopup){
    ui->setupUi(this);

    _colorDialog = new QColorDialog(this);
    _colorDialog->setCurrentColor(pen_color);
    connect(_colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(currentColorChanged(QColor)));

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    ui->parametrsPanel->hide();

    ui->pen_size->setValue(DEFFAULT_SIZE_PEN);
    ui->line_size->setValue(DEFFAULT_SIZE_LINE);
    ui->eraser_size->setValue(DEFFAULT_SIZE_ERASER);
    ui->figure_burder_size->setValue(DEFFAULT_SIZE_FIGURE_BORDER);

    ui->leftedge_label->setMouseTracking(true);
    ui->rightedge_label->setMouseTracking(true);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    ui->pen_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(pen_color.name()));
    ui->line_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(line_main_color.name()));
    ui->line_secondary_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(line_secondary_color.name()));
    ui->figure_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(figure_main_color.name()));
    ui->figure_secondary_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(figure_secondary_color.name()));

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    FigureTypeInit();
}

ScreenshotProcessToolPopup::~ScreenshotProcessToolPopup(){
    delete ui;
}

void ScreenshotProcessToolPopup::FigureTypeInit(){
    const int imageWidth = 42;
    const int imageHeight = 24;

    // Переменные с данными
    std::unique_ptr<FigureObjectClass> figure;

    // Устанавливаем размер иконок
    ui->figure_type->setIconSize(QSize(imageWidth, imageHeight));

    for(int i = 0; i < FIGURE_TYPE_COUNT; i++){

        switch(i){
            case 0: figure = std::make_unique<SquareObject>(nullptr); break;
            case 1: figure = std::make_unique<CircleObject>(nullptr); break;
            case 2: figure = std::make_unique<TriangleObject>(nullptr); break;
            case 3: figure = std::make_unique<DiamondObject>(nullptr); break;
            case 4: figure = std::make_unique<StarObject>(nullptr); break;
        }

        QPixmap drawArea(imageWidth, imageHeight);
        drawArea.fill(Qt::transparent);

        figure->SetSize(imageHeight, imageHeight);
        figure->SetPos((imageWidth - imageHeight) / 2, 0);

        QBrush brush;
        brush.setStyle(Qt::SolidPattern);

        figure->SetBorderSize(0);
        figure->SetColor(0, Qt::white);
        figure->SetColor(1, Qt::white);
        figure->SetFigurePatern(brush);

        QPainter painter(&drawArea);
        figure->DrawAreaPaintFigure(&painter);

        ui->figure_type->addItem(drawArea, "");
    }
}

void ScreenshotProcessToolPopup::PenDashLinesInit(QMap<QString, QVector<QVector<qreal>>> blockPatterns){
    const int imageWidth = 100;
    const int imageHeight = 24;

    // Заполняем список паттернов
    _penLinesPatterns = blockPatterns;

    // Линия
    if(_penLinesPatterns.contains("Lines")){
        const auto &patterns = _penLinesPatterns["Lines"];

        // Устанавливаем размер иконок
        ui->line_style->setIconSize(QSize(imageWidth, imageHeight));

        // Генерируем список паттернов
        for(const auto &pattern : patterns){
            // Заготовка для генерации
            QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);

            QPen pen;
            pen.setWidth(7);
            pen.setColor(Qt::white);

            pen.setDashPattern(pattern);
            pen.setStyle(Qt::CustomDashLine);
            painter.setPen(pen);

            // Рисуем линию с паттерном по центру изображения
            painter.drawLine(7, imageHeight / 2, imageWidth - 7, imageHeight / 2);

            // Вставляем в выпадающий список [Линий]
            ui->line_style->addItem(QPixmap::fromImage(image), "");
        }

        QPen penStyle;
        penStyle.setStyle(Qt::CustomDashLine);
        penStyle.setDashPattern(patterns.at(0));

        DrawLineData data = emit GetLineData();
        data.SetLineStyle(penStyle);
        emit Update_LineData(data);

    }else{
        // Вставляем в выпадающий список [Линий]
        ui->line_style->addItem("SOLID");
    }

    // Контуры фигур
    if(_penLinesPatterns.contains("Figures")){
        const auto &patterns = _penLinesPatterns["Figures"];

        // Устанавливаем размер иконок
        ui->figure_style_border->setIconSize(QSize(imageWidth, imageHeight));

        // Генерируем список паттернов
        for(const auto &pattern : patterns){
            QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);

            QPen pen;
            pen.setWidth(7);
            pen.setColor(Qt::white);

            pen.setDashPattern(pattern);
            pen.setStyle(Qt::CustomDashLine);
            painter.setPen(pen);

            // Рисуем линию с паттерном по центру изображения
            painter.drawLine(7, imageHeight / 2, imageWidth - 7, imageHeight / 2);

            // Вставляем в выпадающий список [Фигур]
            ui->figure_style_border->addItem(QPixmap::fromImage(image), "");
        }        
    }else{
        // Вставляем в выпадающий список [Фигур]
        ui->figure_style_border->addItem("SOLID");
    }

    // Устанавливаем стили линий
    on_line_style_currentIndexChanged(0);
    on_figure_style_border_currentIndexChanged(0);

    // Вызываем обновление видов начала и конца линии
    emit LineCapInit(ui->line_style_capstart, ui->line_style_capend);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////[-------Get-теры/Set-теры--------]////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScreenshotProcessToolPopup::GetScreenshotDisplay(){ return screenshot_hiden; }
QComboBox* ScreenshotProcessToolPopup::GetBrushPatternsBox(){ return ui->figure_patern; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////[-------Основные функции--------]////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcessToolPopup::Show(){
    ////////////////////////////////////

    ChangingScreenshotDisplay(false);
    ui->parametrsPanel->hide();

    emit SetDrawMode(MODE_NONE);

    ////////////////////////////////////

    int width = sizeHint().width();
    int height = sizeHint().height();

    QRect screen = emit GetCurrentScreenGeometry();

    int posX = (screen.width() / 2) - (width / 2);

    setGeometry(posX, 0, width, height);

    show();

    _animationManager.Create_WindowOpacity(this, [this](){ popupGeometryData = geometry(); }, 100, 0, 1).Start();
}

void ScreenshotProcessToolPopup::Hide(){
    ModePaintingExit(emit GetDrawMode());

    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();
}

void ScreenshotProcessToolPopup::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setRenderHint(QPainter::Antialiasing);
    paint.setPen(Qt::NoPen);                            // Убираем обводку
    paint.setBrush(QBrush(QColor(65, 82, 90, 150)));    // Задаем цвет фона
    paint.drawRoundedRect(rect(), 10, 10);              // Задаем радиус закругления
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////[-------Событие выбора инструмента рисования--------]/////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// PEN
void ScreenshotProcessToolPopup::on_pen_size_valueChanged(int arg){
    DrawPenData data = emit GetPenData();
    data.SetWidth(arg);

    emit Update_PenData(data);
}

// ERASER
void ScreenshotProcessToolPopup::on_eraser_size_valueChanged(int arg){
    DrawEraserData data = emit GetEraserData();
    data.SetWidth(arg);

    emit Update_EraserData(data);
}

// LINE
void ScreenshotProcessToolPopup::on_line_size_valueChanged(int arg){
    DrawLineData data = emit GetLineData();
    data.SetWidth(arg);

    emit Update_LineData(data);
}

void ScreenshotProcessToolPopup::on_line_style_currentIndexChanged(int index){
    QPen penStyle;

    if(ui->line_style->count() <= 1){
        penStyle.setStyle(Qt::SolidLine);
    }else{
        if(_penLinesPatterns.contains("Lines")){
            const auto &patterns = _penLinesPatterns["Lines"];

            penStyle.setStyle(Qt::CustomDashLine);
            penStyle.setDashPattern(patterns.at(index));
        }else{
            penStyle.setStyle(Qt::SolidLine);
        }
    }

    DrawLineData data = emit GetLineData();
    data.SetLineStyle(penStyle);

    emit Update_LineData(data);
}

void ScreenshotProcessToolPopup::on_line_style_capstart_currentIndexChanged(int index){
    DrawLineData data = emit GetLineData();

    switch(index){
        case 0: data.SetLineCapStart(LINESTYLE_CAP_NONE); break;
        case 1: data.SetLineCapStart(LINESTYLE_CAP_ARROW); break;
        case 2: data.SetLineCapStart(LINESTYLE_CAP_SPLIT_ARROW); break;
        case 3: data.SetLineCapStart(LINESTYLE_CAP_CROSS); break;
        case 4: data.SetLineCapStart(LINESTYLE_CAP_SQUARE); break;
        case 5: data.SetLineCapStart(LINESTYLE_CAP_CIRCLE); break;

        default: data.SetLineCapStart(LINESTYLE_CAP_NONE); break;
    }

    emit Update_LineData(data);
}

void ScreenshotProcessToolPopup::on_line_style_capend_currentIndexChanged(int index){
    DrawLineData data = emit GetLineData();

    switch(index){
        case 0: data.SetLineCapEnd(LINESTYLE_CAP_NONE); break;
        case 1: data.SetLineCapEnd(LINESTYLE_CAP_ARROW); break;
        case 2: data.SetLineCapEnd(LINESTYLE_CAP_SPLIT_ARROW); break;
        case 3: data.SetLineCapEnd(LINESTYLE_CAP_CROSS); break;
        case 4: data.SetLineCapEnd(LINESTYLE_CAP_SQUARE); break;
        case 5: data.SetLineCapEnd(LINESTYLE_CAP_CIRCLE); break;

        default: data.SetLineCapEnd(LINESTYLE_CAP_NONE);
    }

    emit Update_LineData(data);
}

// FIGURE
void ScreenshotProcessToolPopup::on_figure_type_currentIndexChanged(int index){
    DrawFigureData data = emit GetFigureData();
    data.SetType(static_cast<FIGURE_TYPE>(index));
    emit Update_FigureData(data);
}

void ScreenshotProcessToolPopup::on_figure_patern_currentIndexChanged(int index){
    DrawFigureData data = emit GetFigureData();

    switch(index){
    case 0:{
        QBrush brush;
        brush.setStyle(Qt::NoBrush);
        data.SetPattern(brush);
        break;
    }

    case 1:{
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        data.SetPattern(brush);
        break;
    }

    default:{
        QBitmap mask = emit LoadBrushPatternAtIndex(index - 2);

        QBrush brush(mask);
        brush.setStyle(Qt::TexturePattern);
        data.SetPattern(brush);
        break;
    }
    }

    emit Update_FigureData(data);
}

void ScreenshotProcessToolPopup::on_figure_style_border_currentIndexChanged(int index){
    QPen penStyle;

    if(ui->figure_style_border->count() <= 1){
        penStyle.setStyle(Qt::SolidLine);
    }else{
        if(_penLinesPatterns.contains("Figures")){
            const auto &patterns = _penLinesPatterns["Figures"];

            penStyle.setStyle(Qt::CustomDashLine);
            penStyle.setDashPattern(patterns.at(index));
        }else{
            penStyle.setStyle(Qt::SolidLine);
        }
    }

    DrawFigureData data = emit GetFigureData();
    data.SetBorderStyle(penStyle);

    emit Update_FigureData(data);
}

void ScreenshotProcessToolPopup::on_figure_burder_size_valueChanged(int arg){
    DrawFigureData data = emit GetFigureData();
    data.SetBorderSize(arg);

    emit Update_FigureData(data);
}

void ScreenshotProcessToolPopup::on_figure_border_rounding_toggled(bool checked){
    DrawFigureData data = emit GetFigureData();
    data.SetBorderRounding(checked);

    emit Update_FigureData(data);
}

void ScreenshotProcessToolPopup::currentColorChanged(QColor color){
    switch(_selectColor){
        case COLOR_FROM_NONE: break;

        case COLOR_FROM_PEN:{
            pen_color = color;
            ui->pen_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));

            // Обновляем данные
            DrawPenData data = emit GetPenData();
            data.SetColor(pen_color);
            emit Update_PenData(data);

            break;
        }

        case COLOR_FROM_LINE:{
            line_secondary_color = line_main_color = color;
            ui->line_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));
            ui->line_secondary_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));

            // Обновляем данные
            DrawLineData data = emit GetLineData();
            data.SetSecondaryColor(line_main_color);
            data.SetMainColor(line_main_color);
            emit Update_LineData(data);

            break;
        }

        case COLOR_FROM_SECONDARY_LINE:{
            line_secondary_color = color;
            ui->line_secondary_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));

            // Обновляем данные
            DrawLineData data = emit GetLineData();
            data.SetSecondaryColor(line_secondary_color);
            emit Update_LineData(data);

            break;
        }

        case COLOR_FROM_MAIN_FIGURE:{
            figure_main_color = color;
            ui->figure_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));

            DrawFigureData data = emit GetFigureData();
            data.SetMainColor(figure_main_color);
            emit Update_FigureData(data);

            break;
        }

        case COLOR_FROM_SECONDARY_FIGURE:{
            figure_secondary_color = color;
            ui->figure_secondary_color->setStyleSheet(QString("background-color: %1; border-radius: 0px;").arg(color.name()));

            DrawFigureData data = emit GetFigureData();
            data.SetSecondaryColor(figure_secondary_color);
            emit Update_FigureData(data);

            break;
        }
    }
}

void ScreenshotProcessToolPopup::on_button_displays_released(){
    ChangingScreenshotDisplay(!screenshot_hiden);
}

void ScreenshotProcessToolPopup::on_button_clear_released(){
    emit ClearDrawArea();
}

void ScreenshotProcessToolPopup::on_mode_pen_button_released(){ ChangeMode(MODE_PEN); }
void ScreenshotProcessToolPopup::on_mode_eraser_button_released(){ ChangeMode(MODE_ERASER); }
void ScreenshotProcessToolPopup::on_mode_line_button_released(){ ChangeMode(MODE_LINE); }
void ScreenshotProcessToolPopup::on_mode_figure_button_released(){ ChangeMode(MODE_FIGURE); }

void ScreenshotProcessToolPopup::on_pen_color_released(){
    _selectColor = COLOR_FROM_PEN;
    _colorDialog->setCurrentColor(pen_color);
    _colorDialog->exec();
}

void ScreenshotProcessToolPopup::on_line_color_released(){
    _selectColor = COLOR_FROM_LINE;
    _colorDialog->setCurrentColor(line_main_color);
    _colorDialog->exec();
}

void ScreenshotProcessToolPopup::on_line_secondary_color_released(){
    _selectColor = COLOR_FROM_SECONDARY_LINE;
    _colorDialog->setCurrentColor(line_secondary_color);
    _colorDialog->exec();
}

void ScreenshotProcessToolPopup::on_figure_color_released(){
    _selectColor = COLOR_FROM_MAIN_FIGURE;
    _colorDialog->setCurrentColor(figure_main_color);
    _colorDialog->exec();
}

void ScreenshotProcessToolPopup::on_figure_secondary_color_released(){
    _selectColor = COLOR_FROM_SECONDARY_FIGURE;
    _colorDialog->setCurrentColor(figure_secondary_color);
    _colorDialog->exec();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcessToolPopup::ChangeMode(PAINTING_MODE mode){

    PAINTING_MODE _currentMode = emit GetDrawMode();

    switch(mode){
        case MODE_NONE: break;

        case MODE_PEN:{
            if(_currentMode == MODE_PEN){
                ModePaintingExit(_currentMode);
                break;
            }

            ModePaintingExit(_currentMode);
            ModePaintingEnter(mode);
            break;
        }

        case MODE_ERASER:{
            if(_currentMode == MODE_ERASER){
                ModePaintingExit(_currentMode);
                break;
            }

            ModePaintingExit(_currentMode);
            ModePaintingEnter(mode);
            break;
        }

        case MODE_LINE:{
            if(_currentMode == MODE_LINE){
                ModePaintingExit(_currentMode);
                break;
            }

            ModePaintingExit(_currentMode);
            ModePaintingEnter(mode);
            break;
        }

        case MODE_FIGURE:{
            if(_currentMode == MODE_FIGURE){
                ModePaintingExit(_currentMode);
                break;
            }

            ModePaintingExit(_currentMode);
            ModePaintingEnter(mode);
            break;
        }
    }

    // Плавненько изменяем размер окна при переключении режимов
    QTimer::singleShot(120, this, [=](){
        QRect newRect(x(), y(), sizeHint().width(), sizeHint().height());

        if(popupGeometryData.width() != newRect.width() || popupGeometryData.height() != newRect.height()){

            // Вычысляем новый размер окна
            QRect adjustedNewRect = newRect.translated(-( (newRect.width() - popupGeometryData.width()) / 2 ), 0);

            // Запускаем анимацию
            _animationManager.Create_ObjectGeometry(this, nullptr, 150, popupGeometryData, adjustedNewRect).Start();

            // Обновляем переменную геометрии виджета
            popupGeometryData = adjustedNewRect;
        }
    });
}

void ScreenshotProcessToolPopup::ModePaintingEnter(PAINTING_MODE mode){
    emit SetDrawMode(mode);

    ui->panel_background->show();
    ui->parametrsPanel->show();

    QString styleColor = "background-color: rgba(213, 198, 122, 200);";

    switch(mode){
        case MODE_NONE: break;

        case MODE_PEN: ui->mode_pen_background->setStyleSheet(styleColor); break;
        case MODE_LINE: ui->mode_line_background->setStyleSheet(styleColor); break;
        case MODE_ERASER: ui->mode_eraser_background->setStyleSheet(styleColor); break;
        case MODE_FIGURE: ui->mode_figure_background->setStyleSheet(styleColor); break;
    }

    // Анимация плавной смены страниц (параметров режимов)
    int pageIndex[] = { 3, 1, 2, 4 }; // Страницы Stacked Widget для каждого режима

    if(!( ui->parametrsPanel->currentIndex() == pageIndex[mode - 1] - 1) ){

        int targetIndex = pageIndex[mode - 1] - 1;

        // Запускаем анимацию на скрытие старой страницы
        _animationManager.Create_StackedWidgetOpacity(ui->parametrsPanel->currentWidget(), [this, targetIndex](){

                                // Устанавливаем новую страницу
                                ui->parametrsPanel->setCurrentIndex(targetIndex);

                                // Запускаем анимацию на отображение новой страницы
                                QWidget *pageWidget = ui->parametrsPanel->widget(targetIndex);
                                _animationManager.Create_StackedWidgetOpacity(pageWidget, [pageWidget](){ pageWidget->update(); }, 75, 0, 1).start(); // Отображаем

                             }, 75, 1, 0).start(); // Прячем
    }

    // MODE ENTER CODE
    // ....
}

void ScreenshotProcessToolPopup::ModePaintingExit(PAINTING_MODE mode){

    QString styleColor = "background-color: rgba(0, 0, 0, 25);";

    switch(mode){
        case MODE_NONE: break;

        case MODE_PEN: ui->mode_pen_background->setStyleSheet(styleColor); break;
        case MODE_LINE: ui->mode_line_background->setStyleSheet(styleColor); break;
        case MODE_ERASER: ui->mode_eraser_background->setStyleSheet(styleColor); break;
        case MODE_FIGURE: ui->mode_figure_background->setStyleSheet(styleColor); break;
    }

    // MODE EXIT CODE
    // ....

    emit SetDrawMode(MODE_NONE);
    ui->parametrsPanel->hide();
    ui->panel_background->hide();
}

void ScreenshotProcessToolPopup::ChangingScreenshotDisplay(bool show){
    screenshot_hiden = show;

    QString type = (screenshot_hiden) ? QString("border-image: url(:/Buttons/Resourse/Buttons/Drawing/Hide.png);") : QString("border-image: url(:/Buttons/Resourse/Buttons/Drawing/Show.png);");
    ui->button_displays->setStyleSheet(type + "background-color: rgba(255, 255, 255, 0);");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////[-------Событие перетаскивания--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotProcessToolPopup::mousePressEvent(QMouseEvent *pe){
    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPosition();

    if (pe->buttons() == Qt::LeftButton){
        _action = ACTION_MOVE;
        setCursor(Qt::ClosedHandCursor);
    }else{
        _action = ACTION_NONE;
        setCursor(Qt::ArrowCursor);
    }

    update();
}

void ScreenshotProcessToolPopup::mouseMoveEvent(QMouseEvent *pe){
    mouseMovePosition = pe->pos();

    if(_action == ACTION_MOVE){
        QPointF moveHere;
        moveHere = pe->globalPosition() - mousePressPosition;

        QRect screenRect = emit GetCurrentScreenGeometry();
        QRect newRect = QRect(moveHere.toPoint(), geometry().size());

        if(newRect.x() < 0) newRect.setX(0);
        if(newRect.y() < 0) newRect.setY(0);

        if(newRect.x() + newRect.width() > screenRect.width()) newRect.setX(screenRect.width() - newRect.width());
        if(newRect.y() + newRect.height() > screenRect.height()) newRect.setY(screenRect.height() - newRect.height());

        popupGeometryData = QRect(newRect.topLeft(), QSize(width(), height()));

        move(newRect.topLeft());
        update();
    }
}

void ScreenshotProcessToolPopup::mouseReleaseEvent(QMouseEvent *){
    _action = ACTION_NONE;
    setCursor(Qt::ArrowCursor);

    update();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////[-------Конец--------]/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
