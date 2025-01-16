#include "screenshothistory.h"

// Конструктор
ScreenshotHistory::ScreenshotHistory(QWidget *parent): QWidget{ parent }{
    setParent(parent);

    _buttonsHoverTimer = new QTimer();
    _settingsMenu = new SettingsForm(this);
    _screenshotViewer = new ScreenshotHistoryViewer(this);

    // Атрибуты окна
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    // Подключение сигналов
    connect(this, SIGNAL(GetCurrentScreenGeometry()), parent, SLOT(GetCurrentScreenGeometry()));

    connect(_screenshotViewer, SIGNAL(GetCurrentScreenGeometry()), parent, SLOT(GetCurrentScreenGeometry()));
    connect(_buttonsHoverTimer, &QTimer::timeout, this, &ScreenshotHistory::updateCursorPositionTimer);

    connect(_settingsMenu, SIGNAL(GetProgramSettings()), parent, SLOT(GetProgramSettings()));
    connect(_settingsMenu, SIGNAL(ChangeProgramLanguage(QString)), parent, SLOT(ChangeProgramLanguage(QString)));
    connect(_settingsMenu, SIGNAL(ChangeProgramSettings(ProgramSetting)), parent, SLOT(ChangeProgramSettings(ProgramSetting)));

    // Создаем элементы окна истории
    CreateHistoryUI();
    CreateContexMenu();

    // Получаем размер екрана
    QRect screenRect = emit GetCurrentScreenGeometry();

    // Указываем зоны в которых будут нарисованные кнопки
    _settingButton = new QRect(15, 15, 25, 25);
    _exitButton = new QRect(screenRect.width() - 15 - 30, 15, 30, 30);
    _clearHistoryButton = new QRect(15, _settingButton->y() + _settingButton->height() + 10, 25, 25);
    _uploadImageButton = new QRect(15, _clearHistoryButton->y() + _clearHistoryButton->height() + 10, 25, 25);
    _unloadImageButton = new QRect(15, _uploadImageButton->y() + _uploadImageButton->height() + 10, 25, 25);

    // Добавляем сочетания клавишь (Просто для визуального отображения)
    _showShortcuts.append(new Shortcut("Esc", "Смысла описывать нету, переводчик переведёт"));
    _showShortcuts.append(new Shortcut("x2 PrtSc", "Смысла описывать нету, переводчик переведёт"));
    _showShortcuts.append(new Shortcut("Shift + PrtSc", "Смысла описывать нету, переводчик переведёт"));
}

SettingsForm*ScreenshotHistory::GetSettingsForm() const { return _settingsMenu; }

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void ScreenshotHistory::CheckSettingsWindow(ProgramSetting settings){

    // Устанавливаем позицию окна настроек программы (если настройки сбились)
    if(settings.Get_SettingsWindowGeometry().x() + settings.Get_SettingsWindowGeometry().y() + settings.Get_SettingsWindowGeometry().width() + settings.Get_SettingsWindowGeometry().height() <= 10){

        settings.Set_SettingsWindowGeometry(QRect(_settingButton->x() + _settingButton->width() + 7, _settingButton->y(), 500, 300));

        emit ChangeProgramSettings(settings);
    }
}

// Отображение истории
void ScreenshotHistory::Show(){
    emit ProcessEvent_ShowStart();

    // Устанавливаем разрешение согласно разрешения экрана
    setGeometry(emit GetCurrentScreenGeometry());

    _buttonsHoverTimer->start(1);

    UpdateHistoryGrid(emit GetImagesData());

    show();

    // Анимация плавного появления
    _animationManager.Create_WindowOpacity(this,
        [this](){

            emit ProcessEvent_ShowEnd(this);

    }, 100, 0, 1).Start();

    activateWindow();
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
    if(_operationBlock)
        return false;

    // Закрываем настройки если они были открыты на момент закрытия истории
    if(_settingsMenu->IsVisible())
        _settingsMenu->Hide();

    _buttonsHoverTimer->stop();

    emit ProcessEvent_HideStart();

    // Анимация плавного закрытия окна
    _animationManager.Create_WindowOpacity(this,
        [this](){

            hide();
            emit ProcessEvent_HideEnd();

    }, 100, 1, 0).Start();

    return true;
}

// Тут запрос на переключение текущего просматриваемого изображения
void ScreenshotHistory::SwitchingViewedImages(bool dir){
    if(!_screenshotViewer->IsVisible())
        return;

    int count = emit GetImagesData().count();

    if( dir && (_historyImageSwitchingIndex + 1 < count) ){
        _historyImageSwitchingIndex++;

        _screenshotViewer->ChangePixmap(emit GetImagesData()[_historyImageSwitchingIndex].GetPixmap());
    }else if( !dir && (_historyImageSwitchingIndex - 1 >= 0) ){
        _historyImageSwitchingIndex--;

        _screenshotViewer->ChangePixmap(emit GetImagesData()[_historyImageSwitchingIndex].GetPixmap());
    }
}

// Событие отрисовки окна
void ScreenshotHistory::paintEvent(QPaintEvent *){
    QPainter paint(this);

    QFont font(QFont().defaultFamily(), 100);

    paint.setPen(QPen(Qt::green, 2, Qt::SolidLine, Qt::SquareCap));
    paint.setBrush(QBrush(QColor(0, 0, 0, 175)));
    paint.drawRect(0, 0, width(), height());

    // Рисуем кнопки
    QImage imagePath;

    // Настройки
    imagePath = QImage(":/Buttons/Resourse/Buttons/Settings" + QString(_settingButtonIsHover ? "_Hover.png" : ".png"));
    paint.drawImage(*_settingButton, imagePath);

    // Закрытие программы
    imagePath = QImage(":/Buttons/Resourse/Buttons/Close" + QString(_exitButtonIsHover ? "_Hover.png" : ".png"));
    paint.drawImage(*_exitButton, imagePath);

    // Очистка истории
    imagePath = QImage(":/Buttons/Resourse/Buttons/Clear" + QString(_clearHistoryButtonIsHover ? "_Hover.png" : ".png"));
    paint.drawImage(*_clearHistoryButton, imagePath);

    // Загрузка изображений
    imagePath = QImage(":/Buttons/Resourse/Buttons/Upload" + QString(_uploadImageButtonIsHover ? "_Hover.png" : ".png"));
    paint.drawImage(*_uploadImageButton, imagePath);

    // Выгрузка изображений
    imagePath = QImage(":/Buttons/Resourse/Buttons/Unload" + QString(_unloadImageButtonIsHover ? "_Hover.png" : ".png"));
    paint.drawImage(*_unloadImageButton, imagePath);

    // Если история пуста, выводим сообщение
    if((emit GetImagesData()).isEmpty()){
        // Устанавливаем стиль текста
        paint.setFont(font);

        // Устанавливаем цвет текста
        paint.setPen(QColor(255, 255, 255));

        // Рассчитываем позицию для центрирования текста
        int textWidth = paint.fontMetrics().horizontalAdvance(_text_NoHIstory);
        int textHeight = paint.fontMetrics().height();

        // Рисуем текст на экране
        paint.drawText((width() - textWidth) / 2, (height() / 2) + (textHeight / 2.5), _text_NoHIstory);
    }

    font = QFont(QFont().defaultFamily(), 12);

    // Отображаем текущую версию программы
    paint.setFont(font);
    paint.setPen(QColor(255, 255, 255, 50));
    paint.drawText(rect().adjusted(0, 0, 0, -10), Qt::AlignHCenter | Qt::AlignBottom, APPLICATION_VERSION);
    paint.drawText(rect().adjusted(0, 10, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "[" + QString::number(_historyImageButtonsNum) + " / " + QString::number(_historyMaxSize) + "]");


    // Отображаем Shortcut
    if((emit GetProgramSettings()).Get_ShowShortcut() && _tooltipRect){
        paint.setFont(font);
        paint.setBrush(QBrush(QColor(0, 0, 0, 150)));
        paint.drawRect(*_tooltipRect);

        int startY = _tooltipRect->y() + 10;

        for(Shortcut *shortcut : _showShortcuts){
            int startX = _tooltipRect->x() + 10;

            QStringList keys = shortcut->GetKey().split(" + ");

            for(int keys_iterator = 0; keys_iterator < keys.count(); keys_iterator++){

                if(keys_iterator > 0){
                    QRect keyRect = PaintEventAddKeyFonction(startX, startY, "+", QColor(255, 255, 255, 75), paint);
                    startX += keyRect.width() + 5;
                }

                QRect keyRect = PaintEventAddKeyFonction(startX, startY, keys.at(keys_iterator), QColor(255, 255, 255, 75), paint);
                startX += keyRect.width() + 5;
            }

            QFontMetrics fontMetrics(font);
            paint.drawText(startX, startY + fontMetrics.ascent(), shortcut->GetDescription());
            startY += fontMetrics.height() + 5;
        }
    }

    paint.end();
}

void ScreenshotHistory::mouseReleaseEvent(QMouseEvent *event){
    if(_screenshotViewer->IsVisible() || _operationBlock)
        return;

    // Нажата кнопка настроек
    if(_settingButton->contains(event->pos())){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        if(_settingsMenu->IsVisible())
            _settingsMenu->Hide();
        else
            _settingsMenu->Show();

        // Нажата кнопка закрытия приложения
    }else if(_exitButton->contains(event->pos())){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        _animationManager.Create_WindowOpacity(this, [this](){ emit ClosingRequest(); }, 100, 1, 0);
        _animationManager.Start();

        // Нажата кнопка очистки истории
    }else if(_clearHistoryButton->contains(event->pos())){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        // Создаем кастомный класс диалогового окна с запросом
        CustomDecisionDialog _customDialog(this, _text_Dialog_HistoryClear, QUrl::fromLocalFile(":/Icons/Resourse/Warning.png"));

        _operationBlock = true;

        if(_customDialog.exec() == QDialog::Accepted){
            emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);
            _operationBlock = false;

            // Удаляем все файлы
            emit HistoryRemoveAllItem();

            // Обновляем сетку
            UpdateHistoryGrid(emit GetImagesData());
        }else _operationBlock = false;

        // Нажата кнопка добавления изображения
    }else if(_uploadImageButton->contains(event->pos())){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        // Получаем все поддерживаемые форматы изображений
        QStringList supportedFormats;

        for(const QByteArray &format : QImageReader::supportedImageFormats())
            supportedFormats.append("*." + QString(format));

        _operationBlock = true;

        // Создаем вопросительное окно выбора изображений
        QStringList files = QFileDialog::getOpenFileNames(this, "Select an image", "", "Images (" + supportedFormats.join(" ") + ");; All files (*)");

        _operationBlock = false;

        // Загрузка отменена
        if (files.count() <= 0){
            emit ShowPopup("#IMAGE_LOAD_CANCELED#", "", "", "", 2000);

            return;
        }

        _operationBlock = true;

        CustomImageImportExportDialog *_importDialog = new CustomImageImportExportDialog(this);

        _importDialog->StartImageImport(files,
            [this](int iteration, SaveManagerFileData imageData){
                emit ImportImageToDataList(iteration, imageData);
            },

            [this](int imported){
                _operationBlock = false;

                if(imported > 0 && emit ImportOperationEnd() == true){
                    emit ShowPopup("#IMAGE_LOADED1#: " + QString::number(imported) + " #IMAGE_LOADED2#", "", "", "", 3500);

                    // Обновляем сетку
                    UpdateHistoryGrid(emit GetImagesData());

                    // Сохраняем изменения
                    emit SaveHistory();
                }
        });

        // Нажата кнопка выгрузки изображений
    }else if(_unloadImageButton->contains(event->pos())){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        _operationBlock = true;

        QString folder = QFileDialog::getExistingDirectory(this, "Select a folder", QDir::homePath());

        _operationBlock = false;

        if(!folder.isEmpty()){

            _operationBlock = true;

            CustomImageImportExportDialog *_exportDialog = new CustomImageImportExportDialog(this);

            _exportDialog->StartImageExport(folder, emit GetImagesData(),
                [this](int exported){
                    _operationBlock = false;

                    if(exported > 0)
                        emit ShowPopup("#UNLOAD_IMAGES_DONE# [" + QString::number(exported) + "]", "", "", "", 3500);

            });

        }else
            emit ShowPopup("#UNLOAD_IMAGES_FOLDER_ERROR#", "", "", "", 3500);
    }
}

// Событие получения позиции курсора (для визуального создания эффекта Hover на нарисованных кнопках)
void ScreenshotHistory::updateCursorPositionTimer(){
    bool hover = false;

    hover = _settingButton->contains(QCursor::pos());

    // Настройки
    if(hover && !_settingButtonIsHover){

        _settingButtonIsHover = true;
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_HOVER);
        repaint();

    }else if(!hover && _settingButtonIsHover){

        _settingButtonIsHover = false;
        repaint();

    }

    hover = _exitButton->contains(QCursor::pos());

    // Выход
    if(hover && !_exitButtonIsHover){

        _exitButtonIsHover = true;
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_HOVER);
        repaint();

    }else if(!hover && _exitButtonIsHover){

        _exitButtonIsHover = false;
        repaint();

    }

    hover = _clearHistoryButton->contains(QCursor::pos());

    // Очистка
    if(hover && !_clearHistoryButtonIsHover){

        _clearHistoryButtonIsHover = true;
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_HOVER);
        repaint();

    }else if(!hover && _clearHistoryButtonIsHover){

        _clearHistoryButtonIsHover = false;
        repaint();

    }

    hover = _uploadImageButton->contains(QCursor::pos());

    // Загрузка изображения
    if(hover && !_uploadImageButtonIsHover){

        _uploadImageButtonIsHover = true;
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_HOVER);
        repaint();

    }else if(!hover && _uploadImageButtonIsHover){

        _uploadImageButtonIsHover = false;
        repaint();

    }

    hover = _unloadImageButton->contains(QCursor::pos());

    // Выгрузка изображения
    if(hover && !_unloadImageButtonIsHover){

        _unloadImageButtonIsHover = true;
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_HOVER);
        repaint();

    }else if(!hover && _unloadImageButtonIsHover){

        _unloadImageButtonIsHover = false;
        repaint();

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Левый клик по карточке истории с изображением
void ScreenshotHistory::onLeftClicked(CustomHistoryButton *button){
    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    int id = button->GetID();

    // Отсылаем изображение для вставки его в буфер обмена
    QPixmap image = emit GetImagesData()[id].GetPixmap();

    if(!image.isNull())
        emit ScreenshotProcess_CopyImageToClipboard(image);

    // Закрываем историю после копирования
    Hide();
}

// Средний клик по карточке истории с изображением
void ScreenshotHistory::onMiddleClicked(CustomHistoryButton *button){
    Q_UNUSED(button);

    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    //////// Code int id = button->GetID();
}

// Правый клик по карточке истории с изображением
void ScreenshotHistory::onRightClicked(CustomHistoryButton *button){
    if(_screenshotViewer->IsVisible() || _settingsMenu->IsVisible())
        return;

    _buttonIDBuffer = button->GetID();
    contextMenu->exec(this->mapToGlobal(QCursor::pos()));
}

// Пункт контексного меню (Просмотр изображения)
void ScreenshotHistory::ContextMenuView(){
    if(_screenshotViewer->IsVisible())
        return;

    _historyImageSwitchingIndex = _buttonIDBuffer;
    _screenshotViewer->Show(emit GetImagesData()[_buttonIDBuffer].GetPixmap(), emit GetProgramSettings());

    // Если на момент открытия окна просмотра скриншота было открыто окно настроек, делаем его временно не активным
    if(_settingsMenu->IsVisible())
        _settingsMenu->setEnabled(false);
}

// Пункт контексного меню (Закрепить изображение)
void ScreenshotHistory::ContextMenuWindow(){
    if(_screenshotViewer->IsVisible())
        return;

    emit CreateFloatingWindow(emit GetImagesData()[_buttonIDBuffer].GetPixmap());
}

// Пункт контексного меню (Отобразить изображение)
void ScreenshotHistory::ContextMenuShow(){

    // Изменяем параметр изображения и меняем атрибут отображения
    SaveManagerFileData data = emit GetImagesData()[_buttonIDBuffer];
    data.SetHidden(false);

    // Устанавливаем в список даннных
    emit ChangeImagesData(data, _buttonIDBuffer);

    // Обновляем сетку
    UpdateHistoryGrid(emit GetImagesData(), data);

    // Сохраняем изменения
    emit SaveHistory();
}

// Пункт контексного меню (Скрыть изображение)
void ScreenshotHistory::ContextMenuHide(){

    // Получаем данные изображения и меняем атрибут отображения
    SaveManagerFileData data = emit GetImagesData()[_buttonIDBuffer];
    data.SetHidden(true);

    // Устанавливаем в список даннных
    emit ChangeImagesData(data, _buttonIDBuffer);

    // Обновляем сетку
    UpdateHistoryGrid(emit GetImagesData(), data);

    // Сохраняем изменения
    emit SaveHistory();
}

// Пункт контексного меню (Повернуть изображение вправо)
void ScreenshotHistory::ContextMenuRotate(){

    // Получаем данные изображения
    SaveManagerFileData data = emit GetImagesData()[_buttonIDBuffer];

    // Создаем кастовный класс диалогового окна для вращения изображения
    CustomImageRotateDialog _customImageRotatedDialog(this);

    _customImageRotatedDialog.setPixmap(data.GetPixmap());

    // Показываем диалоговое окно
    _operationBlock = true;

    if(_customImageRotatedDialog.exec() == QDialog::Accepted){
        emit _settingsMenu->PlaySound(SOUND_TYPE_BUTTON_CLICK);

        _operationBlock = false;

        // Устанавливаем повёрнутое изображаение
        data.SetImage(RotatePixmap(data.GetImage(), _customImageRotatedDialog.getAngle()));

        // Устанавливаем в список даннных
        emit ChangeImagesData(data, _buttonIDBuffer);

        // Обновляем сетку
        UpdateHistoryGrid(emit GetImagesData(), data);

        // Сохраняем изменения
        emit SaveHistory();
    }else
        _operationBlock = false;
}

// Пункт контексного меню (Сохранить изображение)
void ScreenshotHistory::ContextMenuSave(){

    // Будем хранить данные прошлых итераций
    static int _tempSaveImageFormatIndex = 0;
    static QString _tempSaveImageFolderPath, _tempSaveImageFileName;

    // Создаем класс
    SaveImageCustomDialog saveDialog(this, _tempSaveImageFolderPath, _tempSaveImageFileName, _tempSaveImageFormatIndex);

    _operationBlock = true;

    // Вызываем его и ждем ответа
    if(saveDialog.exec() == QDialog::Accepted){
        _operationBlock = false;

        _tempSaveImageFileName = saveDialog.GetFileName();
        _tempSaveImageFolderPath = saveDialog.GetFolderPath();
        _tempSaveImageFormatIndex = saveDialog.GetImageFormatIndex();

        if(!_tempSaveImageFolderPath.isEmpty()){
            QString fileName = saveDialog.GetFileName();
            fileName.replace(" ", "_");

            // Проверяем существует ли уже подобный файл
            ////////////////////////////////////////////

            int counter = 1;
            QString fileNameFinal = _tempSaveImageFolderPath + "/" + fileName + saveDialog.GetImageFormatWithDot();

            while(QFile::exists(fileNameFinal)){
                fileNameFinal = _tempSaveImageFolderPath + "/" + fileName + "(" + QString::number(counter) + ")" + saveDialog.GetImageFormatWithDot();
                counter++;
            }

            ////////////////////////////////////////////
            // Проверяем существует ли уже подобный файл

            QFile file(fileNameFinal);
            bool result = emit GetImagesData()[_buttonIDBuffer].GetPixmap().save(&file);

            if(result)
                emit ShowPopup("#HISTORY_IMAGE_SAVED#", "", "", "", 2000);
            else
                emit ShowPopup("#HISTORY_IMAGE_SAVE_ERROR#", "", "", "", 2000);
        }
    }else
        _operationBlock = false;

}

// Пункт контексного меню (Удалить изображение)
void ScreenshotHistory::ContextMenuDeleteImage(){

    // Удаляем изображение из списка и удаляем файл
    emit HistoryRemoveItem(_buttonIDBuffer);

    // Обновляем сетку
    UpdateHistoryGrid(emit GetImagesData());
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Создаем визуальную сетку окна, на которой будут распологаться кнопки с изображениями
void ScreenshotHistory::CreateHistoryUI(){

    // Выделение памяти для классов
    QVBoxLayout *layout = new QVBoxLayout(this);
    CustomHistoryScrollArea *scrollArea = new CustomHistoryScrollArea(this);
    QWidget *gridWidget = new QWidget(scrollArea);
    gridLayout = new QGridLayout(gridWidget);

    // Просчитываем отступы между элементами и краями экрана
    int spacingTop = MapValueLong(emit GetCurrentScreenGeometry().height(), 0, ETALON_HEIGHT, 0, 125);
    int spacing = MapValueLong(emit GetCurrentScreenGeometry().width(), 0, ETALON_WIDTH, 0, 300);

    gridLayout->setVerticalSpacing(spacing); // Отступы по высоте
    gridLayout->setContentsMargins(0, spacingTop, 0, spacingTop); // Отступы [Лево] [Верх] [Право] [Низ]

    gridWidget->setLayout(gridLayout);
    scrollArea->setWidget(gridWidget);

    scrollArea->setFps(120);
    scrollArea->setDuration(50);

    scrollArea->setWidgetResizable(true);
    scrollArea->viewport()->setAttribute(Qt::WA_StaticContents);
    scrollArea->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background: transparent; border: none;");

    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    connect(scrollArea, SIGNAL(GetProgramSettings()), parent(), SLOT(GetProgramSettings()));

    layout->addWidget(scrollArea);
    setLayout(layout);
}

// Создаем контекстное меню на правую кнопку мыши
void ScreenshotHistory::CreateContexMenu(){
    contextMenu = new CustomCenteredQMenu(this);

    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuView()));
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuWindow()));
    contextMenu->addSeparator();
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuShow()));
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuHide()));
    contextMenu->addSeparator();
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuRotate()));
    contextMenu->addSeparator();
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuSave()));
    connect(contextMenu->addAction(""), SIGNAL(triggered()), this, SLOT(ContextMenuDeleteImage()));

    contextMenu->setStyleSheet(R"(
                                QMenu{
                                    color: rgb(255, 255, 255);
                                    background-color: rgba(64, 65, 66, 255);
                                    border: 0px solid rgba(64, 65, 66, 255);
                                }

                                QMenu::separator{
                                    height: 3px;
                                    background-color: rgba(99, 197, 255, 150);
                                    margin-left: 5px;
                                    margin-right: 5px;
                                }

                                QMenu::item{
                                    padding: 10px 10px 10px 10px;
                                    background-color: transparent;
                                }

                                QMenu::item:selected{
                                    color: rgb(255, 255, 255);
                                    background-color: rgba(86, 87, 88, 255);
                                }
    )");
}

void ScreenshotHistory::AddButtonToHistory(){

    // Создаем новую кнопку
    CustomHistoryButton* button = new CustomHistoryButton();

    // Получаем параметры размера кнопки
    int height = MapValueLong(emit GetCurrentScreenGeometry().height(), 0, ETALON_HEIGHT, 0, 250);
    int width = MapValueLong(emit GetCurrentScreenGeometry().width(), 0, ETALON_WIDTH, 0, 250);

    // Получаем позицию новой кнопки
    int rowIndex = (_historyImageButtonsNum / COLUMN_COUNT);
    int colIndex = (_historyImageButtonsNum % COLUMN_COUNT);

    // Настраиваем кнопку
    button->setFlat(true);
    button->setFixedSize(width, height);
    button->setStyleSheet(R"(
                            QPushButton{
                                color: rgb(255, 255, 255);
                                background-color: rgba(74, 94, 103,  100);
                                border-radius: 7px;
                                border-width: 0px;
                                border-style: solid;
                            }
    )");

    // Добавляем количество кнопок
    _historyImageButtonsNum++;

    // Подключаем сигналы кнопки
    connect(button, &CustomHistoryButton::clickedLeft, this, [this](CustomHistoryButton *button){ onLeftClicked(button); });
    connect(button, &CustomHistoryButton::clickedMiddle, this, [this](CustomHistoryButton *button){ onMiddleClicked(button); });
    connect(button, &CustomHistoryButton::clickedRight, this, [this](CustomHistoryButton *button){ onRightClicked(button); });

    // Добавляем новую кнопку на сетку истории
    gridLayout->addWidget(button, rowIndex, colIndex);
}

void ScreenshotHistory::RemoveButtonFromHistory(){

    // Получаем последнюю кнопку
    QLayoutItem* lastItem = gridLayout->takeAt(_historyImageButtonsNum - 1);

    if(lastItem){
        QWidget* widget = lastItem->widget();

        if(widget){

            // Получаем кнопку и отключаем сигналы
            CustomHistoryButton* button = qobject_cast<CustomHistoryButton*>(widget);

            if (button){
                disconnect(button, &CustomHistoryButton::clickedLeft, this, nullptr);
                disconnect(button, &CustomHistoryButton::clickedMiddle, this, nullptr);
                disconnect(button, &CustomHistoryButton::clickedRight, this, nullptr);
            }

            // Уменьшаем количество кнопок
            _historyImageButtonsNum--;

            delete widget;
        }

        delete lastItem;
    }
}

// Обновляем сетку истории с изображениями (если появились новые)
void ScreenshotHistory::UpdateHistoryGrid(QList<SaveManagerFileData> data, SaveManagerFileData modifiedDate){
    _historyMaxSize = GetProgramSettings().Get_HistorySize();

    bool resize = false;

    if(_historyImageButtonsNum > data.count()){

        // Удаляем лишние кнопки
        while(_historyImageButtonsNum > data.count())
            RemoveButtonFromHistory();

        resize = true;

    }else if(_historyImageButtonsNum < data.count()){

        // Добавляем недостающие кнопки
        while(_historyImageButtonsNum < data.count())
            AddButtonToHistory();

        resize = true;
    }

    // Если было изменение то изменяем ID и изображение на кнопках (В случае если изменение было только одной карточки то мы обновляем в ней изображение)
    ImageHistoryUpdate *updater = new ImageHistoryUpdate(this);

    if(resize || !modifiedDate.GetNull()){

        updater->StartButtonImageUpdate(gridLayout, _historyImageButtonsNum, data, modifiedDate, [this](){
            gridLayout->update();

            repaint();
        });
    }
}

QImage ScreenshotHistory::RotatePixmap(const QImage &image, qreal angle){

    QImage bufferImage = image;

    int imgHeight = bufferImage.height();
    int imgWidth = bufferImage.width();

    int centreY = imgHeight / 2;
    int centreX = imgWidth / 2;

    QTransform rotationMatrix;
    rotationMatrix.translate(centreX, centreY);
    rotationMatrix.rotate(angle);
    rotationMatrix.translate(-centreX, -centreY);

    QImage transformedImage = bufferImage.transformed(rotationMatrix, Qt::SmoothTransformation);

    bool needForCircumcision = false;
    int _xStart, _yStart, _xEnd = 0, _yEnd = 0;

    _xStart = transformedImage.width();
    _yStart = transformedImage.height();

    // Проходим по всем пикселям изображения
    for(int y = 0; y < transformedImage.height(); ++y){
        for(int x = 0; x < transformedImage.width(); ++x){

            if(transformedImage.pixelColor(x, y) != Qt::transparent){

                // Обновляем координаты зоны
                if (x < _xStart) _xStart = x;
                if (x > _xEnd) _xEnd = x;
                if (y < _yStart) _yStart = y;
                if (y > _yEnd) _yEnd = y;

                needForCircumcision = true;
            }

        }
    }

    // Копируем зону из изображения
    if(needForCircumcision)
        transformedImage = transformedImage.copy(QRect(_xStart, _yStart, _xEnd - _xStart + 1, _yEnd - _yStart + 1));

    return transformedImage;
}

// Добавляем стилистическую кнопку в историю для Tooltip
QRect ScreenshotHistory::PaintEventAddKeyFonction(int x, int y, QString key, QColor color, QPainter &paint) {
    QFont font(QFont().defaultFamily(), 12);
    QFontMetrics fontMetrics(font);
    QRect keyRect(x, y, fontMetrics.horizontalAdvance(key) + 10, fontMetrics.height());
    paint.setBrush(QColor(255, 255, 255, 25));
    paint.setPen(Qt::NoPen);
    paint.drawRoundedRect(keyRect, 3, 3);
    paint.setPen(color);
    paint.drawText(keyRect, Qt::AlignCenter, key);

    return keyRect;
}

// Маштабируем выходное число относительно диапазона входного
long ScreenshotHistory::MapValueLong(long x, long in_min, long in_max, long out_min, long out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Маштабируем выходное число относительно диапазона входного
float ScreenshotHistory::MapValueFloat(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Событие смены языка программы
void ScreenshotHistory::Event_ChangeLanguage(TranslateData data){
    _text_NoHIstory = data.translate("NO_HISTORY");
    _text_Dialog_HistoryClear = data.translate("CLEAR_HISTORY");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if((emit GetProgramSettings()).Get_ShowShortcut()){
        for(int i = 0; i < _showShortcuts.count(); i++)
            _showShortcuts.at(i)->SetDescription(data.translate("SHORTCUT" + QString::number(i) + "_DESCRIPTION"));

        QFont font(QFont().defaultFamily(), 12);
        QFontMetrics fontMetrics(font);

        // Получаем размер екрана
        QRect screenRect = emit GetCurrentScreenGeometry();

        int tooltipRectWidth = 0;
        int lineHeight = fontMetrics.height();
        int tooltipRectHeight = 0;

        for(Shortcut *shortcut : _showShortcuts){
            int keysWidth = 0;

            QStringList keys = shortcut->GetKey().split(" + ");

            for(const QString &key : keys)
                keysWidth += fontMetrics.horizontalAdvance(key) + 20;

            int width = keysWidth + fontMetrics.horizontalAdvance(shortcut->GetDescription()) + 20;

            if (width > tooltipRectWidth)
                tooltipRectWidth = width;

            tooltipRectHeight += lineHeight + 5;  // Высота строки и отступ
        }

        tooltipRectHeight += 20;  // Отступ сверху и снизу
        tooltipRectWidth += 20;  // Отступ справа и слева

        _tooltipRect = new QRect(screenRect.width() - tooltipRectWidth - 10, screenRect.height() - tooltipRectHeight - 10, tooltipRectWidth, tooltipRectHeight - 5);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    contextMenu->actions().at(0)->setText(data.translate("CONTEXT_MENU_VIEW"));
    contextMenu->actions().at(1)->setText(data.translate("CONTEXT_MENU_WINDOW"));
    // SEPARATOR (ID: 2)
    contextMenu->actions().at(3)->setText(data.translate("CONTEXT_MENU_SHOW"));
    contextMenu->actions().at(4)->setText(data.translate("CONTEXT_MENU_HIDE"));
    // SEPARATOR (ID: 5)
    contextMenu->actions().at(6)->setText(data.translate("CONTEXT_MENU_ROTATE"));
    // SEPARATOR (ID: 7)
    contextMenu->actions().at(8)->setText(data.translate("CONTEXT_MENU_SAVE"));
    contextMenu->actions().at(9)->setText(data.translate("CONTEXT_MENU_DELETE_IMAGE"));

    repaint();
}
