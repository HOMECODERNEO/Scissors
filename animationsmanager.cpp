#include "animationsmanager.h"

// Конструктор
AnimationsManager::AnimationsManager(QObject *parent): QObject{ parent }{
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Очистка анимации
void AnimationsManager::Clear(){
    _animation = nullptr;
    _animationType = NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Создание анимации прозрачности окна
AnimationsManager& AnimationsManager::Create_WindowOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, float startValue, float endValue){
    if(_animation)
        Clear();

    _animation = new QPropertyAnimation(Object, "windowOpacity");

    _animation->setDuration(duration);

    _animation->setStartValue(startValue);
    _animation->setEndValue(endValue);

    if(endAnimation_Func)
        connect(_animation, &QPropertyAnimation::finished, endAnimation_Func);

    _startValue_WindowOpacity = startValue;
    _animationType = WINDOW_OPACITY;

    return *this;
}

// Создание анимации прозрачности обьекта
AnimationsManager& AnimationsManager::Create_ObjectOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, float startValue, float endValue){
    if(_animation)
        Clear();

    QGraphicsOpacityEffect* fade_effect = new QGraphicsOpacityEffect(Object);
    Object->setGraphicsEffect(fade_effect);

    _animation = new QPropertyAnimation(fade_effect, "opacity");

    _animation->setDuration(duration);
    _animation->setStartValue(startValue);
    _animation->setEndValue(endValue);

    if(endAnimation_Func)
        connect(_animation, &QPropertyAnimation::finished, endAnimation_Func);

    _startValue_ObjectOpacity = startValue;
    _animationType = OBJECT_OPACITY;

    return *this;
}

// Создание анимации геометрии обьекта
AnimationsManager& AnimationsManager::Create_ObjectGeometry(QWidget *Object, std::function<void()> endAnimation_Func, int duration, QRect startValue, QRect endValue){
    if(_animation)
        Clear();

    _animation = new QPropertyAnimation(Object, "geometry");

    _animation->setDuration(duration);
    _animation->setStartValue(startValue);
    _animation->setEndValue(endValue);

    if(endAnimation_Func)
        connect(_animation, &QPropertyAnimation::finished, endAnimation_Func);

    _startValue_ObjectGeometry = startValue;
    _animationType = OBJECT_GEOMETRY;

    return *this;
}

QPropertyAnimation& AnimationsManager::Create_StackedWidgetOpacity(QWidget *widget, std::function<void()> endAnimation_Func, int duration, float startValue, float endValue){

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnimation->setDuration(duration);
    fadeOutAnimation->setStartValue(startValue);
    fadeOutAnimation->setEndValue(endValue);

    if(endAnimation_Func){
        connect(fadeOutAnimation, &QPropertyAnimation::finished, endAnimation_Func);
    }

    return *fadeOutAnimation;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Запуск анимации
void AnimationsManager::Start(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::State::Stopped)
        _animation->start();
}

// Остановка анимации
void AnimationsManager::Stop(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Running)
        _animation->stop();
}

// Перезапуск анимации
void AnimationsManager::Restart(){
    if(_animation != nullptr){
        Stop();

        switch(_animationType){
            case WINDOW_OPACITY:{ _animation->setStartValue(_startValue_WindowOpacity); break; }
            case OBJECT_OPACITY:{ _animation->setStartValue(_startValue_ObjectOpacity); break; }
            case OBJECT_GEOMETRY:{ _animation->setStartValue(_startValue_ObjectGeometry); break; }
            default:{ break; }
        }

        Start();
    }
}

// Пауза анимации
void AnimationsManager::Pause(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Running)
        _animation->pause();
}

// Снятие анимации с паузы
void AnimationsManager::Resume(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Paused)
        _animation->resume();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
