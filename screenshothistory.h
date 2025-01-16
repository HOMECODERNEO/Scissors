#ifndef SCREENSHOTHISTORY_H
#define SCREENSHOTHISTORY_H

#include <settingsform.h>
#include <screenshothistoryviewer.h>

#include <QMutex>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QToolTip>
#include <QScrollBar>
#include <QScroller>

// Кастомные классы которые описаные в этом заголовочном файле ниже основного класа
class CustomHistoryButton;

//////////////////////////////////////////////////////////////////////////
//////////////////////////// SCREENSHOTHISTORY ///////////////////////////
//////////////////////////////////////////////////////////////////////////

class ScreenshotHistory : public QWidget{
    Q_OBJECT

    class Shortcut{
        private:
            QString _key;
            QString _description;

        public:
            Shortcut(QString key, QString description){
                _key = key;
                _description = description;
            }

            void SetKey(QString key){
                _key = key;
            }

            void SetDescription(QString description){
                _description = description;
            }

            QString GetKey(){ return _key; }
            QString GetDescription(){ return _description; }
    };

public:
    explicit ScreenshotHistory(QWidget *parent = nullptr);

    void Show();
    bool Hide();

    void SwitchingViewedImages(bool dir);
    void CheckSettingsWindow(ProgramSetting settings);

    SettingsForm* GetSettingsForm() const;

private:
    void CreateHistoryUI();
    void CreateContexMenu();

    QImage RotatePixmap(const QImage &pixmap, qreal angle);

    void AddButtonToHistory();
    void RemoveButtonFromHistory();
    void UpdateHistoryGrid(QList<SaveManagerFileData> data, SaveManagerFileData oneChange = SaveManagerFileData());

    QRect PaintEventAddKeyFonction(int x, int y, QString key, QColor color, QPainter &paint);

    long MapValueLong(long x, long in_min, long in_max, long out_min, long out_max);
    float MapValueFloat(float x, float in_min, float in_max, float out_min, float out_max);

public slots:
    void updateCursorPositionTimer();

    void ContextMenuView();
    void ContextMenuSave();
    void ContextMenuShow();
    void ContextMenuHide();
    void ContextMenuWindow();
    void ContextMenuRotate();
    void ContextMenuDeleteImage();

    void onLeftClicked(CustomHistoryButton *button);
    void onRightClicked(CustomHistoryButton *button);
    void onMiddleClicked(CustomHistoryButton *button);

    void Event_ChangeLanguage(TranslateData data);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *pe);

signals:
    void ProcessEvent_HideEnd();
    void ProcessEvent_ShowStart();
    void ProcessEvent_HideStart();
    void ProcessEvent_ShowEnd(QWidget *);

    void SaveHistory();
    void ClosingRequest();
    bool ImportOperationEnd();
    void ProcessHistory_HideEnd();
    QRect GetCurrentScreenGeometry();
    void HistoryRemoveAllItem();
    void HistoryRemoveItem(int index);
    ProgramSetting GetProgramSettings();
    QList<SaveManagerFileData> GetImagesData();
    void ImportImageToDataList(int, SaveManagerFileData);
    void ShowPopup(QString, QString, QString, QString, int);
    void ScreenshotProcess_CopyImageToClipboard(QPixmap);
    void CreateFloatingWindow(QPixmap image);
    void ChangeProgramSettings(ProgramSetting);
    void ChangeImagesData(SaveManagerFileData, int);

private:
    QList<Shortcut*> _showShortcuts;

    QString _text_NoHIstory, _text_Dialog_HistoryClear;

    QMenu *contextMenu;
    QGridLayout *gridLayout;
    QTimer *_buttonsHoverTimer;

    bool _operationBlock = false;
    QRect *_settingButton, *_exitButton, *_clearHistoryButton, *_uploadImageButton, *_unloadImageButton, *_tooltipRect;

    int _buttonIDBuffer = 0;
    int _historyImageSwitchingIndex = 0;
    int _historyImageButtonsNum = 0, _historyMaxSize = 0;

    bool _exitButtonIsHover = false, _settingButtonIsHover = false, _uploadImageButtonIsHover = false, _unloadImageButtonIsHover = false, _clearHistoryButtonIsHover = false;

    SettingsForm *_settingsMenu;
    AnimationsManager _animationManager;
    ScreenshotHistoryViewer *_screenshotViewer;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////// CUSTOMHISTORYSCROLLAREA /////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomHistoryScrollArea : public QScrollArea{
    Q_OBJECT

public:
    CustomHistoryScrollArea(QWidget *parent) : QScrollArea(parent){
        _ms = 50;
        _fps = 1;
        _stepsTotal = 0;

        _smoothTimer = new QTimer(this);
        connect(_smoothTimer, &QTimer::timeout, this, &CustomHistoryScrollArea::smoothMove);
    }

    void setFps(int fps){ _fps = fps; }
    void setDuration(int ms){ _ms = ms; }

signals:
    ProgramSetting GetProgramSettings();

protected:
    void wheelEvent(QWheelEvent *event) override{

        if( !( (emit GetProgramSettings()).Get_HistorySmoothScroll() ) ){
            QScrollArea::wheelEvent(event);
            return;
        }

        static QQueue<qint64> scrollStamps;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        scrollStamps.enqueue(now);

        while (now - scrollStamps.front() > 500)
            scrollStamps.dequeue();

        double accerationRatio = qMin(scrollStamps.size() / 15.0, 1.0);

        QPointF pos = event->position();
        QPointF globalPos = event->globalPosition();
        QPoint pixelDelta = event->pixelDelta();
        QPoint angleDelta = event->angleDelta();

        Qt::MouseButtons buttons = event->buttons();
        Qt::KeyboardModifiers modifiers = event->modifiers();
        Qt::ScrollPhase phase = event->phase();

        bool inverted = event->inverted();
        Qt::MouseEventSource source = event->source();

        if (!_lastWheelEvent)
            _lastWheelEvent = std::make_unique<QWheelEvent>(pos, globalPos, pixelDelta, angleDelta, buttons, modifiers, phase, inverted, source);
        else
            _lastWheelEvent.reset(new QWheelEvent(pos, globalPos, pixelDelta, angleDelta, buttons, modifiers, phase, inverted, source));

        _stepsTotal = _fps * _ms / 1000;

        double delta = event->angleDelta().y();

        delta += delta * 2.5f * accerationRatio;

        _stepsLeftQueue.push_back(qMakePair(delta, _stepsTotal));
        _smoothTimer->start(1000 / _fps);
    }

public slots:
    void smoothMove(){
        double totalDelta = 0;

        for(QList< QPair<double, int> >::Iterator it = _stepsLeftQueue.begin(); it != _stepsLeftQueue.end(); ++it){
            double m = _stepsTotal / 2.0;
            double x = abs(_stepsTotal - it->second - m);
            double result = (cos(x * M_PI / m) + 1.0) / (2.0 * m) * it->first;

            totalDelta += result;
            --(it->second);
        }

        while(!_stepsLeftQueue.empty() && _stepsLeftQueue.begin()->second == 0)
            _stepsLeftQueue.pop_front();

        QWheelEvent e(_lastWheelEvent->position(), _lastWheelEvent->globalPosition(), QPoint(), QPoint(0, qRound(totalDelta)), _lastWheelEvent->buttons(), Qt::NoModifier, _lastWheelEvent->phase(), _lastWheelEvent->inverted(), _lastWheelEvent->source() );

        QApplication::sendEvent(verticalScrollBar(), &e);

        if (_stepsLeftQueue.empty())
            _smoothTimer->stop();
    }

private:
    int _fps, _ms;
    int _stepsTotal;

    QTimer *_smoothTimer;
    QList< QPair<double, int> > _stepsLeftQueue;
    std::unique_ptr<QWheelEvent> _lastWheelEvent;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////// CUSTOMHISTORYBUTTON ///////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomHistoryButton : public QPushButton{
    Q_OBJECT

public:
    explicit CustomHistoryButton(int id = -1){
        SetID(id);
    }

    int GetID(){
        return _id;
    }

    void SetID(int newID){
        if(newID != -1)
            _id = newID;
    }

private slots:
    void mousePressEvent(QMouseEvent *e){
        if (e->button() == Qt::LeftButton)
            emit clickedLeft(this);
        else if(e->button() == Qt::MiddleButton)
            emit clickedMiddle(this);
        else if(e->button()==Qt::RightButton)
            emit clickedRight(this);
    }

signals:
    void clickedLeft(CustomHistoryButton *button);
    void clickedMiddle(CustomHistoryButton *button);
    void clickedRight(CustomHistoryButton *button);

private:
    int _id = 0;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////// IMPORTTAKS //////////////////////////////
//////////////////////////////////////////////////////////////////////////

class ImageImportTask : public QRunnable{

public:
    ImageImportTask(const QString &filePath, const int iterationID, std::function<void(int, SaveManagerFileData)> updateFunc, std::atomic<bool> &cancelFlag)
        : _iteration(iterationID), _filePath(filePath), _cancelFlag(cancelFlag), _updateFunc(updateFunc) {}

    void run() override{
        if(_cancelFlag.load())
            return;

        if(!_filePath.isEmpty()){

            QImage loadImage;
            loadImage.load(_filePath);
            loadImage.setColorSpace(QColorSpace());

            SaveManagerFileData importImageData;
            importImageData.SetImage(loadImage);

            if(_cancelFlag.load())
                return;

            _updateFunc(_iteration, importImageData);
        }
    }

private:
    int _iteration = 0;
    QString _filePath;
    std::atomic<bool> &_cancelFlag;
    std::function<void(int, SaveManagerFileData)> _updateFunc;
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////// EXPORTTASK ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

class ImageExportTask : public QRunnable{

public:
    ImageExportTask(const QString &exportPath, const SaveManagerFileData &data, std::function<void()> updateFunc, std::atomic<bool> &cancelFlag)
        : _exportPath(exportPath), _data(data), _cancelFlag(cancelFlag), _updateFunc(updateFunc) {}

    void run() override{
        if(_cancelFlag.load())
            return;

        QPixmap image = _data.GetPixmap();
        QString fileName = QString("%1/%2.png").arg(_exportPath).arg(_data.GetID());

        if(!image.isNull() && image.save(fileName)){
            if(_cancelFlag.load())
                return;

            _updateFunc();
        }
    }

private:
    QString _exportPath;
    SaveManagerFileData _data;
    std::atomic<bool> &_cancelFlag;
    std::function<void()> _updateFunc;
};

//////////////////////////////////////////////////////////////////////////
////////////////////// CUSTOMIMAGEIMPORTEXPORTWORKER /////////////////////
//////////////////////////////////////////////////////////////////////////

class ImageImportExportWorker : public QObject{
    Q_OBJECT

public:
    ImageImportExportWorker(QObject *parent = nullptr) : QObject(parent), _threadPool(new QThreadPool(this)){}

    void CancelOperation(){
        _operation = OPERATION_NONE;
        _cancelFlag.store(true);
        _threadPool->clear();
    }

    void SetImportList(const QStringList &fileList){
        _importFileList = fileList;
        _operation = OPERATION_IMPORT;
    }

    void SetExportData(const QString &path, const QList<SaveManagerFileData> &exportData){
        _exportPath = path;
        _exportData = exportData;
        _operation = OPERATION_EXPORT;
    }

signals:
    void finished();
    void processExportUpdate();
    void processImportUpdate(int iteration, SaveManagerFileData imageData);

public slots:
    void doWork(){
        _cancelFlag.store(false);

        switch (_operation){
            case OPERATION_NONE:
                emit finished();
                return;

            case OPERATION_IMPORT:
                for(int i = 0; i < _importFileList.size(); ++i){
                    if (_cancelFlag.load()) break;

                    QString path = _importFileList.at(i);

                    ImageImportTask *task = new ImageImportTask(path, i, [this](int iteration, SaveManagerFileData data){
                            emit processImportUpdate(iteration, data);
                    }, _cancelFlag);

                    _threadPool->start(task);
                }

            break;

            case OPERATION_EXPORT:
                for(int i = 0; i < _exportData.size(); ++i){
                    if (_cancelFlag.load()) break;

                    SaveManagerFileData data = _exportData.at(i);

                    ImageExportTask *task = new ImageExportTask(_exportPath, data, [this](){
                            emit processExportUpdate();
                        }, _cancelFlag);

                    _threadPool->start(task);
                }

            break;
        }

        // Дожидаемся выполнения всех потоков
        _threadPool->waitForDone();

        emit finished();
    }

private:
    enum OPERATION_TYPE{
        OPERATION_NONE,
        OPERATION_IMPORT,
        OPERATION_EXPORT

    } _operation = OPERATION_NONE;

    QString _exportPath;
    QThreadPool *_threadPool;
    QStringList _importFileList;
    std::atomic<bool> _cancelFlag;
    QList<SaveManagerFileData> _exportData;
};

//////////////////////////////////////////////////////////////////////////
////////////////////// ----------------------------- /////////////////////
//////////////////////////////////////////////////////////////////////////

class ImageHistoryUpdateTask : public QRunnable{

public:
    ImageHistoryUpdateTask(CustomHistoryButton *button, SaveManagerFileData buttonData, int id, std::function<void()> updateFunc)
        : _buttonID(id), _button(button), _buttonData(buttonData), _updateFunc(updateFunc) {}

    void run() override{

        if(_button){
            _button->SetID(_buttonID); // Обновляем ID кнопки
            SetButtonImage(*_button, _buttonData); // Обновляем изображения кнопки

            if(_updateFunc)
                _updateFunc();
        }
    }

private:

    void SetButtonImage(CustomHistoryButton &button, SaveManagerFileData &data){
        // Получаем оригинальное изображение (и сразу подгоняем его под размер кнопки с соблюдением пропорций)
        QPixmap originalPixmap = data.GetPixmap();
        originalPixmap = originalPixmap.scaled(button.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if(data.GetHidden())
            originalPixmap = QPixmap::fromImage(GaussianBlur(originalPixmap.toImage(), 50, 2.2f, 1)); // [radius] [gamma] [quality]

        // Устанавливаем изображение в кнопку и указываем размер
        button.setIcon(QIcon(originalPixmap));
        button.setIconSize(button.size());
    }

    QImage GaussianBlur(const QImage &image, int radius, float gamma, int quality){
        // Convert pixmap to image
        QImage blurredImage = image;

        // Apply sRGB gamma correction (approximation)
        if(gamma != 1.0){

            blurredImage = blurredImage.convertToFormat(QImage::Format_ARGB32);

            for(int y = 0; y < blurredImage.height(); ++y){
                for(int x = 0; x < blurredImage.width(); ++x){

                    QColor color = blurredImage.pixelColor(x, y);

                    color.setRedF(std::pow(color.redF(), gamma));
                    color.setGreenF(std::pow(color.greenF(), gamma));
                    color.setBlueF(std::pow(color.blueF(), gamma));

                    blurredImage.setPixelColor(x, y, color);
                }
            }
        }

        // Determine sigma based on quality
        float sigma = radius / 2.0f;
        if(quality == 2) sigma = radius / 3.0f;
        else if(quality == 3) sigma = radius / 4.0f;
        else if(quality == 4) sigma = radius / 5.0f;

        // Utility function to create a Gaussian kernel
        auto createGaussianKernel = [](int radius, float sigma){
            int size = 2 * radius + 1;
            std::vector<float> kernel(size);
            float sum = 0.0f;

            for(int i = -radius; i <= radius; ++i){
                float value = std::exp(-0.5f * (i * i) / (sigma * sigma));
                kernel[i + radius] = value;
                sum += value;
            }

            // Normalize the kernel
            for(float &value : kernel)
                value /= sum;

            return kernel;
        };

        // Create Gaussian kernel
        std::vector<float> kernel = createGaussianKernel(radius, sigma);

        // Function to apply Gaussian blur on QImage
        auto applyGaussianBlur = [&](const QImage &image){
            QImage result(image.size(), image.format());

            int width = image.width();
            int height = image.height();

            // Temporary image to store intermediate results
            QImage tempImage(image);

            // Apply horizontal blur
            for(int y = 0; y < height; ++y){
                for(int x = 0; x < width; ++x){

                    float r = 0, g = 0, b = 0, a = 0;
                    float weightSum = 0;

                    for(int k = -radius; k <= radius; ++k){
                        int sampleX = std::clamp(x + k, 0, width - 1);
                        QColor color = image.pixelColor(sampleX, y);
                        float weight = kernel[k + radius];
                        r += color.redF() * weight;
                        g += color.greenF() * weight;
                        b += color.blueF() * weight;
                        a += color.alphaF() * weight;
                        weightSum += weight;
                    }

                    result.setPixelColor(x, y, QColor::fromRgbF(r / weightSum, g / weightSum, b / weightSum, a / weightSum));
                }
            }

            // Copy result to tempImage
            tempImage = result;

            // Apply vertical blur
            for(int y = 0; y < height; ++y){
                for(int x = 0; x < width; ++x){

                    float r = 0, g = 0, b = 0, a = 0;
                    float weightSum = 0;

                    for(int k = -radius; k <= radius; ++k){

                        int sampleY = std::clamp(y + k, 0, height - 1);
                        QColor color = tempImage.pixelColor(x, sampleY);
                        float weight = kernel[k + radius];

                        r += color.redF() * weight;
                        g += color.greenF() * weight;
                        b += color.blueF() * weight;
                        a += color.alphaF() * weight;

                        weightSum += weight;
                    }

                    result.setPixelColor(x, y, QColor::fromRgbF(r / weightSum, g / weightSum, b / weightSum, a / weightSum));
                }
            }

            return result;
        };

        // Apply Gaussian blur
        blurredImage = applyGaussianBlur(blurredImage);

        // Reverse sRGB gamma correction
        if(gamma != 1.0){

            for(int y = 0; y < blurredImage.height(); ++y){
                for(int x = 0; x < blurredImage.width(); ++x){

                    QColor color = blurredImage.pixelColor(x, y);

                    color.setRedF(std::pow(color.redF(), 1.0f / gamma));
                    color.setGreenF(std::pow(color.greenF(), 1.0f / gamma));
                    color.setBlueF(std::pow(color.blueF(), 1.0f / gamma));

                    blurredImage.setPixelColor(x, y, color);
                }
            }
        }

        return blurredImage;
    }

private:
    int _buttonID;
    CustomHistoryButton *_button;
    SaveManagerFileData _buttonData;
    std::function<void()> _updateFunc;
};

class ImageHistoryUpdateWorker : public QObject{
    Q_OBJECT

public:
    ImageHistoryUpdateWorker(QObject *parent = nullptr) : QObject(parent), _threadPool(new QThreadPool(this)){  }

    void SetData(QGridLayout *layout, int buttonsNum, QList<SaveManagerFileData> buttonsData, SaveManagerFileData modifiedDate){
        _layoutGrid = layout;
        _buttonsNum = buttonsNum;
        _buttonsData = buttonsData;
        _modifiedDate = modifiedDate;
    }

public slots:
    void doWork(){

        if(!_modifiedDate.GetNull()){

            // Изменение было только одной кнопки
            for(int i = 0; i < _buttonsNum; i++){

                // Изменённые данные найдены
                if(_buttonsData[i] == _modifiedDate){
                    CustomHistoryButton *button = GetButtonAtPosition(_layoutGrid, i / COLUMN_COUNT, i % COLUMN_COUNT);

                    if(button){
                        ImageHistoryUpdateTask *task = new ImageHistoryUpdateTask(button, _buttonsData[i], i, nullptr);
                        _threadPool->start(task);
                    }
                }
            }

        }else{

            // Изменения были всей истории
            for(int i = 0; i < _buttonsNum; i++){
                CustomHistoryButton *button = GetButtonAtPosition(_layoutGrid, i / COLUMN_COUNT, i % COLUMN_COUNT);

                if(button && !_buttonsData[i].GetData().isEmpty()){
                    ImageHistoryUpdateTask *task = new ImageHistoryUpdateTask(button, _buttonsData[i], i, nullptr);
                    _threadPool->start(task);
                }
            }
        }

        // Дожидаемся выполнения всех потоков
        _threadPool->waitForDone();

        emit finished();
    }

private:
    CustomHistoryButton* GetButtonAtPosition(QGridLayout *layout, int row, int col){
        QLayoutItem *item = layout->itemAtPosition(row, col);

        if (item){
            QWidget *widget = item->widget();

            if(widget)
                return qobject_cast<CustomHistoryButton*>(widget);
        }

        return nullptr;
    }

signals:
    void finished();

private:
    int _buttonsNum;
    QGridLayout *_layoutGrid;
    QThreadPool *_threadPool;
    SaveManagerFileData _modifiedDate;
    QList<SaveManagerFileData> _buttonsData;
};


class ImageHistoryUpdate : public QObject{
    Q_OBJECT

public:
    explicit ImageHistoryUpdate(QWidget *parent = nullptr) : QObject(parent){ /*-*/ }

    void StartButtonImageUpdate(QGridLayout *layout, int buttonsNum, QList<SaveManagerFileData> buttonsData, SaveManagerFileData modifiedDate, std::function<void()> Close_Func = nullptr){
        _worker = new ImageHistoryUpdateWorker();
        _worker->SetData(layout, buttonsNum, buttonsData, modifiedDate);

        QThread *workerThread = new QThread;
        _worker->moveToThread(workerThread);

        connect(workerThread, &QThread::started, _worker, &ImageHistoryUpdateWorker::doWork);
        connect(_worker, &ImageHistoryUpdateWorker::finished, workerThread, &QThread::quit);
        connect(_worker, &ImageHistoryUpdateWorker::finished, _worker, &ImageImportExportWorker::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        connect(workerThread, &QThread::finished, this, [this, Close_Func](){

            // Удаляем рабочий класс для осовобождения памяти
            QTimer::singleShot(3, this, [this, Close_Func](){
                if(Close_Func)
                    Close_Func();

                this->deleteLater();
            });
        });

        workerThread->start();
    }

private:
    ImageHistoryUpdateWorker *_worker;
};



//////////////////////////////////////////////////////////////////////////
////////////////////// CUSTOMIMAGEIMPORTEXPORTDIALOG /////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomImageImportExportDialog : public QDialog{
    Q_OBJECT

public:
    explicit CustomImageImportExportDialog(QWidget *parent = nullptr) : QDialog(parent){

        setParent(parent);
        setFixedSize(450, 150);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);

        // Создание объектов
        _progressBar = new QProgressBar(this);
        _progressLabel = new QLabel("0 / 0", this);
        _cancelButton = new QPushButton("Cancel", this);
        QLabel *titleLabel = new QLabel("[Image Import/Export]", this);

        // Создание фоновых виджетов
        QWidget *progressBackgroundWidget = new QWidget(this);
        progressBackgroundWidget->setStyleSheet("background-color: rgba(128, 128, 128, 50); border-radius: 5px;");

        // Создание слоев
        QHBoxLayout *_titleLayout = new QHBoxLayout();
        QHBoxLayout *_cancelLayout = new QHBoxLayout();
        QHBoxLayout *_progressLayout = new QHBoxLayout();
        QVBoxLayout *_mainVLayout = new QVBoxLayout(this);
        QVBoxLayout *_progressMainLayout = new QVBoxLayout(progressBackgroundWidget);

        // Добавление виджетов в слои
        _mainVLayout->addLayout(_titleLayout);
        _mainVLayout->addWidget(progressBackgroundWidget);
        _mainVLayout->addLayout(_cancelLayout);

        _titleLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
        _progressMainLayout->addLayout(_progressLayout);
        _progressLayout->addWidget(_progressBar, 7);
        _progressLayout->addWidget(_progressLabel, 1);
        _cancelLayout->addWidget(_cancelButton, 0, Qt::AlignCenter);

        // Центрирование кнопки
        _cancelLayout->setAlignment(Qt::AlignCenter);

        // Установка стилей
        titleLabel->setStyleSheet("color: white;");
        setStyleSheet("background-color: rgb(48, 48, 48);");
        _progressLabel->setStyleSheet("color: white; padding: 4px;");
        _progressLabel->setAlignment(Qt::AlignCenter);
        _progressBar->setStyleSheet(R"(
                                    QProgressBar{
                                        border: 2px solid grey;
                                        border-radius: 5px;
                                        text-align: center;
                                        color: black;
                                        background-color: white;
                                    }

                                    QProgressBar::chunk {
                                        width: 20px;
                                        background-color: green;
                                    }
        )");

        _cancelButton->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 10px;
                                        background-color: rgb(130, 50, 50);
                                        padding: 5px 15px;
                                    }

                                    QPushButton:hover{
                                        background-color: rgb(150, 70, 70);
                                    }
        )");

        _progressBar->setValue(0);
        _progressBar->setRange(0, 100);

        connect(_cancelButton, &QPushButton::clicked, this, &CustomImageImportExportDialog::CancelOperation);
    }

    void setProgressImport(int iteration, SaveManagerFileData imageData){
        _importedCurrentCount++;

        _progressBar->setValue( (( ((double)_importedCurrentCount) / _importNeedCount) * 100) );
        _progressLabel->setText(QString::number(_importedCurrentCount) + " / " + QString::number(_importNeedCount));

        if(imageData.GetPixmap().isNull()) // Не изображение
            return;

        emit importedImage(iteration, imageData);
    }

    void setProgressExport(){
        _exportedCurrentCount++;

        _progressBar->setValue( (( ((double)_exportedCurrentCount) / _exportNeedCount) * 100) );
        _progressLabel->setText(QString::number(_exportedCurrentCount) + " / " + QString::number(_exportNeedCount));
    }

    void StartImageImport(QStringList fileList, std::function<void(int iteration, SaveManagerFileData imageData)> ImageReturn_Func = nullptr, std::function<void(int import)> Close_Func = nullptr){
        _worker = new ImageImportExportWorker();
        _worker->SetImportList(fileList);

        QThread *workerThread = new QThread;
        _worker->moveToThread(workerThread);

        connect(workerThread, &QThread::started, _worker, &ImageImportExportWorker::doWork);
        connect(_worker, &ImageImportExportWorker::processImportUpdate, this, &CustomImageImportExportDialog::setProgressImport);
        connect(_worker, &ImageImportExportWorker::finished, workerThread, &QThread::quit);
        connect(_worker, &ImageImportExportWorker::finished, _worker, &ImageImportExportWorker::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
        connect(workerThread, &QThread::finished, this, &CustomImageImportExportDialog::CloseRequest);

        _importedCurrentCount = 0;
        _importNeedCount = fileList.count();

        if(ImageReturn_Func){
            connect(this, &CustomImageImportExportDialog::importedImage, [ImageReturn_Func](int iteration, SaveManagerFileData imageData){
                ImageReturn_Func(iteration, imageData);
            });
        }

        connect(this, &CustomImageImportExportDialog::CloseRequest, [this, Close_Func](){
            if(Close_Func)
                Close_Func(_importedCurrentCount);

            // Закрываем окно после загрузки
            QTimer::singleShot(250, this, [this](){
                this->close();
                this->deleteLater();
            });
        });

        _progressLabel->setText("0 / " + QString::number(_importNeedCount));

        show();
        workerThread->start();
    }

    void StartImageExport(QString path, QList<SaveManagerFileData> exportData, std::function<void(int exported)> Close_Func = nullptr){
        _worker = new ImageImportExportWorker();
        _worker->SetExportData(path, exportData);

        QThread *workerThread = new QThread;
        _worker->moveToThread(workerThread);

        connect(workerThread, &QThread::started, _worker, &ImageImportExportWorker::doWork);
        connect(_worker, &ImageImportExportWorker::processExportUpdate, this, &CustomImageImportExportDialog::setProgressExport);
        connect(_worker, &ImageImportExportWorker::finished, workerThread, &QThread::quit);
        connect(_worker, &ImageImportExportWorker::finished, _worker, &ImageImportExportWorker::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
        connect(workerThread, &QThread::finished, this, &CustomImageImportExportDialog::CloseRequest);

        _exportedCurrentCount = 0;
        _exportNeedCount = exportData.count();

        connect(this, &CustomImageImportExportDialog::CloseRequest, [this, Close_Func](){
            if(Close_Func)
                Close_Func(_exportedCurrentCount);

            // Закрываем окно после загрузки
            QTimer::singleShot(250, this, [this](){
                this->close();
                this->deleteLater();
            });
        });

        _progressLabel->setText("0 / " + QString::number(_exportNeedCount));

        show();
        workerThread->start();
    }

signals:
    void CloseRequest();
    void importedImage(int iteration, SaveManagerFileData imageData);

private slots:
    void CancelOperation(){
        if(_worker)
            _worker->CancelOperation();

        close();
    }

private:
    int _importNeedCount = 0, _exportNeedCount = 0;
    int _importedCurrentCount = 0, _exportedCurrentCount = 0;

    QLabel *_progressLabel;
    QProgressBar *_progressBar;
    QPushButton *_cancelButton;
    ImageImportExportWorker *_worker;
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////// CUSTOMCENTEREDQMENU //////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomCenteredQMenu : public QMenu{

public:
    using QMenu::QMenu;

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing);

        for (QAction *action : actions()) {
            QStyleOptionMenuItem opt;
            initStyleOption(&opt, action);
            QRect actionRect = actionGeometry(action);

            if (action->isSeparator()){
                opt.rect = actionRect;
                style()->drawControl(QStyle::CE_MenuItem, &opt, &painter, this);
                continue;
            }

            // Отрисовка фона
            opt.rect = actionRect;
            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, &painter, this);

            // Отрисовка текста по центру
            QString text = action->text();
            painter.setPen(opt.palette.text().color());
            painter.drawText(actionRect, Qt::AlignCenter, text);
        }
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////// CUSTOMIMAGEROTATEDIALOG /////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomImageRotateDialog : public QDialog{
    Q_OBJECT

public:
    explicit CustomImageRotateDialog(QWidget *parent = nullptr) : QDialog(parent){

        setParent(parent);
        setFixedSize(450, 550);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);

        // Создание обьектов
        _angleLabel = new QLabel("0°", this);
        _okButton = new QPushButton("ОК", this);
        _imageLabel = new QLabel("[Image]", this);
        _cancelButton = new QPushButton("Cancel", this);
        _angleSlider = new QSlider(Qt::Horizontal, this);

        // Создание слоев
        QWidget *_buttomBackground = new QWidget(this);
        QWidget *_sliderBackground = new QWidget(this);
        QVBoxLayout *_mainVLayout = new QVBoxLayout(this);
        QHBoxLayout* _sliderBackgroundLayout = new QHBoxLayout(_sliderBackground);
        QHBoxLayout* _buttomBackgroundLayout = new QHBoxLayout(_buttomBackground);

        // Формирование бутерброда слоев и виджетов
        _mainVLayout->addWidget(_imageLabel);
        _mainVLayout->addWidget(_sliderBackground);
        _mainVLayout->addWidget(_angleLabel);
        _mainVLayout->addWidget(_buttomBackground);

        _sliderBackgroundLayout->addWidget(_angleSlider);

        // Формирования нижней панели
        _buttomBackgroundLayout->addWidget(_okButton);
        _buttomBackgroundLayout->addWidget(_angleLabel);
        _buttomBackgroundLayout->addWidget(_cancelButton);

        // Установка правил
        _imageLabel->setAlignment(Qt::AlignCenter);
        _angleLabel->setAlignment(Qt::AlignCenter);

        _okButton->setMinimumHeight(25);
        _cancelButton->setMinimumHeight(25);

        _sliderBackground->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        _buttomBackground->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

        // Установка значений и стилей
        _angleSlider->setRange(0, 360);
        _angleSlider->setSingleStep(3);
        _angleSlider->setTickInterval(90);
        _angleSlider->setTickPosition(QSlider::TicksAbove);

        setStyleSheet("background-color: rgb(48, 48, 48);");

        _okButton->setStyleSheet(R"(
                                QPushButton{
                                    color: white;
                                    border-radius: 5px;
                                    background-color: rgb(56, 90, 127);
                                }

                                QPushButton:hover{
                                    background-color: rgb(80, 110, 150);
                                }
        )");

        _cancelButton->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 5px;
                                        background-color: rgb(68, 68, 68);
                                    }

                                    QPushButton:hover{
                                        background-color: rgb(90, 90, 90);
                                    }
        )");

        _buttomBackground->setStyleSheet("background-color: rgba(0, 0, 0, 25);");
        _sliderBackground->setStyleSheet("background-color: rgba(0, 0, 0, 25);");

        _imageLabel->setStyleSheet("border: 2px solid red; background-color: transparent;");
        _angleLabel->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 25);");

        _angleSlider->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 0);");

        // Подключение сигналов
        connect(_okButton, &QPushButton::clicked, this, &CustomImageRotateDialog::accept);
        connect(_cancelButton, &QPushButton::clicked, this, &CustomImageRotateDialog::reject);
        connect(_angleSlider, &QSlider::valueChanged, this, &CustomImageRotateDialog::onAngleChanged);
    }

    void setPixmap(QPixmap pixmap){
        _pixmap = pixmap;

        QPainter painter(&_pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::green, 5));
        painter.drawRect(0, 0, _pixmap.width(), _pixmap.height());

        // Обновляем отображение изображения в QLabel
        updateImage();
    }

    int getAngle(){
        return _angle;
    }

private:
    void updateImage(){
        QTransform transform;
        transform.rotate(getAngle());

        QPixmap bufferPixmap = _pixmap;
        bufferPixmap = bufferPixmap.scaled(QSize(350, 350), Qt::KeepAspectRatio);
        _imageLabel->setPixmap(bufferPixmap.transformed(transform, Qt::SmoothTransformation));
    }

private slots:
    void onAngleChanged(int value){

        /////////////////////////////////////////// Реализация лёгкого подмагничивания к определённым точкам
        const int threshold = 7;
        int snapPoints[] = {0, 90, 180, 270, 360};

        for(int snapPoint : snapPoints){
            if(abs(value - snapPoint) < threshold){
                value = snapPoint;

                _angleSlider->blockSignals(true);
                _angleSlider->setValue(value);
                _angleSlider->blockSignals(false);

                break;
            }
        }
        /////////////////////////////////////////// Реализация лёгкого подмагничивания к определённым точкам

        _angle = value;
        _angleLabel->setText(QString::number(value) + "°");

        // Обновляем отображение изображения в QLabel
        updateImage();
    }

private:
    int _angle = 0;

    QPixmap _pixmap;
    QSlider *_angleSlider;
    QLabel *_imageLabel, *_angleLabel;
    QPushButton *_okButton, *_cancelButton;
};

//////////////////////////////////////////////////////////////////////////
////////////////////////// CUSTOMDECISIONDIALOG //////////////////////////
//////////////////////////////////////////////////////////////////////////

class CustomDecisionDialog : public QDialog {
    Q_OBJECT

public:
    explicit CustomDecisionDialog(QWidget *parent = nullptr, QString text = "", QUrl icon = QUrl()) : QDialog(parent) {

        setParent(parent);
        resize(QSize(350, 115));
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);

        _okButton = new QPushButton("OK", this);
        _cancelButton = new QPushButton("Cancel", this);

        _okButton->setMinimumHeight(25);
        _cancelButton->setMinimumHeight(25);

        _okButton->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 5px;
                                        background-color: rgb(56, 90, 127);
                                    }

                                    QPushButton:hover{
                                        background-color: rgb(80, 110, 150);
                                    }
        )");

        _cancelButton->setStyleSheet(R"(
                                        QPushButton{
                                            color: white;
                                            border-radius: 5px;
                                            background-color: rgb(68, 68, 68);
                                        }

                                        QPushButton:hover{
                                            background-color: rgb(90, 90, 90);
                                        }
        )");

        setStyleSheet(R"(
                        QDialog{
                            background-color: #2b2b2b;
                            border: 1px solid #555;
                            border-radius: 5px;
                        }
        )");

        _messageLabel = new QLabel(this);

        QString htmlText =  "<table>"
                                "<tr>"
                                    "<td align=\"center\" style=\"padding-top: 10px; padding-bottom: 20px;\"><img src=\"" + icon.toString() + "\" width=\"35\" height=\"35\"></td>"
                                "</tr>"

                                "<tr>"
                                    "<td align=\"center\">" + text + "</td>"
                                "</tr>"
                            "</table>";

        _messageLabel->setText(htmlText);
        _messageLabel->setTextFormat(Qt::RichText);
        _messageLabel->setStyleSheet("color: white;");

        // Создаем сетку и добавляем виджеты
        _mainLayout = new QGridLayout(this);

        _mainLayout->addWidget(_messageLabel, 0, 0, 1, 2);
        _mainLayout->addWidget(_okButton, 1, 0);
        _mainLayout->addWidget(_cancelButton, 1, 1);

        _mainLayout->setVerticalSpacing(30);
        _mainLayout->setHorizontalSpacing(10);

        _mainLayout->setRowStretch(1, 1);
        _mainLayout->setAlignment(Qt::AlignCenter);

        // Подключаем сигналы
        connect(_okButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    }

private:
    QLabel *_messageLabel;
    QGridLayout *_mainLayout;
    QPushButton *_okButton, *_cancelButton;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////// EXPORTIMAGECUSTOMDIALOG /////////////////////////
//////////////////////////////////////////////////////////////////////////

class SaveImageCustomDialog : public QDialog{
    Q_OBJECT

public:
    SaveImageCustomDialog(QWidget *parent = nullptr, QString selectedPath = "", QString fileName = "", int indexDefault = 0) : QDialog(parent){

        setParent(parent);
        setFixedSize(215, 235);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Dialog);

        // Выделение памяти элементам
        _dirWidget = new QWidget(this);
        _fileName = new QLineEdit(this);
        _fileNameWidget = new QWidget(this);
        _dirLabel = new QLabel("Dir:", this);
        _formatComboBox = new QComboBox(this);
        _folderButton = new QPushButton(this);
        _okButton = new QPushButton("OK", this);
        _fileNameLabel = new QLabel("File Name:", this);
        _cancelButton = new QPushButton("Cancel", this);

        // Создаем некоторые особенные компоненты
        QFrame *line = new QFrame(this);
        QVBoxLayout *layout = new QVBoxLayout(this);
        QWidget *_buttomBackground = new QWidget(this);
        QVBoxLayout *_dirLayout = new QVBoxLayout(_dirWidget);
        QVBoxLayout *_fileNameLayout = new QVBoxLayout(_fileNameWidget);
        QHBoxLayout* _buttomBackgroundLayout = new QHBoxLayout(_buttomBackground);

        // Формирования слоя с компонентами
        layout->addWidget(_dirWidget);
        layout->addWidget(_fileNameWidget);
        layout->addSpacerItem(new QSpacerItem(7, 7, QSizePolicy::Minimum, QSizePolicy::Fixed));
        layout->addWidget(line);
        layout->addSpacerItem(new QSpacerItem(7, 7, QSizePolicy::Minimum, QSizePolicy::Fixed));
        layout->addWidget(_formatComboBox);
        layout->addWidget(_buttomBackground);

        // Настройка разделяющей линии
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setFixedHeight(5);
        line->setStyleSheet("background-color: rgba(99, 197, 255, 150);");

        // Формируем слой где QLabel и QPushButton будут вместе
        _dirLayout->addWidget(_dirLabel);
        _dirLayout->addWidget(_folderButton);
        _dirLayout->setContentsMargins(0, 0, 0, 0);

        // Формируем слой где QLabel и QLineEdit будут вместе
        _fileNameLayout->addWidget(_fileNameLabel);
        _fileNameLayout->addWidget(_fileName);
        _fileNameLayout->setContentsMargins(0, 0, 0, 0);

        // Добавляем кнопки в нижнюю часть окна
        _buttomBackgroundLayout->addWidget(_okButton);
        _buttomBackgroundLayout->addWidget(_cancelButton);
        _buttomBackgroundLayout->setContentsMargins(0, 0, 0, 0);
        _buttomBackground->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

        // Настройка бокса выбора
        _formatComboBox->addItems({"PNG", "JPEG", "BMP", "GIF", "PPM", "XBM", "XPM", "TIFF", "WEBP"});
        _formatComboBox->setCurrentIndex(indexDefault);

        // Настройка местополения на слое
        _dirLabel->setAlignment(Qt::AlignCenter);
        _fileNameLabel->setAlignment(Qt::AlignCenter);

        // Устанавливаем название файла и путь в кнопку
        _fileName->setText(fileName);
        setButtonPathString(selectedPath);

        // Стилизация кнопки принятия
        _okButton->setStyleSheet(R"(
                                    QPushButton{
                                        color: white;
                                        border-radius: 5px;
                                        background-color: rgb(56, 90, 127);
                                    }
                                    QPushButton:hover{
                                        background-color: rgb(80, 110, 150);
                                    }
        )");

        // Стилизация кнопки отмены
        _cancelButton->setStyleSheet(R"(
                                        QPushButton{
                                            color: white;
                                            border-radius: 5px;
                                            background-color: rgb(68, 68, 68);
                                        }
                                        QPushButton:hover{
                                            background-color: rgb(90, 90, 90);
                                        }
        )");

        // Стилизация текста Label "Пути"
        _dirLabel->setStyleSheet(R"(
                                    QLabel{
                                        color: white;
                                        background-color: rgba(0, 0, 0, 0);
                                    }
        )");

        // Стилизация текста Label "Названия файла"
        _fileNameLabel->setStyleSheet(R"(
                                        QLabel{
                                            color: white;
                                            background-color: rgba(0, 0, 0, 0);
                                        }
        )");

        // Стилизация поля ввода
        _fileName->setStyleSheet(R"(
                                    QLineEdit {
                                        color: white;
                                        background-color: #3a3a3a;
                                        border: 1px solid #555;
                                        border-radius: 5px;
                                        padding: 5px;
                                    }
                                    QLineEdit:focus {
                                        border: 1px solid #80aaff;
                                        background-color: #505050;
                                    }
        )");

        // Настройка стилей самого окна
        setStyleSheet(R"(
                        QDialog{
                            background-color: #2b2b2b;
                            border: 1px solid #555;
                            border-radius: 10px;
                        }

                        QPushButton{
                            color: white;
                            background-color: #3a3a3a;
                            border: 1px solid #555;
                            border-radius: 5px;
                            padding: 5px;
                        }

                        QPushButton:hover{
                            background-color: #505050;
                        }

                        QComboBox{
                            color: white;
                            background-color: #3a3a3a;
                            border: 1px solid #555;
                            border-radius: 0px;
                            padding: 5px;
                        }

                        QComboBox QAbstractItemView{
                            color: white;
                            background-color: #3a3a3a;
                            selection-background-color: #555;
                            border: 1px solid #555;
                        }
        )");

        // Подключение сигналов
        connect(_okButton, &QPushButton::clicked, this, &SaveImageCustomDialog::checkInput);
        connect(_cancelButton, &QPushButton::clicked, this, &CustomImageRotateDialog::reject);
        connect(_folderButton, &QPushButton::clicked, this, &SaveImageCustomDialog::chooseFolder);
    }

    QString GetFolderPath() const{
        return _folderPath;
    }

    QString GetFileName() const{
        return _fileName->text();
    }

    int GetImageFormatIndex() const{
        return _formatComboBox->currentIndex();
    }

    QString GetImageFormat() const{
        return _formatComboBox->currentText();
    }

    QString GetImageFormatWithDot() const{
        return "." + _formatComboBox->currentText().toLower();
    }

protected:
    void paintEvent(QPaintEvent *event) override{
        QDialog::paintEvent(event);

        int strokeWidth = 3;
        QRect _buttonDirRect, _fileNameEditRect;

        QPainter painter(this);
        painter.setPen(QPen(Qt::red, strokeWidth));

        if(_showNull){
            // Находим позицию кнопки
            if(_folderButton && _folderPath.isEmpty()){
                QRect buttomGeometry = _folderButton->geometry();
                QPoint buttonPos = mapFromGlobal(_dirWidget->mapToGlobal(buttomGeometry.topLeft()));

                _buttonDirRect = QRect(buttonPos.x(), buttonPos.y(), buttomGeometry.width(), buttomGeometry.height());
                _buttonDirRect.adjust(-strokeWidth, -strokeWidth, strokeWidth, strokeWidth);
            }else{
                _buttonDirRect = QRect();
            }

            // Находим позицию поля ввода
            if(_fileName && _fileName->text().isEmpty()){
                QRect buttomGeometry = _fileName->geometry();
                QPoint buttonPos = mapFromGlobal(_fileNameWidget->mapToGlobal(buttomGeometry.topLeft()));

                _fileNameEditRect = QRect(buttonPos.x(), buttonPos.y(), buttomGeometry.width(), buttomGeometry.height());
                _fileNameEditRect.adjust(-strokeWidth, -strokeWidth, strokeWidth, strokeWidth);
            }else
                _fileNameEditRect = QRect();
        }

        painter.drawRect(_buttonDirRect);
        painter.drawRect(_fileNameEditRect);
    }

private:
    void setButtonPathString(QString path){
        _folderPath = path;

        if(path != ""){
            _folderButton->setToolTip(_folderPath);

            _folderButton->setText(_folderPath);
            _folderButton->setStyleSheet(R"(
                                            QPushButton{
                                                text-align: left;
                                                padding-left: 5px;
                                            }
            )");
        }else{
            _folderButton->setToolTip("");

            _folderButton->setText("Choose Folder");
            _folderButton->setStyleSheet("");
        }
    }

private slots:
    void chooseFolder(){
        _folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", _folderPath);

        setButtonPathString(_folderPath);
    }

    void checkInput(){
        if(_folderPath.isEmpty() || _fileName->text().isEmpty()){
            _showNull = true;
            repaint();

            QTimer::singleShot(1500, this, [this](){
                _showNull = false;
                repaint();
            });
        }else
            accept();
    }

private:
    bool _showNull = false;

    QString _folderPath;
    QLineEdit *_fileName;
    QComboBox *_formatComboBox;
    QLabel *_dirLabel, *_fileNameLabel;
    QWidget *_dirWidget, *_fileNameWidget;
    QPushButton *_okButton, *_cancelButton, *_folderButton;
};

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////// END //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif // SCREENSHOTHISTORY_H
