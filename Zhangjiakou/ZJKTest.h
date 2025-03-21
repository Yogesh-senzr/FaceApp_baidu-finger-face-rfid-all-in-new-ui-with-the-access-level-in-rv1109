#ifndef ZJKTEST_H
#define ZJKTEST_H

#include <QObject>

//测试类不作为正式使用
class ZJKTest : public QObject
{
    Q_OBJECT
public:
    explicit ZJKTest(QObject *parent = nullptr);
    static ZJKTest *GetInstnce(){static ZJKTest g;return &g;}
signals:

public slots:
};

#endif // ZJKTEST_H
