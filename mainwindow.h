#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Includes.h>

#include <savemanager.h>
#include <soundmanager.h>
#include <popupmanager.h>
#include <screenshotprocess.h>
#include <screenshothistory.h>
#include <screenshotfloatingwindowviewer.h>

class MainWindow : public QMainWindow{
    Q_OBJECT

    enum {
        NONE,
        SCREENSHOT_CREATE,
        SCREENSHOT_HISTORY
    } _currentProcess = NONE;

public:
    MainWindow(QWidget *parent = nullptr);

    bool WinApi_KeyEvent(int keyCode, int modify, int clickCount);
    void SaveManager_OperationsEnd(SaveManagerData returnData);

    static LRESULT CALLBACK GlobalKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void ProcessEvent_HideEnd();
    void ProcessEvent_ShowStart();
    void ProcessEvent_HideStart();
    void ProcessEvent_ShowEnd(QWidget *widget);

    void ScreenshotProcess_CreateScreenshot(QPixmap image, bool hidden);

    void SaveHistory();
    QRect GetCurrentScreenGeometry();
    void HistoryRemoveAllItem();
    bool HistoryImportOperationEnd();
    void HistoryRemoveItem(int index);
    ProgramSetting GetProgramSettings();
    QList<SaveManagerFileData> GetImagesData();
    void ChangeImagesData(SaveManagerFileData imageData, int index);

    void ChangeProgramSettings(ProgramSetting settings);

    void ChangeProgramLanguage(QString);
    void CreateFloatingWindow(QPixmap);
    void FloatingWindowClose(ScreenshotFloatingWindowViewer* window);

    void Event_ChangeLanguage(TranslateData data);
    void ShowPopup(QString mainMessage, QString secondaryMessage, QString windowMessage, QString url, int time);


private:
    void ImportImageAddToDataList(int iteration, SaveManagerFileData imageData);
    void AddImageToDataList(QPixmap image, bool hidden);
    void ScreenshotProcess_CopyImageToClipboard(QPixmap image);
    void ScreenshotProcess_CopyFromClipboard(QPixmap image);
    bool RemovingExcessImage(QList<SaveManagerFileData>& imageData, int num);

private:
    QClipboard *_clipboard;
    bool _startProgramMessage = false;
    bool _processBlockingControl = false;

    TranslateData _translateData;
    ProgramSetting _programSettings;
    QList<SaveManagerFileData> _imageData;
    QMap<int, SaveManagerFileData> _importImageDataQueue;
    QList <ScreenshotFloatingWindowViewer*> m_List_WindowViewer;

    PopupManager *_popup;
    SaveManager *_saveManager;
    SoundManager *_soundManager;
    AnimationsManager _animationManager;
    ScreenshotProcess *_screenshotProcess;
    ScreenshotHistory *_screenshotHistory;
    ScreenshotFloatingWindowViewer *_screenshotHistoryWindowViewer;
};

#endif // MAINWINDOW_H
