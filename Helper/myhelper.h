#ifndef MYHELPER_H
#define MYHELPER_H


#include <stdint.h>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QRect>

class QWidget;
class QSize;
class myHelper
{
public:
    //设置为开机启动
    static void AutoRunWithSystem(bool IsAutoRun);
    //设置编码为UTF8
    static void SetUTF8Code();
    //设置为中文字符
    static void SetChinese();
    //GBK与UTF-8编码判断 并转成可显示的格式
    static QString GetCorrectUnicode(const QByteArray &ba);
    //设置指定样式
    static void SetStyle(const QString &qssFile);
    //读取文件
    static QString ReadFile(const QString &file);
    //读取文件转成base64
    static QString ReadFileToBase64(const QString &file);
    //读取数据压缩
    static QString ReadFileToCompress(const QString &file);
    //写入文件
    static bool WriteFile(const QString &file, const QString &Data);
    static bool WriteFile(const QString &file, const QByteArray &Data);
    //判断是否是IP地址
    static bool IsIP(QString IP);
    //16进制字符串转字节数组
    static QByteArray HexStrToByteArray(QString str);
    static char ConvertHexChar(char ch);
    //字节数组转16进制字符串
    static QString ByteArrayToHexStr(QByteArray data);
    //16进制字符串转10进制
    static int StrHexToDecimal(QString strHex);
    //10进制字符串转10进制
    static int StrDecimalToDecimal(QString strDecimal);
    //2进制字符串转10进制
    static int StrBinToDecimal(QString strBin);
    //16进制字符串转2进制字符串
    static QString StrHexToStrBin(QString strHex);
    //10进制转2进制字符串一个字节
    static QString DecimalToStrBin1(int decimal);
    //10进制转2进制字符串两个字节
    static QString DecimalToStrBin2(int decimal);
    //10进制转16进制字符串,补零.
    static QString DecimalToStrHex(int decimal);
    //16进制加空格
    static QString HexAndSpacing(const QString &hex);
    //延时
    static void Sleep(int sec);
    //设置系统日期时间
    static void SetSystemDateTime(int year, int month, int day, int hour, int min, int sec);
    //窗体居中显示
    static void FormInCenter(QWidget *frm, int deskWidth, int deskHeigth);
    //获取选择的文件
    static QString GetFileName(QString filter);
    //获取选择的文件集合
    static QStringList GetFileNames(QString filter);
    //获取选择的目录
    static QString GetFolderName();
    //保存路径
    static QString GetSaveDirPath(const QString fileName, const QString &setFilter = QString("Word文档(*.docx);;Word 97-2003文档(*.doc);;Excel工作薄(*.xlsx);;Pdf(*.pdf);;"));
    //获取文件名,含拓展名
    static QString GetFileNameWithExtension(QString strFilePath);
    //获取选择文件夹中的文件
    static QStringList GetFolderFileNames(QStringList filter);
    //文件夹是否存在
    static bool FolderIsExist(QString strFolder);
    //文件是否存在
    static bool FileIsExist(QString strFile);
    //复制文件
    static bool copyFile(QString sourceFile, QString targetFile);
    //相对路径转成绝对路径
    static QString absolutePath(const QString &path);
    //异或加密算法
    static QString getXorEncryptDecrypt(QString str, char key);
    //检查IP是否在线
    static bool IPCEnable(const QString rtspAddr, const int rtspPort);
    static int getSrand();
    static QString vFor(double v, bool nZero = false, quint8 pri = 6);
    //设置字体样式
    static QString setStyleSheet(const QString);
    //检测文件夹是否存在, 当前软件目录
    static bool confirmFile(const QString sFile);
    //获取当前路径
    static QString AppPath();
    //删除目录
    static bool RmDir(const QString &dirname);
    //删除路径
    static bool RmPath(const QString &pathname);
    //删除文件
    static bool RmFile(const QString &filename);
    //获取arm版的cpu串口号
    static QString getCpuSerial();

    //获取MAC地址,返回/param/mac.txt里面的内容
    static QString GetNetworkMac();

    //返回/param/baidu_device_id.txt里面的内容
    static QString GetBaiduDeviceID();

    //返回/param/license.key里面的内容
    static QString GetBaiduLicenseKey();

    //返回/param/license.ini里面的内容
    static QString GetBaiduLicenseIni();

    //
    static QString Utils_ExecCmd(const QString cmd);
    //重启设备
    static void Utils_Reboot();
    //
    static void Utils_Poweroff();
    //固件更新
    static void System_Update(const char *strPath);
};
#endif // MYHELPER_H
