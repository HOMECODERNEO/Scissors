#ifndef SCREENSHOTHISTORY_H
#define SCREENSHOTHISTORY_H

#include <settingsform.h>
#include <customhistorybutton.h>
#include <screenshothistoryviewer.h>

class ScreenshotHistory : public QWidget{
    Q_OBJECT

public:
    explicit ScreenshotHistory(QWidget *parent = nullptr);

    void Show(bool needUpdate);
    bool Hide();

private:
    void CreateHistoryUI();
    void CreateContexMenu();

    void UpdateHistoryGrid(QList<SaveManagerFileData> data, bool needUpdate);
    long MapValue(long x, long in_min, long in_max, long out_min, long out_max);

public slots:
    void LoadingSettingsEnd(ProgramSetting settings);
    void updateCursorPositionTimer();

    void ContextMenuView();
    void ContextMenuSave();
    void ContextMenuWindow();
    void ContextMenuDeleteImage();

    void onLeftClicked(byte id);
    void onRightClicked(byte id);
    void onMiddleClicked(byte id);

    void Event_ChangeLanguage(TranslateData data);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

signals:
    void ClosingRequest();
    QScreen* GetActiveScreen();
    void HistoryRemoveAllItem();
    void HistoryRemoveItem(int index);
    ProgramSetting GetProgramSettings();
    void RemoveHistoryFile(QString hash);
    void ScreenshotProcessEnd(QPixmap image);
    QList<SaveManagerFileData> GetImagesData();
    void ShowPopup(QString, QString, int, QString);
    void ScreenshotHistory_ImageCopyToBuffer(int id);
    void CreateFloatingWindow(int id, QPixmap image);
    void ChangeProgramSettings(ProgramSetting settings);

public:
    SettingsForm *_settingsMenu;

private:
    QString _text_NoHIstory, _text_Dialog_HistoryClear;

    QMenu *contextMenu;
    QGridLayout *gridLayout;
    QTimer *_buttonsHoverTimer;

    bool _msgBoxVisible = false, _imageLoadVisible = false;
    QRect *_settingButton, *_exitButton, *_clearHistoryButton, *_uploadImageButton;
    int _buttonIDBuffer = 0, _historyImageButtonsNum = 0, _historyMaxSize = 0;
    bool _exitButtonIsHover = false, _settingButtonIsHover = false, _clearHistoryButtonHover = false, _uploadImageButtonHover = false;

    AnimationsManager _animationManager;
    ScreenshotHistoryViewer *_screenshotViewer;
};

#endif // SCREENSHOTHISTORY_H
