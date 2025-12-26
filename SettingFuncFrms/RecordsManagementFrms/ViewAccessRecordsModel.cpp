#include "ViewAccessRecordsModel.h"

#include <QLabel>
#include <QHBoxLayout>

class ViewAccessRecordsModelPrivate
{
    Q_DECLARE_PUBLIC(ViewAccessRecordsModel)
public:
    ViewAccessRecordsModelPrivate(ViewAccessRecordsModel *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;
    QLabel *m_pSexLabel;
    QLabel *m_pTemperatureLabel;
    QLabel *m_pTransitTimeLabel;
    QLabel *m_pHumanFaceLabel;
private:
    ViewAccessRecordsModel *const q_ptr;
};

ViewAccessRecordsModelPrivate::ViewAccessRecordsModelPrivate(ViewAccessRecordsModel *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

ViewAccessRecordsModel::ViewAccessRecordsModel(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ViewAccessRecordsModelPrivate(this))
{

}

ViewAccessRecordsModel::~ViewAccessRecordsModel()
{

}

void ViewAccessRecordsModelPrivate::InitUI()
{
    m_pNameLabel = new QLabel;
    m_pSexLabel = new QLabel;
    m_pTemperatureLabel = new QLabel;
    m_pTransitTimeLabel = new QLabel;
    m_pHumanFaceLabel = new QLabel;

    QFrame *f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("ViewAccessRecordsModelFrm");
    f->setFixedWidth(1);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_pNameLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("ViewAccessRecordsModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pSexLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("ViewAccessRecordsModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pTemperatureLabel);
    layout->addWidget(f);

    f = new QFrame;
    f->setFrameShape(QFrame::VLine);
    f->setObjectName("ViewAccessRecordsModelFrm");
    f->setFixedWidth(1);
    layout->addWidget(m_pTransitTimeLabel);
    layout->addWidget(f);

    layout->addWidget(m_pHumanFaceLabel);

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->setSpacing(0);
    malayout->addLayout(layout);
}

void ViewAccessRecordsModelPrivate::InitData()
{
    m_pTransitTimeLabel->setWordWrap(true);
    m_pNameLabel->setAlignment(Qt::AlignCenter);
    m_pSexLabel->setAlignment(Qt::AlignCenter);
    m_pTemperatureLabel->setAlignment(Qt::AlignCenter);
    m_pTransitTimeLabel->setAlignment(Qt::AlignCenter);
    m_pHumanFaceLabel->setAlignment(Qt::AlignCenter);


    m_pNameLabel->setFixedWidth(132);
    m_pSexLabel->setFixedWidth(80);
    m_pTemperatureLabel->setFixedWidth(112);
    m_pTransitTimeLabel->setFixedWidth(144);
    m_pHumanFaceLabel->setFixedWidth(180);

    //    m_pNameLabel->setStyleSheet("background:red");
    //    m_pSexLabel->setStyleSheet("background:green");
    //    m_pTemperatureLabel->setStyleSheet("background:blue");
    //    m_pTransitTimeLabel->setStyleSheet("background:yellow");
    //    m_pHumanFaceLabel->setStyleSheet("background:black");

    m_pNameLabel->setStyleSheet("font-size:24px;");
    m_pSexLabel->setStyleSheet("font-size:24px;");
    m_pTemperatureLabel->setStyleSheet("font-size:24px;");
    m_pTransitTimeLabel->setStyleSheet("font-size:24px;");
    m_pHumanFaceLabel->setStyleSheet("font-size:24px;");
}

void ViewAccessRecordsModelPrivate::InitConnect()
{

}

void ViewAccessRecordsModel::setData(const QString &name, const QString &sex, const QString &temperature, const QString &timer, const QString &path)
{
    Q_D(ViewAccessRecordsModel);
    d->m_pNameLabel->setText(name);
    d->m_pSexLabel->setText(sex);
    d->m_pTemperatureLabel->setText(temperature);
    d->m_pTransitTimeLabel->setText(timer);

    QPixmap png = QPixmap(path);
    if(!png.isNull())png = png.scaled(180, 110);
    d->m_pHumanFaceLabel->setPixmap(png);
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void ViewAccessRecordsModel::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif