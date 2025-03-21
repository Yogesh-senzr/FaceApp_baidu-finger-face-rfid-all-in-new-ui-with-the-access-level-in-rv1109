#ifndef VIEWACCESSRECORDSMODEL_H
#define VIEWACCESSRECORDSMODEL_H

#include <QWidget>
//通行记录模型
class ViewAccessRecordsModelPrivate;
class ViewAccessRecordsModel : public QWidget
{
    Q_OBJECT
public:
    explicit ViewAccessRecordsModel(QWidget *parent = nullptr);
    ~ViewAccessRecordsModel();
public:
    void setData(const QString &name, const QString &sex, const QString &temperature, const QString &timer, const QString &path);
private:
    QScopedPointer<ViewAccessRecordsModelPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture 
    void mouseDoubleClickEvent(QMouseEvent*);   
#endif        
private:
    Q_DECLARE_PRIVATE(ViewAccessRecordsModel)
    Q_DISABLE_COPY(ViewAccessRecordsModel)
};

#endif // VIEWACCESSRECORDSMODEL_H
