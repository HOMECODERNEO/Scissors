#include "customhistorybutton.h"

CustomHistoryButton::CustomHistoryButton(int id, std::function<void()> leftClick_Func, std::function<void()> middleClick_Func, std::function<void()> rightClick_Func){
    if(id != -1)
        _id = id;

    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
    if(leftClick_Func){
        connect(this, &CustomHistoryButton::clickedLeft, [leftClick_Func](){
            leftClick_Func();
        });
    }

    if(middleClick_Func){
        connect(this, &CustomHistoryButton::clickedMiddle, [middleClick_Func](){
            middleClick_Func();
        });
    }

    if(rightClick_Func){
        connect(this, &CustomHistoryButton::clickedRight, [rightClick_Func](){
            rightClick_Func();
        });
    }
    //////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////
}

void CustomHistoryButton::mousePressEvent(QMouseEvent *e){
    if (e->button() == Qt::LeftButton)
        emit clickedLeft(_id);
    else if(e->button() == Qt::MiddleButton)
        emit clickedMiddle(_id);
    else if(e->button()==Qt::RightButton)
        emit clickedRight(_id);
}


