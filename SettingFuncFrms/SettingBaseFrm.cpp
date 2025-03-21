#include "SettingBaseFrm.h"

#include <QEvent>
#include <QDebug>

SettingBaseFrm::SettingBaseFrm(QWidget *parent) : QWidget(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
}

bool SettingBaseFrm::event(QEvent *event)
{
    if(event->type() == QEvent::ContextMenu)
        emit sigShowLevelFrm();
    return QWidget::event(event);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void SettingBaseFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif 