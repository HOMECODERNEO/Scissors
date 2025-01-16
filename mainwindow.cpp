#include "mainwindow.h"

HHOOK g_hHook = NULL;
MainWindow *_mainWindowInstance = nullptr;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){
    _mainWindowInstance = this;

    ///////////////////////////////////////////////// Выделение памяти классам
    _screenshotProcess = new ScreenshotProcess(this);
    _screenshotHistory = new ScreenshotHistory(this);

    _clipboard = QApplication::clipboard();
    _soundManager = new SoundManager(this);
    _popup = new PopupManager(_screenshotHistory);

    _saveManager = new SaveManager(this, [this](SaveManagerData returnData){
        SaveManager_OperationsEnd(returnData);
    });
    ///////////////////////////////////////////////// Выделение памяти классам

    ///////////////////////////////////////////////// Подключение сигналов классов
    connect(_saveManager, &SaveManager::Event_LoadLanguage, this, &MainWindow::Event_ChangeLanguage);
    connect(_saveManager, &SaveManager::Event_LoadLanguage, _popup, &PopupManager::Event_ChangeLanguage);
    connect(_saveManager, &SaveManager::Event_LoadLanguage, _screenshotHistory, &ScreenshotHistory::Event_ChangeLanguage);
    connect(_saveManager, &SaveManager::Event_LoadLanguage, _screenshotHistory->GetSettingsForm(), &SettingsForm::Event_ChangeLanguage);

    connect(_popup, &PopupManager::PlaySound, _soundManager, &SoundManager::PlaySound);
    connect(_popup, &PopupManager::GetProgramSettings, this, &MainWindow::GetProgramSettings);
    connect(_popup, &PopupManager::GetCurrentScreenGeometry, this, &MainWindow::GetCurrentScreenGeometry);

    connect(_soundManager, &SoundManager::GetProgramSettings, this, &MainWindow::GetProgramSettings);

    connect(_screenshotProcess, &ScreenshotProcess::PlaySound, _soundManager, &SoundManager::PlaySound);
    connect(_screenshotProcess, &ScreenshotProcess::GetProgramSettings, this, &MainWindow::GetProgramSettings);
    connect(_screenshotProcess, &ScreenshotProcess::ProcessEvent_HideEnd, this, &MainWindow::ProcessEvent_HideEnd);
    connect(_screenshotProcess, &ScreenshotProcess::ProcessEvent_ShowEnd, this, &MainWindow::ProcessEvent_ShowEnd);
    connect(_screenshotProcess, &ScreenshotProcess::ProcessEvent_ShowStart, this, &MainWindow::ProcessEvent_ShowStart);
    connect(_screenshotProcess, &ScreenshotProcess::ProcessEvent_HideStart, this, &MainWindow::ProcessEvent_HideStart);
    connect(_screenshotProcess, &ScreenshotProcess::GetCurrentScreenGeometry, this, &MainWindow::GetCurrentScreenGeometry);
    connect(_screenshotProcess, &ScreenshotProcess::ScreenshotProcess_CreateScreenshot, this, &MainWindow::ScreenshotProcess_CreateScreenshot);
    connect(_screenshotProcess->GetToolPopup(), &ScreenshotProcessToolPopup::LoadBrushPatternAtIndex, _saveManager, &SaveManager::LoadBrushPatternAtIndex);

    connect(_screenshotHistory, &ScreenshotHistory::ShowPopup, this, &MainWindow::ShowPopup);
    connect(_screenshotHistory, &ScreenshotHistory::ClosingRequest, this, &MainWindow::close);
    connect(_screenshotHistory, &ScreenshotHistory::SaveHistory, this, &MainWindow::SaveHistory);
    connect(_screenshotHistory, &ScreenshotHistory::GetImagesData, this, &MainWindow::GetImagesData);
    connect(_screenshotHistory, &ScreenshotHistory::ChangeImagesData, this, &MainWindow::ChangeImagesData);
    connect(_screenshotHistory, &ScreenshotHistory::HistoryRemoveItem, this, &MainWindow::HistoryRemoveItem);
    connect(_screenshotHistory, &ScreenshotHistory::GetProgramSettings, this, &MainWindow::GetProgramSettings);
    connect(_screenshotHistory, &ScreenshotHistory::HistoryRemoveAllItem, this, &MainWindow::HistoryRemoveAllItem);
    connect(_screenshotHistory, &ScreenshotHistory::CreateFloatingWindow, this, &MainWindow::CreateFloatingWindow);
    connect(_screenshotHistory, &ScreenshotHistory::ProcessEvent_HideEnd, this, &MainWindow::ProcessEvent_HideEnd);
    connect(_screenshotHistory, &ScreenshotHistory::ProcessEvent_ShowEnd, this, &MainWindow::ProcessEvent_ShowEnd);
    connect(_screenshotHistory, &ScreenshotHistory::ChangeProgramSettings, this, &MainWindow::ChangeProgramSettings);
    connect(_screenshotHistory->GetSettingsForm(), &SettingsForm::PlaySound, _soundManager, &SoundManager::PlaySound);
    connect(_screenshotHistory, &ScreenshotHistory::ImportOperationEnd, this, &MainWindow::HistoryImportOperationEnd);
    connect(_screenshotHistory, &ScreenshotHistory::ProcessEvent_ShowStart, this, &MainWindow::ProcessEvent_ShowStart);
    connect(_screenshotHistory, &ScreenshotHistory::ProcessEvent_HideStart, this, &MainWindow::ProcessEvent_HideStart);
    connect(_screenshotHistory, &ScreenshotHistory::ImportImageToDataList, this, &MainWindow::ImportImageAddToDataList);
    connect(_screenshotHistory, &ScreenshotHistory::ScreenshotProcess_CopyImageToClipboard, this, &MainWindow::ScreenshotProcess_CopyImageToClipboard);
    ///////////////////////////////////////////////// Подключение сигналов классов

    /////////////////////////////////////////////////////////////////////// WINAPI HOOK
    /////////////////////////////////////////////////////////////////////// WINAPI HOOK

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    if(hInstance)
        g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, GlobalKeyboardProc, hInstance, 0);

    /////////////////////////////////////////////////////////////////////// WINAPI HOOK
    /////////////////////////////////////////////////////////////////////// WINAPI HOOK

    // Запуск загрузки настроек и данных программы
    _saveManager->LoadSettings();
}

// Запрос на закрытие приложения
void MainWindow::closeEvent(QCloseEvent *event){
    if(g_hHook != NULL){
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }

    // Сигнализируем менеджеру сохранений об завершении работы программы
    _saveManager->ProgramExit();

    // Подтверждаем операцию закрытия
    event->accept();
}

QList<SaveManagerFileData> MainWindow::GetImagesData(){ return _imageData; } // Получение данных об изображениях
ProgramSetting MainWindow::GetProgramSettings(){ return _programSettings; } // Получение настроек программы
QRect MainWindow::GetCurrentScreenGeometry(){ return QGuiApplication::primaryScreen()->geometry(); } // Получаем текущий размер экрана

// Изменяем данные об изображении в базе
void MainWindow::ChangeImagesData(SaveManagerFileData imageData, int index){
    if(index >= 0 && index < _imageData.count())
        _imageData[index] = imageData;
}

// Запрос на сохрание истории
void MainWindow::SaveHistory(){
    _saveManager->SaveHistory(_imageData);
}

// Удаления изображения истории
void MainWindow::HistoryRemoveItem(int index){
    if(index >= 0 && index < _imageData.count()){
        // Удаляем изображение
        _imageData.removeAt(index);

        // Проигрываем звук удаления одного файла
        _soundManager->PlaySound(SOUND_TYPE_REMOVE_FILE);

        // Сохраняем историю
        SaveHistory();
    }
}

// Очищаем весь список истории, а так же удаляем файлы
void MainWindow::HistoryRemoveAllItem(){
    // Очищаем список
    _imageData.clear();

    // Проигрываем звук очистки истории
    _soundManager->PlaySound(SOUND_TYPE_RECYCLE_CLEANING);

    // Сохраняем историю
    SaveHistory();
}

// Событие изменения настроек программы
void MainWindow::ChangeProgramSettings(ProgramSetting settings){
    // Присваиваем новые настройки
    _programSettings = settings;

    // Сохраняем новые настройки
    _saveManager->SaveSettings(_programSettings);
}

// Событие окончания выполнения задания от менеджера сохранений
void MainWindow::SaveManager_OperationsEnd(SaveManagerData returnData){
    switch(returnData.GetReturnType()){

        // Загрузка настроек
        case SaveManagerData::SAVEMANAGER_SETTINGS_LOADED:{
            _programSettings = returnData.GetProgramSettings();

            // Отправляем настройки в историю для проверки (позицию и развер окна настроек)
            _screenshotHistory->CheckSettingsWindow(_programSettings);

            // Запускаем пачку загрузок данных
            _saveManager->LoadHistory();
            _saveManager->CheckProgramNewVersion();
            _screenshotProcess->PenDashLinesInit(_saveManager->LoadPenDashPatterns());
            _saveManager->LoadBrushPatterns(_screenshotProcess->GetToolPopup()->GetBrushPatternsBox());
            _saveManager->LoadTranslations(_programSettings.Get_ProgramLanguage(), _screenshotHistory->GetSettingsForm()->GetLanguageBox());
            break;
        }

        // Загрузка данных изображений
        case SaveManagerData::SAVEMANAGER_HISTORY_LOADED:{
            _imageData = returnData.GetHistoryData();

            //Если количество больше установленного то удаляем лишнее и сохраняем изменения
            if(RemovingExcessImage(_imageData, _programSettings.Get_HistorySize()))
                SaveHistory();

            break;
        }

        // Окончание проверки новой версии
        case SaveManagerData::SAVEMANAGER_VERSIONCHECKER_DONE:{

            switch(returnData.GetVersionCheckerType()){
                // EMPTY
                case 0:
                    break;

                // Доступна новая версия (Отображаем без таймера - бесконечно)
                case 1:
                    ShowPopup(returnData.GetVersionCheckerMessage(), returnData.GetVersionCheckerMessageAdditional(), returnData.GetVersionCheckerWindowMessage(), returnData.GetVersionCheckerUrl(), 0);
                    break;

                // Ошибка (Отображаем 5 секунд)
                case 2: ShowPopup(returnData.GetVersionCheckerMessage(), returnData.GetVersionCheckerMessageAdditional(), "", returnData.GetVersionCheckerUrl(), 5000);
                    break;
            }

            break;
        }
    }
}

// Нужно создать и закрепить плавающее окно с изображением
void MainWindow::CreateFloatingWindow(QPixmap image){

    // Скрываем историю
    _screenshotHistory->Hide();

    // Создаем экземпляр класса и вставляем его в список
    _screenshotHistoryWindowViewer = new ScreenshotFloatingWindowViewer(image, &m_List_WindowViewer, this);
    m_List_WindowViewer.append(_screenshotHistoryWindowViewer);

    // Отображаем окно
    _screenshotHistoryWindowViewer->show();
}

// Запрос на закрытие плавающего окна
void MainWindow::FloatingWindowClose(ScreenshotFloatingWindowViewer* window){
    int index = m_List_WindowViewer.indexOf(window);

    if(index != -1){
        ScreenshotFloatingWindowViewer* windowToRemove = m_List_WindowViewer.takeAt(index);

        _animationManager.Create_WindowOpacity(this, [windowToRemove](){

                                                                        /*Освобождаем память после завершения анимации*/
                                                                        delete windowToRemove;

                                                                        }, 100, 1, 0).Start();
    }
}

void MainWindow::ScreenshotProcess_CreateScreenshot(QPixmap image, bool hidden){
    // Добавляем в буфер обмена
    ScreenshotProcess_CopyImageToClipboard(image);

    // Добавляем в список скришнотов
    AddImageToDataList(image, hidden);

    // Сохраняем новое изображение
    SaveHistory();
}

// Событие нажатия клавиш с WinApi
bool MainWindow::WinApi_KeyEvent(int keyCode, int modify, int clickCount){
    // keyCode == Qt::Key_Print || Qt::Key_Escape || Qt::Key_Right || Qt::Key_Left || Qt::Key_Delete
    // modify == Qt::CTRL || Qt::SHIFT
    // clickCount == Number of presses per time

    // Процесс нажатий PrtSc
    if(keyCode == Qt::Key_Print && modify == 0 && _currentProcess == NONE){

        switch(clickCount){
            // Одно нажатие (делаем стоп-кадр если включен параметр в настройках)
            case 1:{
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

                break;
            }

            // Второе нажатие уже открывает само окно процесса создания изображения
            case 2:{
                _screenshotProcess->Show();
                return true;
            }
        }

    }

    if(!_processBlockingControl){
        if(keyCode == Qt::Key_Print && modify == Qt::SHIFT && clickCount == 1 && _currentProcess == NONE){
            _screenshotHistory->Show();
            return true;
        }

        if(keyCode == Qt::Key_Escape && modify == 0 && clickCount == 1 && _currentProcess != NONE){
            switch(_currentProcess){
                case NONE: break;

                case SCREENSHOT_CREATE:{
                    if(_screenshotProcess->Hide())
                        return true;

                    break;
                }

                case SCREENSHOT_HISTORY:{
                    if(_screenshotHistory->Hide())
                        return true;

                    break;
                }
            }
        }
    }

    if (keyCode == Qt::Key_Print && modify == Qt::CTRL && _currentProcess == NONE){
        const QMimeData *mimeData = _clipboard->mimeData();

        if(mimeData->hasImage()){
            QPixmap image = _clipboard->pixmap();

            if(!image.isNull())
                ScreenshotProcess_CopyFromClipboard(image);
            else
                ShowPopup("#CLIPBOARD_IMAGE_NOT_FOUND#", "", "", "", 2000); // [Main Message - Secondary Message - Window Message - URL - Display Time]
        }else
            ShowPopup("#CLIPBOARD_IMAGE_NOT_FOUND#", "", "", "", 2000); // [Main Message - Secondary Message - Window Message - URL - Display Time]

        return true;
    }

    if(modify == 0 && _currentProcess == SCREENSHOT_HISTORY){
        if(keyCode == Qt::Key_Right){
            _screenshotHistory->SwitchingViewedImages(true);
            return 1;

        }else if(keyCode == Qt::Key_Left){
            _screenshotHistory->SwitchingViewedImages(false);
            return 1;
        }
    }

    if(keyCode == Qt::Key_Delete && modify == 0 && _currentProcess == SCREENSHOT_CREATE){
        _screenshotProcess->ClearFigure();
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// PRIVATE FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////////////////////// PRIVATE FUNCTION
////////////////////////////////////////////////////////////////////////////////////////////////////////////// PRIVATE FUNCTION

// Левый клик по изображению в истории
void MainWindow::ScreenshotProcess_CopyImageToClipboard(QPixmap image){
    // Вставляем изображение в буффер обмена
    _clipboard->setPixmap(image);
}

void MainWindow::ScreenshotProcess_CopyFromClipboard(QPixmap image){
    AddImageToDataList(image, false);

    // Выводим сообщение
    ShowPopup("#CLIPBOARD_IMAGE_ADDED#", "", "", "", 2000);

    // Проигрываем звук копирования изображения из буфера обмена в историю
    _soundManager->PlaySound(SOUND_TYPE_GET_CLIPBOARD_IMAGE);

    // Сохраняем изображение
    SaveHistory();
}

// [Main Message - Secondary Message - Window Message - URL - Display Time]
void MainWindow::AddImageToDataList(QPixmap image, bool hidden){
    // Вставляем изображение в начало
    _imageData.prepend(SaveManagerFileData(0, image, -1, hidden));

    // Если количество больше установленного то удаляем лишнее и сохраняем изменения
    if(RemovingExcessImage(_imageData, _programSettings.Get_HistorySize()))
        SaveHistory();

    // Обновляем ID сдвинутых элементов
    for(int i = 1; i < _imageData.count(); i++)
        _imageData[i].SetID(i);
}

// Импортируем в список очереди изображения с множествва потоков
void MainWindow::ImportImageAddToDataList(int iteration, SaveManagerFileData imageData){
    _importImageDataQueue.insert(iteration, imageData);
}

// Сортируем импортированые как попало изображения
bool MainWindow::HistoryImportOperationEnd(){
    int count = _importImageDataQueue.count();

    for(int key = 0; key < count; key++){

        if(_importImageDataQueue.contains((count - 1) - key))
            _imageData.prepend(_importImageDataQueue[(count - 1) - key]);

    }
    
    // Если количество больше установленного то удаляем лишнее и сохраняем изменения
    if(RemovingExcessImage(_imageData, _programSettings.Get_HistorySize()))
        SaveHistory();

    // Обновляем ID сдвинутых элементов
    for(int i = 0; i < _imageData.count(); i++)
        _imageData[i].SetID(i);

    _importImageDataQueue.clear();

    return true;
}

// Функция удаления лишних изображений в истории (если их кол-во больше выставленного лимита)
bool MainWindow::RemovingExcessImage(QList<SaveManagerFileData>& imageData, int num){
    bool removingExcess = false;

    while(imageData.count() > num){
        imageData.removeLast();

        removingExcess = true;
    }

    return removingExcess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// OPEN AND CLOSE EVENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////// OPEN AND CLOSE EVENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////// OPEN AND CLOSE EVENT

void MainWindow::ProcessEvent_ShowStart(){
    _processBlockingControl = true;
}

void MainWindow::ProcessEvent_HideStart(){
    _processBlockingControl = true;
}

void MainWindow::ProcessEvent_ShowEnd(QWidget *widget){
    _processBlockingControl = false;

    if(widget == _screenshotProcess)
        _currentProcess = SCREENSHOT_CREATE;
    else if(widget == _screenshotHistory)
        _currentProcess = SCREENSHOT_HISTORY;
}

void MainWindow::ProcessEvent_HideEnd(){
    _processBlockingControl = false;

    _currentProcess = NONE;
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

            if(elapsedTime < static_cast<DWORD>(_mainWindowInstance->_programSettings.Get_PrtSc_Timeout()) && pKeyInfo->vkCode == lastKeyPress)
                keyPressCount++;
            else
                keyPressCount = 1;

            if(pKeyInfo->vkCode == VK_SNAPSHOT){
                lastKeyPress = VK_SNAPSHOT;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Print, _keyModify, keyPressCount))
                    return 1;

            }else if(pKeyInfo->vkCode == VK_ESCAPE){
                lastKeyPress = VK_ESCAPE;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Escape, _keyModify, keyPressCount))
                    return 1;

            }else if(pKeyInfo->vkCode == VK_RIGHT){
                lastKeyPress = VK_RIGHT;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Right, _keyModify, keyPressCount))
                    return 1;

            }else if(pKeyInfo->vkCode == VK_LEFT){
                lastKeyPress = VK_LEFT;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Left, _keyModify, keyPressCount))
                    return 1;
            }else if(pKeyInfo->vkCode == VK_DELETE){
                lastKeyPress = VK_DELETE;
                lastKeyPressTime = GetTickCount();

                if(_mainWindowInstance->WinApi_KeyEvent(Qt::Key_Delete, _keyModify, keyPressCount))
                    return 1;
            }
        }
    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE
////////////////////////////////////////////////////////////////////////////////////////////////////////////// TRANSLATE

// Поступил сигнал с формы (изменили язык в настройках)
void MainWindow::ChangeProgramLanguage(QString lang){
    _saveManager->ChangeProgramLanguage(&_translateData, lang);
}

// Переводчик закончил формирование списка перевода
void MainWindow::Event_ChangeLanguage(TranslateData data){
    _translateData = data;

    if(_startProgramMessage || !_programSettings.Get_StartupMessage())
        return;

    _startProgramMessage = true;
    ShowPopup("#PROGRAM_START1#", "", "", "", 1000); // [Main Message - Secondary Message - Window Message - URL - Display Time]
    ShowPopup("#PROGRAM_START2#", "", "", "", 2000); // [Main Message - Secondary Message - Window Message - URL - Display Time]
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// POPUP
////////////////////////////////////////////////////////////////////////////////////////////////////////////// POPUP
////////////////////////////////////////////////////////////////////////////////////////////////////////////// POPUP

// Функция вызова уведомление
void MainWindow::ShowPopup(QString mainMessage, QString secondaryMessage, QString windowMessage, QString url, int time){
    // Отображаем сообщение
    _popup->showMessage(mainMessage, secondaryMessage, windowMessage, url, time, _translateData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
