#include "ChangeDateTimeFrm.h"
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QDateTime>

class ChangeDateTimeFrmPrivate
{
    Q_DECLARE_PUBLIC(ChangeDateTimeFrm)
private:
    ChangeDateTimeFrmPrivate(ChangeDateTimeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pAlterButton;//更改
    QPushButton *m_pCancelButton;//取消
    QComboBox *m_pYearComBox;//年
    QComboBox *m_pMonthComBox;//月
    QComboBox *m_pDayComBox;//日
    QComboBox *m_pHourComBox;//时
    QComboBox *m_pMinuteComBox;//分
private:
    ChangeDateTimeFrm *const q_ptr;
};

ChangeDateTimeFrmPrivate::ChangeDateTimeFrmPrivate(ChangeDateTimeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ChangeDateTimeFrm::ChangeDateTimeFrm(QWidget *parent)
    : QDialog(parent, Qt::WindowCloseButtonHint)
    , d_ptr(new ChangeDateTimeFrmPrivate(this))
{

}

ChangeDateTimeFrm::~ChangeDateTimeFrm()
{

}

void ChangeDateTimeFrmPrivate::InitUI()
{
    m_pAlterButton = new QPushButton;//更改
    m_pCancelButton = new QPushButton;//取消
    m_pYearComBox = new QComboBox;//年
    m_pMonthComBox = new QComboBox;//月
    m_pDayComBox = new QComboBox;//日
    m_pHourComBox = new QComboBox;//时
    m_pMinuteComBox = new QComboBox;//分

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(QObject::tr("ModifyDateAndTime")));//更改日期和时间
    hlayout->addStretch();

    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addWidget(new QLabel(QObject::tr("Date")));//日期
    hlayout1->addStretch();

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(m_pYearComBox);
    hlayout2->addSpacing(20);
    hlayout2->addWidget(m_pMonthComBox);
    hlayout2->addSpacing(20);
    hlayout2->addWidget(m_pDayComBox);
    hlayout2->addStretch();


    QHBoxLayout *hlayout3 = new QHBoxLayout;
    hlayout3->addWidget(new QLabel(QObject::tr("Time")));//时间
    hlayout3->addStretch();

    QHBoxLayout *hlayout4 = new QHBoxLayout;
    hlayout4->addWidget(m_pHourComBox);
    hlayout4->addSpacing(20);
    hlayout4->addWidget(m_pMinuteComBox);
    hlayout4->addStretch();

    QHBoxLayout *hlayout5 = new QHBoxLayout;
    hlayout5->addStretch();
    hlayout5->addWidget(m_pAlterButton);
    hlayout5->addSpacing(20);
    hlayout5->addWidget(m_pCancelButton);

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setContentsMargins(30, 20, 20, 30);
    vlayout->setSpacing(0);
    vlayout->addLayout(hlayout);
    vlayout->addStretch();
    vlayout->addLayout(hlayout1);
    vlayout->addLayout(hlayout2);
    vlayout->addStretch();
    vlayout->addLayout(hlayout3);
    vlayout->addLayout(hlayout4);
    vlayout->addStretch();
    vlayout->addLayout(hlayout5);
}

void ChangeDateTimeFrmPrivate::InitData()
{
    q_func()->setWindowTitle(QObject::tr("ModifyDateAndTime"));//更改日期和时间
    q_func()->setFixedSize(550, 344);
    q_func()->setObjectName("ChangeDateTimeFrm");

    m_pAlterButton->setText(QObject::tr("Modify"));//更改
    m_pCancelButton->setText(QObject::tr("Cancel"));//取消

    m_pAlterButton->setFixedSize(100, 52);
    m_pCancelButton->setFixedSize(100, 52);

    m_pYearComBox->setFixedSize(148, 42);
    m_pMonthComBox->setFixedSize(148, 42);
    m_pDayComBox->setFixedSize(148, 42);
    m_pHourComBox->setFixedSize(110, 42);
    m_pMinuteComBox->setFixedSize(110, 42);

    QDateTime dateTime = QDateTime::currentDateTime();

    for(int i = 1991; i<2052; i++)
        m_pYearComBox->addItem(QString::number(i) + QObject::tr("Year"));//年
    m_pYearComBox->setCurrentText(QObject::tr("%1Year").arg(dateTime.date().year()));//%1年

    for(int i = 1; i<13; i++)
        m_pMonthComBox->addItem(QString::number(i) + QObject::tr("Month"));//月
    m_pMonthComBox->setCurrentText(QObject::tr("%1Month").arg(dateTime.date().month()));//%1月

    for(int i = 1; i<32; i++)
        m_pDayComBox->addItem(QString::number(i) + QObject::tr("Day"));//日
    m_pDayComBox->setCurrentText(QObject::tr("%1Day").arg(dateTime.date().day()));//%1日

    for(int i = 0; i<24; i++)
        m_pHourComBox->addItem(QString::asprintf("%0.2d", i));
    m_pHourComBox->setCurrentText(QString::asprintf("%0.2d", dateTime.time().hour()));

    for(int i = 0; i<60; i++)
        m_pMinuteComBox->addItem(QString::asprintf("%0.2d", i));
    m_pMinuteComBox->setCurrentText(QString::asprintf("%0.2d", dateTime.time().minute()));
}

void ChangeDateTimeFrmPrivate::InitConnect()
{
    QObject::connect(m_pAlterButton, &QPushButton::clicked, [&] {
#ifdef Q_OS_LINUX
        QString Year = m_pYearComBox->currentText().left(4);//年
        //QString Month = m_pMonthComBox->currentText().left(2);//月
		QString Month = m_pMonthComBox->currentText().left(m_pMonthComBox->currentText().size() - 1);//月

        QString Day = m_pDayComBox->currentText().left(m_pDayComBox->currentText().size() - 1);//日
        QString Hour = m_pHourComBox->currentText().left(2);//时
        QString Minute = m_pMinuteComBox->currentText().left(2);//分

        QString setDate = QString("date -s \"%1-%2-%3 %4:%5:00\"").arg(Year).arg(Month).arg(Day).arg(Hour).arg(Minute);		
        system(setDate.toLatin1().data());
        system("hwclock -w -u");
#endif
        q_func()->done(0);
    });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [&] {
        q_func()->done(1);
    });
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void ChangeDateTimeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 