#include "settingsform.h"
#include "ui_settingsform.h"

SettingsForm::SettingsForm(QWidget *parent) :QWidget(parent), ui(new Ui::SettingsForm){
    ui->setupUi(this);

    _parent = parent;

    setMouseTracking(true);
    ui->BarWidget->setMouseTracking(true);
    ui->MainWidget->setMouseTracking(true);
    ui->Page_Sounds->setMouseTracking(true);
    ui->Page_General->setMouseTracking(true);
    ui->Page_Capture->setMouseTracking(true);
    ui->Page_History->setMouseTracking(true);
    ui->Widget_Language->setMouseTracking(true);

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::ToolTip);

    ui->MainWidget->setCurrentIndex(0);
    ui->prtsc_timeout_slider->setPageStep(1);
    ui->prtsc_timeout_slider->setSingleStep(1);
    ui->prtsc_timeout_slider->setRange(200, 1000);
}

QComboBox* SettingsForm::GetLanguageBox(){
    return ui->combobox_language;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SettingsForm::IsVisible(){
    return _isVisible;
}

void SettingsForm::Show(){
    show();
    _isVisible = true;

    UpdateSettings(emit GetProgramSettings());
    _animationManager.Create_WindowOpacity(this, nullptr, 100, 0, 1).Start();
}

void SettingsForm::Hide(){
    _isVisible = false;

    _animationManager.Create_WindowOpacity(this, [this](){ hide(); }, 100, 1, 0).Start();
}

void SettingsForm::UpdateSettings(ProgramSetting settings){
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    ui->checkBox_autoRun->blockSignals(true);
    ui->checkBox_rectSize->blockSignals(true);
    ui->checkBox_stopFrame->blockSignals(true);
    ui->checkBox_soundsUse->blockSignals(true);
    ui->checkBox_showShortcut->blockSignals(true);
    ui->checkBox_smoothScroll->blockSignals(true);
    ui->checkBox_startupMessages->blockSignals(true);
    ui->checkBox_viewer_showpercent->blockSignals(true);
    ui->checkBox_screenshotModificationArea->blockSignals(true);

    ui->prtsc_timeout_slider->blockSignals(true);
    ui->volume_delete_file_slider->blockSignals(true);
    ui->volume_notification_slider->blockSignals(true);
    ui->volume_button_click_slider->blockSignals(true);
    ui->volume_history_clear_slider->blockSignals(true);
    ui->volume_make_screenshot_slider->blockSignals(true);
    ui->volume_button_hover_sound_slider->blockSignals(true);
    ui->volume_clipboard_image_get_slider->blockSignals(true);

    ui->history_size_lineEdit->blockSignals(true);

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    ui->checkBox_autoRun->setChecked(settings.Get_Startup());
    ui->checkBox_soundsUse->setChecked(settings.Get_UseSound());
    ui->checkBox_stopFrame->setChecked(settings.Get_StopFrame());
    ui->checkBox_showShortcut->setChecked(settings.Get_ShowShortcut());
    ui->checkBox_startupMessages->setChecked(settings.Get_StartupMessage());
    ui->checkBox_smoothScroll->setChecked(settings.Get_HistorySmoothScroll());
    ui->checkBox_rectSize->setChecked(settings.Get_ShowScreenshotZoneGeometry());
    ui->checkBox_viewer_showpercent->setChecked(settings.Get_ViewerShowPercent());
    ui->checkBox_screenshotModificationArea->setChecked(settings.Get_ShowModificationArea());

    ui->prtsc_timeout_slider->setValue(settings.Get_PrtSc_Timeout());
    ui->prtsc_timeout_spinbox->setValue(settings.Get_PrtSc_Timeout());

    ui->volume_delete_file_slider->setValue(settings.Get_VolumeDeleteFile());
    ui->volume_delete_file_spinbox->setValue(settings.Get_VolumeDeleteFile());

    ui->volume_notification_slider->setValue(settings.Get_VolumeNotification());
    ui->volume_notification_spinbox->setValue(settings.Get_VolumeNotification());

    ui->volume_history_clear_slider->setValue(settings.Get_VolumeHistoryClear());
    ui->volume_history_clear_spinbox->setValue(settings.Get_VolumeHistoryClear());

    ui->volume_make_screenshot_slider->setValue(settings.Get_VolumeMakeScreenshot());
    ui->volume_make_screenshot_spinbox->setValue(settings.Get_VolumeMakeScreenshot());

    ui->volume_button_hover_sound_slider->setValue(settings.Get_VolumeButtonHover());
    ui->volume_button_hover_sound_spinbox->setValue(settings.Get_VolumeButtonHover());

    ui->volume_button_click_slider->setValue(settings.Get_VolumeButtonClick());
    ui->volume_button_click_spinbox->setValue(settings.Get_VolumeButtonClick());

    ui->volume_clipboard_image_get_slider->setValue(settings.Get_VolumeClipboardGetImage());
    ui->volume_clipboard_image_get_spinbox->setValue(settings.Get_VolumeClipboardGetImage());

    ////
    ui->history_size_lineEdit->setText(QString::number(settings.Get_HistorySize()));

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    ui->checkBox_autoRun->blockSignals(false);
    ui->checkBox_rectSize->blockSignals(false);
    ui->checkBox_soundsUse->blockSignals(false);
    ui->checkBox_stopFrame->blockSignals(false);
    ui->checkBox_showShortcut->blockSignals(false);
    ui->checkBox_startupMessages->blockSignals(false);
    ui->checkBox_viewer_showpercent->blockSignals(false);
    ui->checkBox_screenshotModificationArea->blockSignals(false);

    ui->prtsc_timeout_slider->blockSignals(false);
    ui->checkBox_smoothScroll->blockSignals(false);
    ui->volume_delete_file_slider->blockSignals(false);
    ui->volume_button_click_slider->blockSignals(false);
    ui->volume_notification_slider->blockSignals(false);
    ui->volume_history_clear_slider->blockSignals(false);
    ui->volume_make_screenshot_slider->blockSignals(false);
    ui->volume_button_hover_sound_slider->blockSignals(false);
    ui->volume_clipboard_image_get_slider->blockSignals(false);

    ui->history_size_lineEdit->blockSignals(false);

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    // Устанавливаем позицию и размер окна с сохраненных настроек
    setGeometry(settings.Get_SettingsWindowGeometry());
}

// Смена страницы в Stucked Widget
void SettingsForm::Change_StuckWidget_Page(int index){
    if(ui->MainWidget->currentIndex() == index)
        return;

    _animationManager.Create_StackedWidgetOpacity(ui->MainWidget->currentWidget(), [this, index](){

                             ui->MainWidget->setCurrentIndex(index);
                             _animationManager.Create_StackedWidgetOpacity(ui->MainWidget->currentWidget(), nullptr, 150, 0, 1).start();

                         }, 150, 1, 0).start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsForm::paintEvent(QPaintEvent *){
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setBrush(QBrush(QColor(40, 40, 50, 200))); // Задаем цвет фона
    paint.drawRoundedRect(rect(), 30, 30); // Задаем радиус закругления
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Включение/Отключение автозапуска программы при старте системы
void SettingsForm::on_checkBox_autoRun_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();

    settings.Set_Startup(checked);

    #ifdef Q_OS_WIN32
        QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

        if(checked){
            reg.setValue(APPLICATION_NAME, QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
            reg.sync();
        }else{
            reg.remove(APPLICATION_NAME);
        }
    #endif

    emit ChangeProgramSettings(settings);
}

// Включаем и отключаем стартовые сообщения при запуске программы
void SettingsForm::on_checkBox_startupMessages_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();

    settings.Set_StartupMessage(checked);

    emit ChangeProgramSettings(settings);
}

// Включаем и отключаем отображение сочетания клавишь для операций в истории
void SettingsForm::on_checkBox_showShortcut_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();

    settings.Set_ShowShortcut(checked);

    emit ChangeProgramSettings(settings);

    _parent->repaint();
}

// Включаем или отключаем плавную прокрутку истории
void SettingsForm::on_checkBox_smoothScroll_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();

    settings.Set_HistorySmoothScroll(checked);

    emit ChangeProgramSettings(settings);
}

// Включение/Отключение отображение размера и позиции при редактировании зоны создания скриншота
void SettingsForm::on_checkBox_rectSize_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ShowScreenshotZoneGeometry(checked);
    emit ChangeProgramSettings(settings);
}

// Включение/Отключение использование стоп-кадра при создании скриншота
void SettingsForm::on_checkBox_stopFrame_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_StopFrame(checked);
    emit ChangeProgramSettings(settings);
}

// Включение/Отключение отображение процента масштабирования изображения в просмотрщике
void SettingsForm::on_checkBox_viewer_showpercent_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ViewerShowPercent(checked);
    emit ChangeProgramSettings(settings);
}

// Включение/Отключение возможности модифицировать зону создания скриншота
void SettingsForm::on_checkBox_screenshotModificationArea_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ShowModificationArea(checked);
    emit ChangeProgramSettings(settings);
}

// Включение/Отключение использования языка
void SettingsForm::on_checkBox_soundsUse_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_UseSound(checked);
    emit ChangeProgramSettings(settings);
}

// Изменение языка программы
void SettingsForm::on_combobox_language_currentTextChanged(const QString &arg){
    emit ChangeProgramLanguage(arg);

    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ProgramLanguage(arg);
    emit ChangeProgramSettings(settings);
}

// Установка размера истории
void SettingsForm::on_history_size_lineEdit_editingFinished(){
    ProgramSetting settings = emit GetProgramSettings();

    int size = ui->history_size_lineEdit->text().toInt();

    if(size < 1)
        size = 1;
    else if(size > HISTORY_IMAGE_MAX_SIZE)
        size = HISTORY_IMAGE_MAX_SIZE;

    settings.Set_HistorySize(size);
    ui->history_size_lineEdit->setText(QString::number(size));

    emit ChangeProgramSettings(settings);
}

// Нажатия на кнопки пунктов в меню настроек
void SettingsForm::on_btn_general_released(){ emit PlaySound(SOUND_TYPE_BUTTON_CLICK); Change_StuckWidget_Page(0); }
void SettingsForm::on_btn_sounds_released(){ emit PlaySound(SOUND_TYPE_BUTTON_CLICK); Change_StuckWidget_Page(1); }
void SettingsForm::on_btn_history_released(){ emit PlaySound(SOUND_TYPE_BUTTON_CLICK); Change_StuckWidget_Page(2); }
void SettingsForm::on_btn_capture_released(){ emit PlaySound(SOUND_TYPE_BUTTON_CLICK); Change_StuckWidget_Page(3); }

// Тестирование звуков с текущими настройками громкости в меню настроек
void SettingsForm::on_button_notification_sound_test_released(){ emit PlaySound(SOUND_TYPE_POPUP); }
void SettingsForm::on_button_delete_file_sound_test_released(){ emit PlaySound(SOUND_TYPE_REMOVE_FILE); }
void SettingsForm::on_button_button_click_sound_test_released(){ emit PlaySound(SOUND_TYPE_BUTTON_CLICK); }
void SettingsForm::on_button_button_hover_sound_test_released(){ emit PlaySound(SOUND_TYPE_BUTTON_HOVER); }
void SettingsForm::on_button_make_screenshot_sound_test_released(){ emit PlaySound(SOUND_TYPE_PRINTSCREEN); }
void SettingsForm::on_button_history_clear_sound_test_released(){ emit PlaySound(SOUND_TYPE_RECYCLE_CLEANING); }
void SettingsForm::on_button_clipboard_image_get_sound_test_released(){ emit PlaySound(SOUND_TYPE_GET_CLIPBOARD_IMAGE); }

// Изменение значений слайдеров
void SettingsForm::on_prtsc_timeout_slider_valueChanged(int value){ ui->prtsc_timeout_spinbox->setValue(value); }
void SettingsForm::on_volume_delete_file_slider_valueChanged(int value){ ui->volume_delete_file_spinbox->setValue(value); }
void SettingsForm::on_volume_notification_slider_valueChanged(int value){ ui->volume_notification_spinbox->setValue(value); }
void SettingsForm::on_volume_button_click_slider_valueChanged(int value){ ui->volume_button_click_spinbox->setValue(value); }
void SettingsForm::on_volume_history_clear_slider_valueChanged(int value){ ui->volume_history_clear_spinbox->setValue(value); }
void SettingsForm::on_volume_make_screenshot_slider_valueChanged(int value){ ui->volume_make_screenshot_spinbox->setValue(value); }
void SettingsForm::on_volume_button_hover_sound_slider_valueChanged(int value){ ui->volume_button_hover_sound_spinbox->setValue(value); }
void SettingsForm::on_volume_clipboard_image_get_slider_valueChanged(int value){ ui->volume_clipboard_image_get_spinbox->setValue(value); }

// Изменение значения слайдера отвечающего за таймаут двойного нажатия кнопки PrtSc
void SettingsForm::on_prtsc_timeout_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_PrtSc_Timeout(ui->prtsc_timeout_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука уведомления
void SettingsForm::on_volume_notification_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeNotification(ui->volume_notification_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука очистки истории
void SettingsForm::on_volume_history_clear_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeHistoryClear(ui->volume_history_clear_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука удаления файла из истории
void SettingsForm::on_volume_delete_file_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeDeleteFile(ui->volume_delete_file_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука создания скриншота
void SettingsForm::on_volume_make_screenshot_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeMakeScreenshot(ui->volume_make_screenshot_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука наведения на кнопки
void SettingsForm::on_volume_button_hover_sound_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeButtonHover(ui->volume_button_hover_sound_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука нажатия на кнопки
void SettingsForm::on_volume_button_click_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeButtonClick(ui->volume_button_click_slider->value());
    emit ChangeProgramSettings(settings);
}

// Изменение значения слайдера отвечающего за громкость звука при добавлении изображения из буфера обмена
void SettingsForm::on_volume_clipboard_image_get_slider_sliderReleased(){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_VolumeClipboardGetImage(ui->volume_clipboard_image_get_slider->value());
    emit ChangeProgramSettings(settings);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////[-------Событие перетаскивания--------]//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsForm::mousePressEvent(QMouseEvent *pe){
    mousePressPosition = pe->pos();
    mousePressGlobalPosition = pe->globalPosition();
    mousePressDiffFromBorder.setWidth(width() - pe->position().x());
    mousePressDiffFromBorder.setHeight(height() - pe->position().y());

    if (pe->buttons() == Qt::LeftButton){
        if ((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth){
            _action = ACTION_RESIZE_RIGHT_DOWN;
            setCursorOnAll(Qt::SizeFDiagCursor);
        }else{
            _action = ACTION_MOVE;
            setCursorOnAll(Qt::ClosedHandCursor);
        }
    }else{ _action = ACTION_NONE; }

    repaint();
}

void SettingsForm::mouseMoveEvent(QMouseEvent *pe){
    QRect screen = QGuiApplication::primaryScreen()->geometry();

    mouseMovePosition = pe->pos();

    if(_action == ACTION_NONE)
        checkAndSetCursors(pe);

    else if(_action == ACTION_MOVE){

        QPointF moveHere;
        moveHere = pe->globalPosition() - mousePressPosition;
        QRect newRect = QRect(moveHere.toPoint(), geometry().size());
        bool snapped;

        snapped = snapEdgeToScreenOrClosestFellow(
            newRect, screen,
            [](QRect& r, int v) { r.moveLeft(v); },
            [](const QRect& r) { return r.left(); },
            [](const QRect& r) { return r.right() + 1; });
        if(!snapped){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) { r.moveRight(v); },
                [](const QRect& r) { return r.right(); },
                [](const QRect& r) { return r.left() - 1; }
                );
        }

        snapped = snapEdgeToScreenOrClosestFellow(
            newRect, screen,
            [](QRect& r, int v) { r.moveTop(v); },
            [](const QRect& r) { return r.top(); },
            [](const QRect& r) { return r.bottom() + 1; }
            );

        if(!snapped){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) { r.moveBottom(v); },
                [](const QRect& r) { return r.bottom(); },
                [](const QRect& r) { return r.top() - 1; }
                );
        }

        move(newRect.topLeft());
    }else{
        QRect newRect = resizeAccordingly(pe);

        if(_action == ACTION_RESIZE_RIGHT_DOWN){
            snapEdgeToScreenOrClosestFellow(
                newRect, screen,
                [](QRect& r, int v) {	r.setBottom(v); },
                [](const QRect& r) { return r.bottom(); },
                [](const QRect& r) { return r.top() - 1; }
                );
        }

        if(newRect.size() != geometry().size())
            this->resize(newRect.size());

        if(newRect.topLeft() != geometry().topLeft())
            this->move(newRect.topLeft());
    }
}

void SettingsForm::mouseReleaseEvent(QMouseEvent *pe){
    _action = ACTION_NONE;

    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_SettingsWindowGeometry(geometry());
    emit ChangeProgramSettings(settings);

    checkAndSetCursors(pe);
    repaint();
}

void SettingsForm::setCursorOnAll(Qt::CursorShape cursor){
    setCursor(cursor);
}

QRect SettingsForm::resizeAccordingly(QMouseEvent *pe){
    int newWidth = width();
    int newHeight = height();
    int newX = x();
    int newY = y();

    switch(_action){
        case ACTION_MOVE:
            break;


        case ACTION_RESIZE_RIGHT_DOWN:
            newWidth = pe->position().x() + mousePressDiffFromBorder.width();
            newHeight = pe->position().y() + mousePressDiffFromBorder.height();
            newWidth = (newWidth <= FormMinimumWidth) ? FormMinimumWidth : newWidth;
            newHeight = (newHeight <= FormMinimumHeight) ? FormMinimumHeight : newHeight;

            break;

        default: break;
    }

    return QRect(newX, newY, newWidth, newHeight);
}

void SettingsForm::checkAndSetCursors(QMouseEvent *pe){
    if ((height() - pe->position().y()) < FormBorderWidth && (width() - pe->position().x()) < FormBorderWidth)
        setCursorOnAll(Qt::SizeFDiagCursor);
    else
        setCursorOnAll(Qt::ArrowCursor);
}

// Эффект магнита к краям экрана и других окон
bool SettingsForm::snapEdgeToScreenOrClosestFellow(
    QRect& newRect,
    const QRect& screen,
    std::function<void(QRect&, int)> setter,
    std::function<int(const QRect&)> getter,
    std::function<int(const QRect&)>){

    if(abs(getter(newRect) - getter(screen)) < FORM_STIKY_CLOSER_PIXELS){
        setter(newRect, getter(screen));
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsForm::Event_ChangeLanguage(TranslateData data){
    ui->btn_sounds->setText(data.translate("SOUNDS_BUTTON"));
    ui->btn_general->setText(data.translate("GENERAL_BUTTON"));
    ui->btn_history->setText(data.translate("HISTORY_BUTTON"));
    ui->btn_capture->setText(data.translate("CAPTURE_BUTTON"));

    ui->settings_label->setText(data.translate("SETTINGS_LABEL"));
    ui->prtsc_timeout_label->setText(data.translate("PRTSC_TIMEOUT_LABEL"));
    ui->language_label->setText("[" + data.translate("LANGUAGE_LABEL") + "]");
    ui->delete_file_volume_label->setText(data.translate("DELETEFILE_VOLUME_LABEL"));
    ui->button_click_volume_label->setText(data.translate("BUTTON_CLICK_VOLUME_LABEL"));
    ui->notification_volume_label->setText(data.translate("NOTIFICATION_VOLUME_LABEL"));
    ui->history_clear_volume_label->setText(data.translate("HISTORYCLEAR_VOLUME_LABEL"));
    ui->make_screenshot_volume_label->setText(data.translate("MAKESCREENSHOT_VOLUME_LABEL"));
    ui->button_hover_sound_volume_label->setText(data.translate("BUTTON_HOVER_VOLUME_LABEL"));
    ui->clipboard_image_get_volume_label->setText(data.translate("CLIPBOARD_IMAGE_ADD_VOLUME_LABEL"));

    ui->checkBox_autoRun->setText(data.translate("CHECKBOX_AUTORUN"));
    ui->checkBox_rectSize->setText(data.translate("CHECKBOX_RECT_SIZE"));
    ui->checkBox_soundsUse->setText(data.translate("CHECKBOX_SOUNDS_USE"));
    ui->checkBox_stopFrame->setText(data.translate("CHECKBOX_STOP_FRAME"));
    ui->checkBox_showShortcut->setText(data.translate("CHECKBOX_SHOW_SHORTCUT"));
    ui->checkBox_startupMessages->setText(data.translate("CHECKBOX_STARTUP_MESSAGE"));
    ui->checkBox_viewer_showpercent->setText(data.translate("CHECKBOX_VIEW_PERCENT"));
    ui->checkBox_smoothScroll->setText(data.translate("CHECKBOX_HISTORY_SMOOTH_SCROLL"));
    ui->checkBox_screenshotModificationArea->setText(data.translate("CHECKBOX_SCREENSHOT_MODIFICATION_AREA"));

    ui->history_size_label->setText(data.translate("LINE_EDIT_HISTORY_SIZE"));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////[-------Конец--------]/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
