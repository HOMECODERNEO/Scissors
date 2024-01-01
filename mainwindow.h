#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Includes.h>

#include <savemanager.h>
#include <screenshotprocess.h>
#include <screenshothistory.h>
#include <translatormanager.h>
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
    void SaveManager_OperationsEnd(byte type, QList<SaveManagerFileData> _listData, ProgramSetting _settingsData);

    static LRESULT CALLBACK GlobalKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

public slots:
    void ClosingRequest();

    QScreen* GetActiveScreen();
    void HistoryRemoveAllItem();
    void HistoryRemoveItem(int index);
    ProgramSetting GetProgramSettings();
    QList<SaveManagerFileData> GetImagesData();

    void ScreenshotProcessEnd(QPixmap image);
    void ScreenshotHistory_ImageCopyToBuffer(int id);
    void ChangeProgramSettings(ProgramSetting settings);

    void ChangeProgramLanguage(QString);
    void CreateFloatingWindow(int, QPixmap);
    void FloatingWindowClose(ScreenshotFloatingWindowViewer* window, int id, QPixmap image);

private:
    HHOOK g_hHook = NULL;

    QClipboard *_clipboard;
    bool _historyNeedUpdate = false;

    TranslateData _translateData;
    ProgramSetting _programSettings;
    QList<SaveManagerFileData> _imageData;
    QList <ScreenshotFloatingWindowViewer*> m_List_WindowViewer;

    SaveManager *_saveManager;
    ScreenshotProcess *_screenshotProcess;
    ScreenshotHistory *_screenshotHistory;
    TranslatorManager *_translatorManager;
    ScreenshotFloatingWindowViewer *_screenshotHistoryWindowViewer;
};

#endif // MAINWINDOW_H
