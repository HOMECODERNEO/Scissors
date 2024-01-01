#include "animationsmanager.h"

AnimationsManager::AnimationsManager(QObject *parent): QObject{ parent }{
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AnimationsManager::Clear(){
    _animation = nullptr;
    _animationType = NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationsManager& AnimationsManager::Create_WindowOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, int startValue, int endValue){
    if(_animation)
        Clear();

    _animation = new QPropertyAnimation(Object, "windowOpacity");

    _animation->setDuration(duration);

    _animation->setStartValue(startValue);
    _animation->setEndValue(endValue);

    if(endAnimation_Func)
        connect(_animation, &QPropertyAnimation::finished, endAnimation_Func);

    _startValue_WindowOpacity = startValue;

    return *this;
}

AnimationsManager& AnimationsManager::Create_ObjectOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, int startValue, int endValue){
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

    return *this;
}

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

    return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AnimationsManager::Start(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::State::Stopped)
        _animation->start();
}

void AnimationsManager::Stop(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Running)
        _animation->stop();
}

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

void AnimationsManager::Pause(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Running)
        _animation->pause();
}

void AnimationsManager::Resume(){
    if(_animation != nullptr && _animation->state() == QPropertyAnimation::Paused)
        _animation->resume();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

