#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <Includes.h>

namespace Ui { class SettingsForm; }

class SettingsForm : public QWidget{
    Q_OBJECT

protected:
    virtual void paintEvent(QPaintEvent *);

signals:
    void ChangeProgramSettings(ProgramSetting settings);
    void ChangeProgramLanguage(QString lang);
    ProgramSetting GetProgramSettings();

private slots:
    void on_checkBox_autoRun_toggled(bool checked);
    void on_checkBox_rectSize_toggled(bool checked);
    void on_checkBox_stopFrame_toggled(bool checked);
    void on_history_size_lineEdit_editingFinished();
    void on_checkBox_screenshotModificationArea_toggled(bool checked);

    void on_combobox_language_currentTextChanged(const QString &arg1);

    void Event_ChangeLanguage(TranslateData);

public:
    explicit SettingsForm(QWidget *parent = nullptr);

    void Show();
    void Hide();
    bool IsVisible();

    QComboBox* GetLanguageBox();

private:
    void UpdateSettings(ProgramSetting settings);

private:
    bool _isVisible = false;

    Ui::SettingsForm *ui;
    AnimationsManager _animationManager;
};

#endif // SETTINGSFORM_H
