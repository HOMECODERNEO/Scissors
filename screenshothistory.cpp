#include "screenshothistory.h"

// Конструктор
ScreenshotHistory::ScreenshotHistory(QWidget *parent): QWidget{parent}{
    _buttonsHoverTimer = new QTimer();
    _settingsMenu = new SettingsForm(this);
    _screenshotViewer = new ScreenshotHistoryViewer(this);

    // Атрибуты окна
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    // Подключение сигналов
    connect(parent, SIGNAL(LoadingSettingsEnd(ProgramSetting)), this, SLOT(LoadingSettingsEnd(ProgramSetting)));

    connect(this, SIGNAL(GetImagesData()), parent, SLOT(GetImagesData()));
    connect(this, SIGNAL(ClosingRequest()), parent, SLOT(ClosingRequest()));
    connect(this, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));
    connect(this, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(this, SIGNAL(HistoryRemoveAllItem()), parent, SLOT(HistoryRemoveAllItem()));
    connect(this, SIGNAL(HistoryRemoveItem(int)), parent, SLOT(HistoryRemoveItem(int)));
    connect(this, SIGNAL(ScreenshotProcessEnd(QPixmap)), parent, SLOT(ScreenshotProcessEnd(QPixmap)));
    connect(this, SIGNAL(CreateFloatingWindow(int, QPixmap)), parent, SLOT(CreateFloatingWindow(int, QPixmap)));
    connect(_settingsMenu, SIGNAL(ChangeProgramLanguage(QString)), parent, SLOT(ChangeProgramLanguage(QString)));
    connect(this, SIGNAL(ScreenshotHistory_ImageCopyToBuffer(int)), parent, SLOT(ScreenshotHistory_ImageCopyToBuffer(int)));
    connect(this, SIGNAL(ShowPopup(QString, QString, int, QString)), parent, SLOT(ShowPopup(QString, QString, int, QString)));

    connect(_screenshotViewer, SIGNAL(GetActiveScreen()), parent, SLOT(GetActiveScreen()));

    connect(_settingsMenu, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(_buttonsHoverTimer, &QTimer::timeout, this, &ScreenshotHistory::updateCursorPositionTimer);
    connect(_settingsMenu, SIGNAL(ChangeProgramSettings(ProgramSetting)), parent, SLOT(ChangeProgramSettings(ProgramSetting)));
    connect(this, SIGNAL(ChangeProgramSettings(ProgramSetting)), parent, SLOT(ChangeProgramSettings(ProgramSetting)));

    // Создаем элементы окна истории
    CreateHistoryUI();
    CreateContexMenu();

    // Устанавливаем разрешение согласно разрешения экрана
    setGeometry(emit GetActiveScreen()->geometry());

    // Указываем зоны в которых будут нарисованные кнопки
    _settingButton = new QRect(15, 15, 25, 25);
    _exitButton = new QRect(width() - 15 - 25, 15, 25, 25);
    _clearHistoryButton = new QRect(15, _settingButton->y() + _settingButton->height() + 10, 25, 25);
    _uploadImageButton = new QRect(15, _clearHistoryButton->y() + _clearHistoryButton->height() + 10, 25, 25);

    // Устанавливаем позицию окна настроек программы (если настройки сбились)
    ProgramSetting settings = emit GetProgramSettings();
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotHistory::LoadingSettingsEnd(ProgramSetting settings){

    // Устанавливаем позицию окна настроек программы (если настройки сбились)
    if(settings.Get_SettingsWindowGeometry().x() + settings.Get_SettingsWindowGeometry().y() + settings.Get_SettingsWindowGeometry().width() + settings.Get_SettingsWindowGeometry().height() <= 10){
        settings.Set_SettingsWindowGeometry(QRect(_settingButton->x() + _settingButton->width() + 7, _settingButton->y(), 500, 300));
        emit ChangeProgramSettings(settings);
    }
}

// Отображение истории
void ScreenshotHistory::Show(bool needUpdate){
    _buttonsHoverTimer->start(1);

    UpdateHistoryGrid(emit GetImagesData(), needUpdate);

    show();
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

// Скрытие истории
bool ScreenshotHistory::Hide(){
    if(_screenshotViewer->IsVisible()){
        _screenshotViewer->Hide();

        // Если меню настроек было открыто то возобновляем его работу
        if(_settingsMenu->IsVisible())
            _settingsMenu->setEnabled(true);

        return false;
    }

    // Не даем закрыть пока не выберут действие || или же пока открыто окно выбора изображения мы не закроем окно
    if(_msgBoxVisible || _imageLoadVisible)
        return false;

    // Закрываем настройки если они были открыты на момент закрытия истории
    if(_settingsMenu->IsVisible())
        _settingsMenu->Hide();

    _buttonsHoverTimer->stop();
    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();

    return true;
}

// Событие отрисовки окна
void ScreenshotHistory::paintEvent(QPaintEvent *){
    QPainter paint(this);

    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 175)));
    paint.drawRect(0, 0, width(), height());

    // Рисуем кнопки
    paint.drawImage(*_settingButton, QImage(_settingButtonIsHover ? ":/Buttons/Resourse/Buttons/Settings.png" : ":/Buttons/Resourse/Buttons/Settings_Transparent.png"));
    paint.drawImage(*_exitButton, QImage(_exitButtonIsHover ? ":/Buttons/Resourse/Buttons/Close.png" : ":/Buttons/Resourse/Buttons/Close_Transparent.png"));
    paint.drawImage(*_clearHistoryButton, QImage(_clearHistoryButtonHover ? ":/Buttons/Resourse/Buttons/Clear.png" : ":/Buttons/Resourse/Buttons/Clear_Transparent.png"));
    paint.drawImage(*_uploadImageButton, QImage(_uploadImageButtonHover ? ":/Buttons/Resourse/Buttons/Upload.png" : ":/Buttons/Resourse/Buttons/Upload_Transparent.png"));

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

    // Отображаем текущую версию программы
    QFont font = paint.font();
    font.setPointSize(12);
    paint.setFont(font);
    paint.setPen(QColor(255, 255, 255, 100));
    paint.drawText(rect().adjusted(0, 0, 0, -10), Qt::AlignHCenter | Qt::AlignBottom, APPLICATION_VERSION);
    paint.drawText(rect().adjusted(0, 10, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "[" + QString::number(_historyImageButtonsNum) + " / " + QString::number(_historyMaxSize) + "]");

    paint.end();
}

void ScreenshotHistory::mouseReleaseEvent(QMouseEvent *event){
    if(_screenshotViewer->IsVisible())
        return;

    // Нажата кнопка настроек
    if(_settingButton->contains(event->pos())){

        if(_settingsMenu->IsVisible())
            _settingsMenu->Hide();
        else
            _settingsMenu->Show();

    // Нажата кнопка закрытия приложения
    }else if(_exitButton->contains(event->pos())){

        _animationManager.Create_WindowOpacity(this, [this](){ emit ClosingRequest(); }, 100, 1, 0);
        _animationManager.Start();

    // Нажата кнопка очистки истории
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

    // Нажата кнопка добавления изображения
    }else if(_uploadImageButton->contains(event->pos())){

        _imageLoadVisible = true;

        QStringList files = QFileDialog::getOpenFileNames(this, "Выберите изображение", "", "Изображения (*.png *.jpg *.bmp);;Все файлы (*)");

        _imageLoadVisible = false;

        // Записываем количество
        int count = files.count();

        // Загрузка отменена
        if(count <= 0){
            emit ShowPopup("#IMAGE_LOAD_CANCELED#", "", 2000, "");
            return;
        }

        // Проходимся по каждому файлу
        for(const QString& path : files){
            if(!path.isEmpty()){
                QPixmap image = QPixmap(path);

                if(!image.isNull()){
                    emit ScreenshotProcessEnd(image);
                }else{
                    // Ошибка загрузки изображения
                    count--;
                    emit ShowPopup("ERROR: #IMAGE_LOAD_ERROR#: " + QFileInfo(path).fileName(), "", 3000, "");
                }
            }
        }

        // Обновляем сетку и выводим сообщение
        UpdateHistoryGrid(emit GetImagesData(), true);
        emit ShowPopup("#IMAGE_LOADED1#: " + QString::number(count) + " #IMAGE_LOADED2#", "", 2000, "");
    }
}

// Событие получения позиции курсора (для визуального создания эффекта Hover на нарисованных кнопках)
void ScreenshotHistory::updateCursorPositionTimer(){
    bool hover = false;

    hover = _settingButton->contains(QCursor::pos());

    // Настройки
    if(hover && !_settingButtonIsHover){
        _settingButtonIsHover = true;
        repaint();
    }else if(!hover && _settingButtonIsHover){
        _settingButtonIsHover = false;
        repaint();
    }

    hover = _exitButton->contains(QCursor::pos());

    // Выход
    if(hover && !_exitButtonIsHover){
        _exitButtonIsHover = true;
        repaint();
    }else if(!hover && _exitButtonIsHover){
        _exitButtonIsHover = false;
        repaint();
    }

    hover = _clearHistoryButton->contains(QCursor::pos());

    // Очистка
    if(hover && !_clearHistoryButtonHover){
        _clearHistoryButtonHover = true;
        repaint();
    }else if(!hover && _clearHistoryButtonHover){
        _clearHistoryButtonHover = false;
        repaint();
    }

    hover = _uploadImageButton->contains(QCursor::pos());

    // Загрузка изображения
    if(hover && !_uploadImageButtonHover){
        _uploadImageButtonHover = true;
        repaint();
    }else if(!hover && _uploadImageButtonHover){
        _uploadImageButtonHover = false;
        repaint();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Левый клик по карточке истории с изображением
void ScreenshotHistory::onLeftClicked(byte id){
    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    emit ScreenshotHistory_ImageCopyToBuffer(id);
}

// Средний клик по карточке истории с изображением
void ScreenshotHistory::onMiddleClicked(byte /*id*/){
    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    //////// Code
}

// Правый клик по карточке истории с изображением
void ScreenshotHistory::onRightClicked(byte id){
    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    _buttonIDBuffer = id;
    contextMenu->exec(this->mapToGlobal(QCursor::pos()));
}

// Пункт контексного меню (Просмотр изображения)
void ScreenshotHistory::ContextMenuView(){
    if(_screenshotViewer->IsVisible())
        return;

    _screenshotViewer->Show(emit GetImagesData()[_buttonIDBuffer].GetImage(), emit GetProgramSettings());

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
        bool result = emit GetImagesData()[_buttonIDBuffer].GetImage().save(&file, "PNG");

        if(result)
            emit ShowPopup("#HISTORY_IMAGE_SAVED#", "", 2000, "");
        else
            emit ShowPopup("#HISTORY_IMAGE_SAVE_ERROR#", "", 2000, "");
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

    _historyMaxSize = emit GetProgramSettings().Get_HistorySize();

    if(! (_historyImageButtonsNum == data.count() && !needUpdate) ){

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

                    QPixmap originalPixmap = data[iterator].GetImage();
                    button->setIcon(QIcon(originalPixmap.scaled(button->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
                    button->setIconSize(button->size());

                    iterator++;
                    gridLayout->addWidget(button, row, col);
                }
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

// Событие смены языка программы
void ScreenshotHistory::Event_ChangeLanguage(TranslateData data){
    _text_NoHIstory = data.translate("NO_HISTORY");
    _text_Dialog_HistoryClear = data.translate("CLEAR_HISTORY");

    contextMenu->actions().at(0)->setText(data.translate("CONTEXT_MENU_VIEW"));
    contextMenu->actions().at(1)->setText(data.translate("CONTEXT_MENU_WINDOW"));
    contextMenu->actions().at(3)->setText(data.translate("CONTEXT_MENU_SAVE"));
    contextMenu->actions().at(4)->setText(data.translate("CONTEXT_MENU_DELETE_IMAGE"));

    repaint();
}
