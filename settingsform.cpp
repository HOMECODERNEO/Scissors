#include "settingsform.h"
#include "ui_settingsform.h"

SettingsForm::SettingsForm(QWidget *parent) :QWidget(parent), ui(new Ui::SettingsForm){
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
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
}

void SettingsForm::Hide(){
    hide();
    _isVisible = false;
}

void SettingsForm::UpdateSettings(ProgramSetting settings){
    ui->checkBox_autoRun->blockSignals(true);
    ui->checkBox_rectSize->blockSignals(true);
    ui->checkBox_stopFrame->blockSignals(true);
    ui->history_size_lineEdit->blockSignals(true);
    ui->checkBox_screenshotModificationArea->blockSignals(true);

    ui->checkBox_autoRun->setChecked(settings.Get_Startup());
    ui->checkBox_stopFrame->setChecked(settings.Get_StopFrame());
    ui->checkBox_rectSize->setChecked(settings.Get_ShowScreenshotZoneGeometry());
    ui->history_size_lineEdit->setText(QString::number(settings.Get_HistorySize()));
    ui->checkBox_screenshotModificationArea->setChecked(settings.Get_ShowModificationArea());

    ui->checkBox_autoRun->blockSignals(false);
    ui->checkBox_rectSize->blockSignals(false);
    ui->checkBox_stopFrame->blockSignals(false);
    ui->history_size_lineEdit->blockSignals(false);
    ui->checkBox_screenshotModificationArea->blockSignals(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsForm::paintEvent(QPaintEvent *){
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.setBrush(QBrush(QColor(46, 46, 57, 200))); // Задаем цвет фона
    paint.drawRoundedRect(rect(), 30, 30); // Задаем радиус закругления
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void SettingsForm::on_checkBox_rectSize_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ShowScreenshotZoneGeometry(checked);
    emit ChangeProgramSettings(settings);
}

void SettingsForm::on_checkBox_stopFrame_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_StopFrame(checked);
    emit ChangeProgramSettings(settings);
}

void SettingsForm::on_history_size_lineEdit_editingFinished(){
    ProgramSetting settings = emit GetProgramSettings();

    int size = ui->history_size_lineEdit->text().toInt();

    if(size < 1)
        size = 1;
    else if(size > 255)
        size = 255;

    settings.Set_HistorySize(size);
    ui->history_size_lineEdit->setText(QString::number(size));

    emit ChangeProgramSettings(settings);
}

void SettingsForm::on_checkBox_screenshotModificationArea_toggled(bool checked){
    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ShowModificationArea(checked);
    emit ChangeProgramSettings(settings);
}

// Изменение языка программы
void SettingsForm::on_combobox_language_currentTextChanged(const QString &arg){
    emit ChangeProgramLanguage(arg);

    ProgramSetting settings = emit GetProgramSettings();
    settings.Set_ProgramLanguage(arg);
    emit ChangeProgramSettings(settings);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsForm::Event_ChangeLanguage(TranslateData data){
    ui->settings_label->setText(data.translate("SETTINGS_LABEL"));

    ui->checkBox_autoRun->setText(data.translate("CHECKBOX_AUTORUN"));
    ui->checkBox_screenshotModificationArea->setText(data.translate("CHECKBOX_SCREENSHOT_MODIFICATION_AREA"));
    ui->checkBox_stopFrame->setText(data.translate("CHECKBOX_STOP_FRAME"));
    ui->checkBox_rectSize->setText(data.translate("CHECKBOX_RECT_SIZE"));

    ui->history_size_label->setText(data.translate("LINE_EDIT_HISTORY_SIZE"));
}

