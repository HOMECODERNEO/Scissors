#ifndef CUSTOMHISTORYBUTTON_H
#define CUSTOMHISTORYBUTTON_H

#include <Includes.h>

class CustomHistoryButton : public QPushButton{
    Q_OBJECT

public:
    explicit CustomHistoryButton(int id = -1,
                                 std::function<void()> leftClick_Func = nullptr,
                                 std::function<void()> middleClick_Func = nullptr,
                                 std::function<void()> rightClick_Func = nullptr);

private slots:
    void mousePressEvent(QMouseEvent *e);

signals:
    void clickedLeft(int id);
    void clickedMiddle(int id);
    void clickedRight(int id);

private:
    int _id = 0;
};

#endif // CUSTOMHISTORYBUTTON_H
