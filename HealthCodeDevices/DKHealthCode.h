#ifndef DKHealthCode_H_
#define DKHealthCode_H_

#include <QThread>

//DK平台健康码接口
class DKHealthCodePrivate;
class DKHealthCode: public QThread
{
    Q_OBJECT
public:
    DKHealthCode(QObject *parent = Q_NULLPTR);
    ~DKHealthCode();
public:
    void setQueryHealthCode(const QString &name, const QString &idCard, const QString &sex);
public:
    Q_SIGNAL void sigDKHealthCodeMsg(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);
private:
    //-1二维码头未插入或初始化失败
    int IdentityCheckData(unsigned char *name, unsigned char *id, unsigned char *publicKey, unsigned int publicKeyLen,
                           unsigned char *publicVer, unsigned char *randomNum, char *jsonData);
private:
    void run();
private:
    QScopedPointer<DKHealthCodePrivate>d_ptr;
private:
    Q_DISABLE_COPY(DKHealthCode)
    Q_DECLARE_PRIVATE(DKHealthCode)
};

#endif /* DKHealthCode_H_ */
