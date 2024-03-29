#include "mainwindow.h"

MainWindow *_mainWindowInstance = nullptr;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){
    _mainWindowInstance = this;

    _popup = new PopupManager(this);
    _clipboard = QApplication::clipboard();
    _versionChecker = new VersionChecker(this);
    _translatorManager = new TranslatorManager(this);
    _screenshotProcess = new ScreenshotProcess(this);
    _screenshotHistory = new ScreenshotHistory(this);

    _audioOutput = new QAudioOutput();
    _mediaPlayer = new QMediaPlayer();
    _mediaPlayer->setAudioOutput(_audioOutput);

    _saveManager = new SaveManager(this, [this](byte type, QList<SaveManagerFileData> _listData, ProgramSetting _programSettings){
        SaveManager_OperationsEnd(type, _listData, _programSettings);
    });

    connect(_screenshotHistory, SIGNAL(RemoveHistoryFile(QString)), _saveManager, SLOT(RemoveHistoryFile(QString)));
    connect(_translatorManager, SIGNAL(Event_ChangeLanguage(TranslateData)), _popup, SLOT(Event_ChangeLanguage(TranslateData)));
    connect(_translatorManager, SIGNAL(Event_ChangeLanguage(TranslateData)), _screenshotHistory, SLOT(Event_ChangeLanguage(TranslateData)));
    connect(_translatorManager, SIGNAL(Event_ChangeLanguage(TranslateData)), _screenshotHistory->_settingsMenu, SLOT(Event_ChangeLanguage(TranslateData)));

    /////////////////////////////////////////////////////////////////////// WINAPI HOOK
    /////////////////////////////////////////////////////////////////////// WINAPI HOOK
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    if(hInstance)
        g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardProc, hInstance, 0);
    /////////////////////////////////////////////////////////////////////// WINAPI HOOK
    /////////////////////////////////////////////////////////////////////// WINAPI HOOK

    _saveManager->LoadSettings();
}

// Запрос на закрытие приложения через кнопку в истории
void MainWindow::ClosingRequest(){
    if(g_hHook != NULL){
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }

    this->close();
}

QList<SaveManagerFileData> MainWindow::GetImagesData(){ return _imageData; } // Получение данных об изображениях
ProgramSetting MainWindow::GetProgramSettings(){ return _programSettings; } // Получение настреок программы
QScreen* MainWindow::GetActiveScreen(){ return QGuiApplication::primaryScreen(); } // Получение активного экрана

// Удаления изображения истории
void MainWindow::HistoryRemoveItem(int index){
    _saveManager->RemoveHistoryFile(_imageData[index].GetFileOldName());
    _imageData.removeAt(index);

    // Проигрываем звук удаления одного файла
    PlaySound("qrc:/sounds/Resourse/Sounds/removefile.mp3", _programSettings.Get_VolumeDeleteFile() / 100);
}

// Очищаем весь список истории, а так же удаляем файлы
void MainWindow::HistoryRemoveAllItem(){
    for(int index = _imageData.count() - 1; index >= 0; index--){
        _saveManager->RemoveHistoryFile(_imageData[index].GetFileOldName());
        _imageData.removeAt(index);
    }

    // Проигрываем звук очистки истории
    PlaySound("qrc:/sounds/Resourse/Sounds/recycle_cleaning.wav", _programSettings.Get_VolumeHistoryClear() / 100);
}

// Событие изменения настроек программы
void MainWindow::ChangeProgramSettings(ProgramSetting settings){
    _programSettings = settings;
    _saveManager->SaveSettings(_programSettings);
}

// Событие окончания выполнения задания от менеджера сохранений
void MainWindow::SaveManager_OperationsEnd(byte type, QList<SaveManagerFileData> _listData, ProgramSetting settingsData){
    switch(type){
        case 0:{ // LOAD HISTORY
            _imageData = _listData;

            //Если количество больше установленного то удаляем лишнее
            for(int i = _imageData.count(); i > _programSettings.Get_HistorySize(); i--){
                _saveManager->RemoveHistoryFile(_imageData[i - 1].GetFileOldName());
                _imageData.removeAt(i - 1);
            }
            break;
        }

        case 1:{ // LOAD SETTINGS
            _programSettings = settingsData;

            emit LoadingSettingsEnd(_programSettings);

            _saveManager->LoadHistory();
            _translatorManager->LoadTranslate(_programSettings.Get_ProgramLanguage(), _screenshotHistory->_settingsMenu->GetLanguageBox());
            _versionChecker->check();
            break;
        }
    }
}

// Конец работы редактирования зоны создания скриншота
void MainWindow::ScreenshotProcessEnd(QPixmap image){
    _clipboard->setPixmap(image); // Вставляем изображение в буффер обмена

    _imageData.prepend(SaveManagerFileData(0, image)); // Вставляем изображение в начало

    while(_imageData.count() > _programSettings.Get_HistorySize()){ // Ходим бесконечно и удаляем изображения, пока их не станет [значение]
        _saveManager->RemoveHistoryFile(_imageData.last().GetFileOldName());
        _imageData.removeLast();
    }

    // Обновляем ID сдвинутых элементов
    for(int i = 1; i < _imageData.count(); i++)
        _imageData[i].SetID(i);

    _saveManager->SaveHistory(_imageData);

    _historyNeedUpdate = true;

    if(!_screenshotHistory->isVisible() && !_screenshotProcess->isVisible()){
        // Проигрываем звук создания скриншота
        PlaySound("qrc:/sounds/Resourse/Sounds/printscreen.wav", _programSettings.Get_VolumeMakeScreenshot() / 100);
        _currentProcess = NONE;
    }
}

// Левый клик по изображению в истории
void MainWindow::ScreenshotHistory_ImageCopyToBuffer(int id){
    _clipboard->setPixmap(_imageData[id].GetImage()); // Вставляем изображение в буффер обмена

    _screenshotHistory->Hide();
    _currentProcess = NONE;
}

// Нужно создать и закрепить плавающее окно с изображением
void MainWindow::CreateFloatingWindow(int id, QPixmap image){
    if(_screenshotHistory->Hide()){
        _currentProcess = NONE;

        _screenshotHistoryWindowViewer = new ScreenshotFloatingWindowViewer(id, image, &m_List_WindowViewer, this);
        m_List_WindowViewer.append(_screenshotHistoryWindowViewer);
        _screenshotHistoryWindowViewer->show();
    }
}

// Запрос на закрытие плавающего окна
void MainWindow::FloatingWindowClose(ScreenshotFloatingWindowViewer* window, int, QPixmap image){
    if(!image.isNull()){

    }else{
        m_List_WindowViewer.removeAt(m_List_WindowViewer.indexOf(window));
    }
}

void MainWindow::PlaySound(QString dir, float volume){
    if(!_programSettings.Get_UseSound())
        return;

    _mediaPlayer->setSource(QUrl());
    _mediaPlayer->setSource(QUrl(dir));
    _audioOutput->setVolume(volume);
    _mediaPlayer->play();
}

// Событие нажатия клавиш с WinApi
bool MainWindow::WinApi_KeyEvent(int keyCode, int modify, int clickCount){
    // keyCode == Qt::Key_Print || Qt::Key_Escape
    // modify == Qt::CTRL || Qt::SHIFT
    // clickCount == Number of presses per time

    if(keyCode == Qt::Key_Print && modify == 0 && _currentProcess == NONE){
        if(clickCount == 1){
            if(GetProgramSettings().Get_StopFrame()){
                HDC screenDC = GetDC(0);
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                HDC memDC = CreateCompatibleDC(screenDC);
                HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, screenWidth, screenHeight);
                HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);
                BitBlt(memDC, 0, 0, screenWidth, screenHeight, screenDC, 0, 0, SRCCOPY);
                hBitmap = (HBITMAP)SelectObject(memDC, hOldBitmap);

                _screenshotProcess->SetStopFrameImage(QPixmap::fromImage(QImage::fromHBITMAP(hBitmap)));

                // Освобождение ресурсов
                DeleteDC(memDC);
                ReleaseDC(0, screenDC);
            }
        }else if(clickCount == 2){
            _currentProcess = SCREENSHOT_CREATE;
            _screenshotProcess->Show();
        }

    }else if(keyCode == Qt::Key_Print && modify == Qt::SHIFT && clickCount == 1 && _currentProcess == NONE){
        _currentProcess = SCREENSHOT_HISTORY;
        _screenshotHistory->Show(_historyNeedUpdate);
        _historyNeedUpdate = false;
    }

    if(keyCode == Qt::Key_Escape && modify == 0 && clickCount == 1 && _currentProcess != NONE){
        if(_currentProcess == SCREENSHOT_CREATE){
            _screenshotProcess->Hide();
            _currentProcess = NONE;
            return true;

        }else if(_currentProcess == SCREENSHOT_HISTORY){
            if(_screenshotHistory->Hide()){
                _currentProcess = NONE;
                return true;
            }
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// WINAPI HOOK
////////////////////////////////////////////////////////////////////////////////////////////////////////////// WINAPI HOOK
////////////////////////////////////////////////////////////////////////////////////////////////////////////// WINAPI HOOK
LRESULT CALLBACK MainWindow::GlobalKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){

    static DWORD lastKeyPressTime = 0, lastKeyPress = 0;
    static int keyPressCount = 0, _keyModify = 0;

    if (nCode >= 0 && wParam == WM_KEYDOWN){
        KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;

        if(_mainWindowInstance){
            DWORD currentTime = GetTickCount();
            DWORD elapsedTime = currentTime - lastKeyPressTime;

            ////////////////////////////////////////////
            if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
                _keyModify = Qt::CTRL;
            else if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
                _keyModify = Qt::SHIFT;
            else
                _keyModify = 0;
            ///////////////////////////////////////////

            if(elapsedTime < static_cast<DWORD>(_mainWindowInstance->_programSettings.Get_PrtSc_Timeout()) && pKeyInfo->vkCode == lastKeyPress/*ЭТО В МЕНЮ НАСТРОЕК, В ДИАПАЗОН ОТ 200 ДО 1000*/)
                keyPressCount++;
            else
                keyPressCount = 1;

            if(pKeyInfo->vkCode == VK_SNAPSHOT){
                lastKeyPress = VK_SNAPSHOT;
                lastKeyPressTime = GetTickCount();

                _mainWindowInstance->WinApi_KeyEvent(Qt::Key_Print, _keyModify, keyPressCount);
            }else if(pKeyInfo->vkCode == VK_ESCAPE){
                lastKeyPress = VK_ESCAPE;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Escape, _keyModify, keyPressCount))
                    return 0;
            }
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE

// Поступил сигнал с формы (изменили язык в настройках)
void MainWindow::ChangeProgramLanguage(QString lang){
    _translatorManager->ChangeProgramLanguage(&_translateData, lang);
}

// Переводчик закончил формирование списка перевода
void MainWindow::Event_ChangeLanguage(TranslateData data){
    _translateData = data;

    if(_startProgramMessage)
        return;

    _startProgramMessage = true;
    ShowPopup("#PROGRAM_START#", "", 3000, "");
}

// Функция вызова уведомление
void MainWindow::ShowPopup(QString text, QString additional, int time, QString url){

    // Отображаем сообщение
    _popup->showMessage(text, additional, time, url, _translateData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
