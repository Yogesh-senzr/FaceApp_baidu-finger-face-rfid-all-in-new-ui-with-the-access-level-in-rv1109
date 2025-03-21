#include "SettingMenuFrm.h"
#include "SettingMenuTitleFrm.h"

#include "HomeFrm.h"
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

class SettingMenuFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingMenuFrm)
public:
    SettingMenuFrmPrivate(SettingMenuFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    HomeFrm *m_pHomeFrm;
private:
    SettingMenuTitleFrm *m_pSettingMenuTitleFrm;
    QStackedWidget *m_pStackedWidget;
private:
    SettingMenuFrm *const q_ptr;
};


SettingMenuFrmPrivate::SettingMenuFrmPrivate(SettingMenuFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SettingMenuFrm::SettingMenuFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SettingMenuFrmPrivate(this))
{

}

SettingMenuFrm::~SettingMenuFrm()
{

}

void SettingMenuFrmPrivate::InitUI()
{
    m_pSettingMenuTitleFrm = new SettingMenuTitleFrm;
    m_pStackedWidget = new QStackedWidget;

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->setSpacing(0);
    malayout->addWidget(m_pSettingMenuTitleFrm);
    malayout->addWidget(m_pStackedWidget);

    m_pHomeFrm = new HomeFrm;
    m_pStackedWidget->addWidget(m_pHomeFrm);
}

void SettingMenuFrmPrivate::InitData()
{
    q_func()->setObjectName("SettingMenuFrm");
    m_pSettingMenuTitleFrm->setFixedHeight(120);
}

void SettingMenuFrmPrivate::InitConnect()
{

}

void SettingMenuFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture 
void SettingMenuFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}	
#endif 