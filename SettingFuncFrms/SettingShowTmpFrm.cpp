#include "SettingShowTmpFrm.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class SettingShowTmpFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingShowTmpFrm)
public:
    SettingShowTmpFrmPrivate(SettingShowTmpFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftPicLabel;
    QLabel *m_pNameLabel;
    QLabel *m_pRightPicLabel;
private:
    SettingShowTmpFrm *const q_ptr;
};

SettingShowTmpFrmPrivate::SettingShowTmpFrmPrivate(SettingShowTmpFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SettingShowTmpFrm::SettingShowTmpFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SettingShowTmpFrmPrivate(this))
{

}

SettingShowTmpFrm::~SettingShowTmpFrm()
{

}

void SettingShowTmpFrmPrivate::InitUI()
{
    m_pLeftPicLabel = new QLabel;
    m_pNameLabel = new QLabel;
    m_pRightPicLabel = new QLabel;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(15);
    layout->addWidget(m_pLeftPicLabel);
    layout->addSpacing(10);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pRightPicLabel);
    layout->addSpacing(20);
}

void SettingShowTmpFrmPrivate::InitData()
{

}

void SettingShowTmpFrmPrivate::InitConnect()
{

}

void SettingShowTmpFrm::setData(const QString &Name, const QString &qstrPic)
{
    Q_D(SettingShowTmpFrm);
    d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    d->m_pNameLabel->setText(Name);
    d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/ListWidgetTmpRight.png"));
}

#ifdef SCREENCAPTURE  //ScreenCapture   
void SettingShowTmpFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif