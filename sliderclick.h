#ifndef SLIDERCLICK_H
#define SLIDERCLICK_H

#include <QMouseEvent>
#include <QCoreApplication>
#include <QSlider>

//支持拖拽和点击的slider
class SliderClick : public QSlider
{
    Q_OBJECT
public:
    SliderClick(Qt::Orientation orientation, QWidget *parent = 0) : QSlider(orientation, parent)
    {
    }
    ~SliderClick()
    {

    }
protected:
    void mousePressEvent(QMouseEvent *ev)
    {
        //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
        QSlider::mousePressEvent(ev);
        //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
        double pos = ev->pos().x() / (double)width();
        double value = pos * (maximum() - minimum()) + minimum();
        //value + 0.5 四舍五入
        setValue(value + 0.5);
        //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
        QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
        QCoreApplication::sendEvent(parentWidget(), &evEvent);
    }
    void mouseReleaseEvent(QMouseEvent *ev)
    {
        QSlider::mouseReleaseEvent(ev);
        QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 2));
        QCoreApplication::sendEvent(parentWidget(), &evEvent);
    }
};

#endif // SLIDERCLICK_H
