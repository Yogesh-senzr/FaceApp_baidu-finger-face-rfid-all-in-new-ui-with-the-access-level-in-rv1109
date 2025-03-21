#ifndef ZHANGJIAKOUHEALTHCODE_H
#define ZHANGJIAKOUHEALTHCODE_H

#include <QObject>

//张家口健康码状态
class ZhangjiakouHealthCodePrivate;
class ZhangjiakouHealthCode : public QObject
{
    Q_OBJECT
public:
    explicit ZhangjiakouHealthCode(QObject *parent = nullptr);
    ~ZhangjiakouHealthCode();
public:
    Q_SLOT void slotQueryHealthCode(const QString name, const QString idCard, const QString sex);
    //上传温度记录到张家口
    Q_SLOT void slotPostTempRecognition(const QString path,  const QString personName, const QString idCard, const QString sex
                                        , const int deviceType, const int checkType, const int codeLevel
                                        , const float temperature, const float translate, const bool warning);
public:
    Q_SIGNAL void sigHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);
private:
    QScopedPointer<ZhangjiakouHealthCodePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ZhangjiakouHealthCode)
    Q_DISABLE_COPY(ZhangjiakouHealthCode)
};

#endif // ZHANGJIAKOUHEALTHCODE_H
