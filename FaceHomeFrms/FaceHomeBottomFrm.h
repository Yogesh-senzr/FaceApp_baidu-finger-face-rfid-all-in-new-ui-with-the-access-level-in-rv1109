// Make sure your FaceHomeBottomFrm.h has these includes and declarations:

#ifndef FACEHOMEBOTTOMFRM_H
#define FACEHOMEBOTTOMFRM_H

#include "FaceHomeFrms/HomeBottomBaseFrm.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QMouseEvent;  // Forward declaration
QT_END_NAMESPACE

class FaceHomeBottomFrmPrivate;

class FaceHomeBottomFrm : public HomeBottomBaseFrm
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FaceHomeBottomFrm)

public:
    explicit FaceHomeBottomFrm(QWidget *parent = nullptr);
    ~FaceHomeBottomFrm();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;  // Add this declaration

public slots:
    // New sync-related methods
    void setTenantName(const QString &tenantName);
    void setSyncUserCount(int currentCount, int totalCount);
    void setSyncStatus(const QString &status);
    void setLastSyncTime(const QString &time);
    void setLocalFaceCount(int localCount, int totalCount);  // New method for local face count
    void setNetInfo(const QString &address, const QString &make);
private:
    FaceHomeBottomFrmPrivate *const d_ptr;
};

#endif // FACEHOMEBOTTOMFRM_H