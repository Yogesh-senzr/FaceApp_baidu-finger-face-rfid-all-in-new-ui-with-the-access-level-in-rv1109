#ifndef LRHealthCode_H
#define LRHealthCode_H

#include <QtCore/QThread>
#include "SharedInclude/GlobalDef.h"


//二维码
class LRHealthCodePrivate;
class LRHealthCode : public QThread
{
    Q_OBJECT
public:
    explicit LRHealthCode(QObject *parent = nullptr);
    ~LRHealthCode();

public:
    Q_SIGNAL void sigShowYKHealthCode( HEALTINFO_t info) const;
    Q_SIGNAL void sigLRHealthCodeMsg( HEALTINFO_t info) const;
private:
    void run();
private:
    QScopedPointer<LRHealthCodePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(LRHealthCode)
    Q_DISABLE_COPY(LRHealthCode)
};

#endif // LRHealthCode_H
