#ifndef FACEHOMETITLEFRM_H
#define FACEHOMETITLEFRM_H

#include <QtWidgets/QWidget>

class FaceHomeTitleFrmPrivate;
class FaceHomeTitleFrm : public QWidget
{
    Q_OBJECT
public:
    explicit FaceHomeTitleFrm(QWidget *parent = nullptr);
    ~FaceHomeTitleFrm();
public:
    void setTitleText(const QString &text);
    void setLinkState(const bool &, const int &type);
       // Add this new method for server time updates
    void setServerTime(const QDateTime &serverTime);
    void setSubtitleText(const QString &text);
void setSystemStatus(const QString &status, bool isOnline = true);
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<FaceHomeTitleFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(FaceHomeTitleFrm)
    Q_DISABLE_COPY(FaceHomeTitleFrm)
};

#endif // FACEHOMETITLEFRM_H
