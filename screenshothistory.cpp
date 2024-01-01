#include "screenshothistory.h"

ScreenshotHistory::ScreenshotHistory(QWidget *parent): QWidget{parent}{
    _settingsButtonTimer = new QTimer();
    _settingsMenu = new SettingsForm(this);
    _screenshotViewer = new ScreenshotHistoryViewer(this);

    // Атрибуты окна
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    // Подключение сигналов
    connect(this, SIGNAL(GetImagesData()), parent, SLOT(GetImagesData()));
    connect(this, SIGNAL(ClosingRequest()), parent, SLOT(ClosingRequest()));
    connect(this, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));
    connect(this, SIGNAL(HistoryRemoveAllItem()), parent, SLOT(HistoryRemoveAllItem()));
    connect(this, SIGNAL(HistoryRemoveItem(int)), parent, SLOT(HistoryRemoveItem(int)));
    connect(this, SIGNAL(ScreenshotHistory_ImageCopyToBuffer(int)), parent, SLOT(ScreenshotHistory_ImageCopyToBuffer(int)));

    connect(_screenshotViewer, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));

    connect(_settingsMenu, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(_settingsButtonTimer, &QTimer::timeout, this, &ScreenshotHistory::updateCursorPositionTimer);
    connect(_settingsMenu, SIGNAL(ChangeProgramSettings(ProgramSetting)), parent, SLOT(ChangeProgramSettings(ProgramSetting)));

    // Создаем элементы окна истории
    CreateHistoryUI();
    CreateContexMenu();

    // Устанавливаем разрешение согласно разрешения экрана
    setGeometry(emit GetActiveScreen()->geometry());

    // Указываем зоны в которых будут нарисованные кнопки
    _settingButton = new QRect(15, 15, 25, 25);
    _exitButton = new QRect(width() - 15 - 25, 15, 25, 25);
    _clearHistoryButton = new QRect(15, _settingButton->x() + _settingButton->height() + 10, 25, 25);

    // Устанавливаем позицию окна настроек программы
    _settingsMenu->move(QPoint(_settingButton->x() + _settingButton->width() + 7, _settingButton->y()));
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotHistory::Show(bool needUpdate){
    _settingsButtonTimer->start(25);

    UpdateHistoryGrid(emit GetImagesData(), needUpdate);

    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

bool ScreenshotHistory::Hide(){
    if(_screenshotViewer->IsVisible()){
        _screenshotViewer->Hide();

        // Если меню настроек было открыто то возобновляем его работу
        if(_settingsMenu->IsVisible())
            _settingsMenu->setEnabled(true);

        return false;
    }

    // Не даем закрыть пока не выберут действие
    if(_msgBoxVisible)
        return false;

    // Закрываем настройки если они были открыты на момент закрытия истории
    if(_settingsMenu->IsVisible())
        _settingsMenu->Hide();

    _settingsButtonTimer->stop();
    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();

    return true;
}

void ScreenshotHistory::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 175)));
    paint.drawRect(0, 0, width(), height());

    // Кнопка настроек
    paint.drawImage(*_settingButton, QImage(_settingButtonIsHover ? ":/Buttons/Resourse/Buttons/Settings.png" : ":/Buttons/Resourse/Buttons/Settings_Transparent.png"));
    paint.drawImage(*_exitButton, QImage(_exitButtonIsHover ? ":/Buttons/Resourse/Buttons/Close.png" : ":/Buttons/Resourse/Buttons/Close_Transparent.png"));
    paint.drawImage(*_clearHistoryButton, QImage(_clearHistoryButtonHover ? ":/Buttons/Resourse/Buttons/Clear.png" : ":/Buttons/Resourse/Buttons/Clear_Transparent.png"));

    // Если история пуста, выводим сообщение
    if((emit GetImagesData()).isEmpty()){
        // Устанавливаем стиль текста
        paint.setFont(QFont(QFont().defaultFamily(), 100));

        // Устанавливаем цвет текста
        paint.setPen(QColor(255, 255, 255));

        // Рассчитываем позицию для центрирования текста
        int textWidth = paint.fontMetrics().horizontalAdvance(_text_NoHIstory);
        int textHeight = paint.fontMetrics().height();

        // Рисуем текст на экране
        paint.drawText((width() - textWidth) / 2, (height() / 2) + (textHeight / 2.5), _text_NoHIstory);
    }

    paint.end();
}

void ScreenshotHistory::mouseReleaseEvent(QMouseEvent *event){
    if(_screenshotViewer->IsVisible())
        return;

    if(_settingButton->contains(event->pos())){

        // Нажата кнопка настроек
        if(_settingsMenu->IsVisible())
            _settingsMenu->Hide();
        else
            _settingsMenu->Show();

    }else if(_exitButton->contains(event->pos())){
        // Нажата кнопка закрытия приложения

        _animationManager.Create_WindowOpacity(this, [this](){ emit ClosingRequest(); }, 100, 1, 0);
        _animationManager.Start();
    }else if(_clearHistoryButton->contains(event->pos())){

        QDialog _dialog;

        // Устанавливаем тип окна
        _dialog.setWindowFlags(Qt::ToolTip);
        _dialog.resize(QSize(350, 115));
        _dialog.setStyleSheet("background-color: rgb(48, 48, 48);");

        // Создаем кнопки и вопросительный текст
        QPushButton okButton("OK");
        okButton.setMinimumHeight(25);
        okButton.setStyleSheet("QPushButton{ background-color: rgb(56, 90, 127); border-radius: 5px; color: white; }"
                               "QPushButton:hover{ background-color: rgb(80, 110, 150); }");

        QPushButton cancelButton("Cancel");
        cancelButton.setMinimumHeight(25);
        cancelButton.setStyleSheet("QPushButton{ background-color: rgb(68, 68, 68); border-radius: 5px; color: white; }"
                               "QPushButton:hover{ background-color: rgb(90, 90, 90); }");

        QLabel messageLabel(_text_Dialog_HistoryClear);

        QString htmlText = "<table><tr>"
                           "<td align=\"center\"><img src=\"" + QUrl::fromLocalFile(":/Icons/Resourse/Warning.png").toString() + "\" width=\"35\" height=\"35\"></td>"
                           "</tr><tr>"
                           "<td align=\"center\">" + _text_Dialog_HistoryClear + "</td>"
                           "</tr></table>";

        messageLabel.setText(htmlText);
        messageLabel.setTextFormat(Qt::RichText);
        messageLabel.setStyleSheet("color: white;");

        // Создаем сетку и добавляем виджеты
        QGridLayout layout(&_dialog);
        layout.addWidget(&messageLabel, 0, 0, 1, 2);
        layout.addWidget(&okButton, 1, 0);
        layout.addWidget(&cancelButton, 1, 1);

        // Устанавливаем растяжение для строки между messageLabel и кнопками
        layout.setRowStretch(1, 1);

        // Установите выравнивание элементов по центру
        layout.setAlignment(Qt::AlignCenter);

        // Подключаем сигналы кнопок к результатам
        connect(&okButton, &QPushButton::clicked, &_dialog, &QDialog::accept);
        connect(&cancelButton, &QPushButton::clicked, &_dialog, &QDialog::reject);

        _msgBoxVisible = true;

        if(_dialog.exec() == QDialog::Accepted){
            _msgBoxVisible = false;

            // Удаляем все файлы
            emit HistoryRemoveAllItem();

            // Обновляем сетку
            UpdateHistoryGrid(emit GetImagesData(), true);
        }else _msgBoxVisible = false;
    }
}

// Событие получения позиции курсора (для визуального создания эффекта Hover на нарисованных кнопках)
void ScreenshotHistory::updateCursorPositionTimer(){
    // Настройки
    if(_settingButton->contains(QCursor::pos())){
        _settingButtonIsHover = true;
        repaint();
    }else if(_settingButtonIsHover){
        _settingButtonIsHover = false;
        repaint();
    }

    // Выход
    if(_exitButton->contains(QCursor::pos())){
        _exitButtonIsHover = true;
        repaint();
    }else if(_exitButtonIsHover){
        _exitButtonIsHover = false;
        repaint();
    }

    // Очистка
    if(_clearHistoryButton->contains(QCursor::pos())){
        _clearHistoryButtonHover = true;
        repaint();
    }else if(_clearHistoryButtonHover){
        _clearHistoryButtonHover = false;
        repaint();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Левый клик по карточке истории с изображением
void ScreenshotHistory::onLeftClicked(byte id){
    if(_screenshotViewer->IsVisible())
        return;

    emit ScreenshotHistory_ImageCopyToBuffer(id);
}

// Средний клик по карточке истории с изображением
void ScreenshotHistory::onMiddleClicked(byte /*id*/){
    if(_screenshotViewer->IsVisible())
        return;

    //////// Code
}

// Правый клик по карточке истории с изображением
void ScreenshotHistory::onRightClicked(byte id){
    if(_screenshotViewer->IsVisible())
        return;

    _buttonIDBuffer = id;
    contextMenu->exec(this->mapToGlobal(QCursor::pos()));
}

// Пункт контексного меню (Просмотр изображения)
void ScreenshotHistory::ContextMenuView(){
    if(_screenshotViewer->IsVisible())
        return;

    _screenshotViewer->Show(emit GetImagesData()[_buttonIDBuffer].GetImage());

    // Если на момент открытия окна просмотра скриншота было открыто окно настроек, делаем его временно не активным
    if(_settingsMenu->IsVisible())
        _settingsMenu->setEnabled(false);
}

// Пункт контексного меню (Закрепить изображение)
void ScreenshotHistory::ContextMenuWindow(){
    if(_screenshotViewer->IsVisible())
        return;

    emit CreateFloatingWindow(_buttonIDBuffer, emit GetImagesData()[_buttonIDBuffer].GetImage());
}

// Пункт контексного меню (Сохранить изображение)
void ScreenshotHistory::ContextMenuSave(){
    if(_screenshotViewer->IsVisible())
        return;

    // Вызываем окно выбора места сохранения
    QString filename = QFileDialog::getSaveFileName(this, "Save file", "", ".png");

    // Если место выбрано то сохраняем выбранное изображение
    if(!filename.isEmpty()){
        QFile file(filename + ".png");
        emit GetImagesData()[_buttonIDBuffer].GetImage().save(&file, "PNG");
    }
}

// Пункт контексного меню (Удалить изображение)
void ScreenshotHistory::ContextMenuDeleteImage(){
    // Удаляем изображение из списка и удаляем файл
    emit HistoryRemoveItem(_buttonIDBuffer);

    // Обновляем сетку
    UpdateHistoryGrid(emit GetImagesData(), true);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Создаем визуальную сетку окна, на которой будут распологаться кнопки с изображениями
void ScreenshotHistory::CreateHistoryUI(){
    // Выделение памяти для классов
    QVBoxLayout *layout = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *gridWidget = new QWidget(scrollArea);
    gridLayout = new QGridLayout(gridWidget);

    // Просчитываем отступы между элементами и краями экрана
    int spacingTop = MapValue(emit GetActiveScreen()->geometry().height(), 0, ETALON_WIDTH, 0, 125);
    int spacing = MapValue(emit GetActiveScreen()->geometry().width(), 0, ETALON_HEIGHT, 0, 300);

    gridLayout->setVerticalSpacing(spacing); // Отступы по высоте
    gridLayout->setContentsMargins(0, spacingTop, 0, spacingTop); // Отступы [Лево] [Верх] [Право] [Низ]

    gridWidget->setLayout(gridLayout);
    scrollArea->setWidget(gridWidget);

    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background: transparent; border: none;");

    layout->addWidget(scrollArea);
    this->setLayout(layout);
}

// Создаем контекстное меню на правую кнопку мыши
void ScreenshotHistory::CreateContexMenu(){
    contextMenu = new QMenu(this);
    //contextMenu->setMaximumWidth(contextMenu->width() + 13);

    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuView()));
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuWindow()));
    contextMenu->addSeparator();
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuSave()));
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuDeleteImage()));

    contextMenu->setStyleSheet(
        "QMenu{"
            "color: rgb(255, 255, 255);"
            "background-color: rgba(64, 65, 66, 255);"
            "border: 0px solid rgba(64, 65, 66, 255);"
        "}"

        "QMenu::separator{"
            "height: 3px;"
            "background-color: rgba(99, 197, 255, 150);"
            "margin-left: 5px;"
            "margin-right: 5px;"
        "}"

        "QMenu::item {"
            "padding: 10px 10px 10px 10px;"
            "background-color: transparent;"
        "}"

        "QMenu::item:selected{"
            "color: rgb(255, 255, 255);"
            "background-color: rgba(86, 87, 88, 255);"
        "}"
    );
}

// Обновляем сетку истории с изображениями (если появились новые)
void ScreenshotHistory::UpdateHistoryGrid(QList<SaveManagerFileData> data, bool needUpdate){
    if(_historyImageButtonsNum == data.count() && !needUpdate)
        return;

    _historyImageButtonsNum = data.count();

    /////////////////////////////////////////////////////// Очистка сетки
    QLayoutItem* item;

    while((item = gridLayout->takeAt(0))){
        QWidget* widget = item->widget();

        if (widget)
            delete widget;

        delete item;
    }
    /////////////////////////////////////////////////////// Очистка сетки

    int iterator = 0;

    int col_num = COLUMN_COUNT;
    int row_num = _historyImageButtonsNum / COLUMN_COUNT;

    int height = MapValue(emit GetActiveScreen()->geometry().height(), 0, ETALON_WIDTH, 0, 250);
    int width = MapValue(emit GetActiveScreen()->geometry().width(), 0, ETALON_HEIGHT, 0, 250);

    if(row_num * COLUMN_COUNT < _historyImageButtonsNum)
        row_num++;

    for (int row = 0; row < row_num; ++row){
        for (int col = 0; col < col_num; ++col){
            if(iterator < _historyImageButtonsNum){

                CustomHistoryButton* button = new CustomHistoryButton(0,
                    [this, iterator](){ onLeftClicked(iterator); },
                    [this, iterator](){ onMiddleClicked(iterator); },
                    [this, iterator](){ onRightClicked(iterator); }
                );

                button->setFlat(true);
                button->setFixedSize(width, height);
                button->setStyleSheet("QPushButton{"
                                      "color: rgb(255, 255, 255);"
                                      "background-color: rgba(74, 94, 103,  100);"
                                      "border-radius: 7px;"
                                      "border-width: 0px;"
                                      "border-style: solid;"
                                      "}");

                button->setIcon(data[iterator].GetImage());
                button->setIconSize(QSize(button->width(), button->height()));

                iterator++;
                gridLayout->addWidget(button, row, col);
            }
        }
    }

    repaint();
}

// Маштабируем выходное число относительно диапазона входного
long ScreenshotHistory::MapValue(long x, long in_min, long in_max, long out_min, long out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotHistory::Event_ChangeLanguage(TranslateData data){
    _text_NoHIstory = data.translate("NO_HISTORY");
    _text_Dialog_HistoryClear = data.translate("CLEAR_HISTORY");

    contextMenu->actions().at(0)->setText(data.translate("CONTEXT_MENU_VIEW"));
    contextMenu->actions().at(1)->setText(data.translate("CONTEXT_MENU_WINDOW"));
    contextMenu->actions().at(3)->setText(data.translate("CONTEXT_MENU_SAVE"));
    contextMenu->actions().at(4)->setText(data.translate("CONTEXT_MENU_DELETE_IMAGE"));

    repaint();
}
