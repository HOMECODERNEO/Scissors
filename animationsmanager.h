#ifndef ANIMATIONSMANAGER_H
#define ANIMATIONSMANAGER_H

#include <QWidget>
#include <QObject>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class AnimationsManager : public QObject{
    Q_OBJECT

    enum{
        NONE,
        WINDOW_OPACITY,
        OBJECT_OPACITY,
        OBJECT_GEOMETRY
    } _animationType = NONE;

public:
    explicit AnimationsManager(QObject *parent = nullptr);

    AnimationsManager& Create_WindowOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, int startValue, int endValue);
    AnimationsManager& Create_ObjectOpacity(QWidget *Object, std::function<void()> endAnimation_Func, int duration, int startValue, int endValue);
    AnimationsManager& Create_ObjectGeometry(QWidget *Object, std::function<void()> endAnimation_Func, int duration, QRect startValue, QRect endValue);


    void Start();
    void Stop();

    void Clear();
    void Restart();

    void Pause();
    void Resume();

private:
    QRect _startValue_ObjectGeometry;
    int _startValue_WindowOpacity = 0;
    int _startValue_ObjectOpacity = 0;

    QPropertyAnimation *_animation;
};

#endif // ANIMATIONSMANAGER_H
