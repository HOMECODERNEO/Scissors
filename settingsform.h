#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <Includes.h>

namespace Ui { class SettingsForm; }

class SettingsForm : public QWidget{
    Q_OBJECT

    enum {
        ACTION_NONE,
        ACTION_MOVE,
        ACTION_RESIZE_RIGHT_DOWN
    } _action = ACTION_NONE;

    void setCursorOnAll(Qt::CursorShape);
    void checkAndSetCursors(QMouseEvent*);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

signals:
    void ChangeProgramSettings(ProgramSetting settings);
    void ChangeProgramLanguage(QString lang);
    ProgramSetting GetProgramSettings();
    void PlaySound(SOUNDMANAGER_SOUND_TYPE type);

public slots:
    void Event_ChangeLanguage(TranslateData);

private slots:
    void on_btn_general_released();
    void on_btn_sounds_released();
    void on_btn_history_released();
    void on_btn_capture_released();

    void on_button_delete_file_sound_test_released();
    void on_button_notification_sound_test_released();
    void on_button_button_hover_sound_test_released();
    void on_button_button_click_sound_test_released();
    void on_button_history_clear_sound_test_released();
    void on_button_make_screenshot_sound_test_released();
    void on_button_clipboard_image_get_sound_test_released();

    void on_checkBox_autoRun_toggled(bool checked);
    void on_checkBox_rectSize_toggled(bool checked);
    void on_checkBox_stopFrame_toggled(bool checked);
    void on_checkBox_soundsUse_toggled(bool checked);
    void on_checkBox_startupMessages_toggled(bool checked);
    void on_checkBox_viewer_showpercent_toggled(bool checked);
    void on_checkBox_screenshotModificationArea_toggled(bool checked);

    void on_prtsc_timeout_slider_sliderReleased();
    void on_volume_delete_file_slider_sliderReleased();
    void on_volume_notification_slider_sliderReleased();
    void on_volume_button_click_slider_sliderReleased();
    void on_volume_history_clear_slider_sliderReleased();
    void on_volume_make_screenshot_slider_sliderReleased();
    void on_volume_button_hover_sound_slider_sliderReleased();
    void on_volume_clipboard_image_get_slider_sliderReleased();

    void on_prtsc_timeout_slider_valueChanged(int value);
    void on_volume_delete_file_slider_valueChanged(int value);
    void on_volume_notification_slider_valueChanged(int value);
    void on_volume_button_click_slider_valueChanged(int value);
    void on_volume_history_clear_slider_valueChanged(int value);
    void on_volume_make_screenshot_slider_valueChanged(int value);
    void on_volume_button_hover_sound_slider_valueChanged(int value);
    void on_volume_clipboard_image_get_slider_valueChanged(int value);

    void on_history_size_lineEdit_editingFinished();
    void on_combobox_language_currentTextChanged(const QString &arg1);

    void on_checkBox_showShortcut_toggled(bool checked);

    void on_checkBox_smoothScroll_toggled(bool checked);

public:
    explicit SettingsForm(QWidget *parent = nullptr);

    void Show();
    void Hide();
    bool IsVisible();

    QComboBox* GetLanguageBox();

private:
    void Change_StuckWidget_Page(int index);
    void UpdateSettings(ProgramSetting settings);

private:
    bool _isVisible = false;

    Ui::SettingsForm *ui;
    AnimationsManager _animationManager;

    /////////////////////////////// MOUSE WINDOW MOVE
    QString sizeBuffer;
    QSize mousePressDiffFromBorder;
    QRect resizeAccordingly(QMouseEvent*);
    QPointF mousePressGlobalPosition, mousePressPosition, mouseMovePosition;
    bool snapEdgeToScreenOrClosestFellow(QRect& newRect, const QRect& screen, std::function<void(QRect&,int)> setter, std::function<int(const QRect&)> getter, std::function<int(const QRect&)> oppositeGetter);
    /////////////////////////////// MOUSE WINDOW MOVE

    QWidget *_parent;
};

#endif // SETTINGSFORM_H
