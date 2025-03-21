#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "myhelper.h"
#include "MessageHandler/Log.h"

#ifdef Q_OS_LINUX
#include "PCIcore/Watchdog.h"
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QFileDialog>
#include <QMap>
#include <random>
#include <iostream>
#include <QSettings>
#include <QTextCodec>
#include <QTime>
#include <QProcess>
#include <QDateTime>
#include <fstream>
#include <ios>


//定义写入的注册表路径
#define SELFSTART_REGEDIT_PATH "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"
//定义路径最大程度
#define MAX_PATH_NUM 4096

void myHelper::AutoRunWithSystem(bool bAutoRun)
{
#ifndef _MSC_VER
    QSettings  reg(SELFSTART_REGEDIT_PATH, QSettings::NativeFormat);
    if (bAutoRun)
    {
        QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        strAppPath.insert(0, "\"");
        strAppPath.append("\" -autorun");
        reg.setValue("DeliApp", strAppPath);
    }
    else reg.remove("DeliApp");
#else
    QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    CEnablePriv::AppAutoRun(bAutoRun, strAppPath, QCoreApplication::applicationDirPath() + "\\Trayon.ico");
#endif
}

void myHelper::SetUTF8Code() 
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#endif
}

void myHelper::SetChinese() 
{
#if 0
    QTranslator *translator = new QTranslator(qApp);
    translator->load(":/qt_zh_CN.qm");
    qApp->installTranslator(translator);
#endif
}

QString myHelper::GetCorrectUnicode(const QByteArray & ba)
{
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(ba.constData(), ba.size(), &state);
    if (state.invalidChars > 0)
    {
        text = QTextCodec::codecForName("GBK")->toUnicode(ba);
    }
    else
    {
        text = ba;
    }

    return text;
}

void myHelper::SetStyle(const QString &qssFile) 
{
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        qApp->setStyleSheet(qss);
        QString PaletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(PaletteColor)));
        file.close();
    }
}

bool myHelper::IsIP(QString IP) {
    QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    return RegExp.exactMatch(IP);
}

bool myHelper::confirmFile(const QString sFile)
{
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sFilePath = sAppPath + "/" + sFile;
    //文件是否存在
    if (QFile::exists(sFilePath))
        return true;//文件存在，则确认可以操作
    //文件不存在，先看路径是否存在
    QDir dir(sFilePath);
    if (!dir.exists(sFilePath))
    {
        //路径不存在，则创建路径
        if (!dir.mkdir(sFilePath))
            return false;
        //路径创建失败就没办法了，通常不会失败
    }
    return true;
}

QString myHelper::AppPath()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
}

bool myHelper::RmDir(const QString & dirname)
{
    QDir dir(dirname);
    return dir.rmdir(dirname);
}
bool myHelper::RmPath(const QString & pathname)
{
    QDir dir(pathname);
    return dir.rmpath(pathname);
}
bool myHelper::RmFile(const QString & filename)
{
    QDir dir(filename);
    return dir.remove(filename);
}

QString myHelper::getCpuSerial()
{
    std::string line;
    std::ifstream infile("/proc/cpuinfo", std::ios::in);
    if (infile)
    {
        while (!infile.eof())
        {
            getline(infile, line);
            //			printf("%s %s[%d] %s \n", __FILE__, __FUNCTION__, __LINE__, line.c_str());
            if (line.find("Serial") != std::string::npos)
            {
                break;
            }
        }
        if (line.length() > 1)
        {
            line = line.substr(line.find("Serial		: ") + strlen("Serial		: "), line.length() - strlen("Serial		: "));
        }
        infile.close();
    }

    return QString::fromStdString(line);
}

QString myHelper::Utils_ExecCmd(const QString cmd)
{
	LogD("%s %s[%d] %s \n",__FILE__,__FUNCTION__,__LINE__,cmd.toStdString().c_str());
	QString str = "";
    if (cmd.size() > 1)
    {
        FILE *pFile = popen(cmd.toStdString().c_str(), "r");
        if (pFile)
        {
        	std::string ret = "";
    		char buf[256] = { 0 };
    		int readSize = 0;
    		do
    		{
    			readSize = fread(buf, 1, sizeof(buf), pFile);
    			if (readSize > 0)
    			{
    				ret += std::string(buf, 0, readSize);
    			}
    		} while (readSize > 0);
            pclose(pFile);

            str = QString::fromStdString(ret);
        }
    }
    return str;
}

void myHelper::Utils_Reboot()
{
#ifdef Q_OS_LINUX
    Utils_ExecCmd("sync; touch /isc/reboot_flag");
    Utils_ExecCmd("sync");
    Utils_ExecCmd("sync");
    YNH_LJX::Watchdog::WatchDog_OpenWatchDog();
    YNH_LJX::Watchdog::WatchDog_FeedWatchDog(1);
    system("reboot");
#endif
}

void myHelper::Utils_Poweroff()
{
#ifdef Q_OS_LINUX
    system("sync");
    YNH_LJX::Watchdog::WatchDog_CloseWatchDog();
    system("poweroff");
#endif
}

void myHelper::System_Update(const char *strPath)
{
    char szCmd[128] = { 0 };
    LogD("%s %s[%d] %s \n",__FILE__,__FUNCTION__,__LINE__,strPath);
    snprintf(szCmd, sizeof(szCmd), "updateEngine --image_url=%s --misc=update --savepath=%s --reboot", strPath, strPath);
    //printf("%s %s[%d] %s \n", __FILE__, __FUNCTION__, __LINE__, szCmd);
    Utils_ExecCmd(szCmd);
    Utils_ExecCmd("sync");
//    Utils_Reboot();
}

QString myHelper::absolutePath(const QString &path)
{
    QDir temDir(path);
    QString absDir = temDir.absolutePath();
    absDir.replace(QString("/"), QString("\\\\"));
    return absDir;
}


QByteArray myHelper::HexStrToByteArray(QString str) {
    QByteArray senddata;
    int hexdata, lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len / 2);
    char lstr, hstr;
    for (int i = 0; i < len; ) {
        hstr = str[i].toLatin1();
        if (hstr == ' ') {
            i++;
            continue;
        }
        i++;
        if (i >= len) {
            break;
        }
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if ((hexdata == 16) || (lowhexdata == 16)) {
            break;
        }
        else {
            hexdata = hexdata * 16 + lowhexdata;
        }
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
    return senddata;
}

char myHelper::ConvertHexChar(char ch) {
    if ((ch >= '0') && (ch <= '9')) {
        return ch - 0x30;
    }
    else if ((ch >= 'A') && (ch <= 'F')) {
        return ch - 'A' + 10;
    }
    else if ((ch >= 'a') && (ch <= 'f')) {
        return ch - 'a' + 10;
    }
    else {
        return (-1);
    }
}

QString myHelper::ByteArrayToHexStr(QByteArray data) {
    QString temp = "";
    QString hex = data.toHex();
    for (int i = 0; i < hex.length(); i = i + 2) {
        temp += hex.mid(i, 2) + " ";
    }
    return temp.trimmed().toUpper();
}

int myHelper::StrHexToDecimal(QString strHex) {
    bool ok;
    return strHex.toInt(&ok, 16);
}

int myHelper::StrDecimalToDecimal(QString strDecimal) {
    bool ok;
    return strDecimal.toInt(&ok, 10);
}

int myHelper::StrBinToDecimal(QString strBin) {
    bool ok;
    return strBin.toInt(&ok, 2);
}

QString myHelper::StrHexToStrBin(QString strHex) {
    uchar decimal = StrHexToDecimal(strHex);
    QString bin = QString::number(decimal, 2);
    uchar len = bin.length();
    if (len < 8) {
        for (int i = 0; i < 8 - len; i++) {
            bin = "0" + bin;
        }
    }
    return bin;
}

QString myHelper::DecimalToStrBin1(int decimal) {
    QString bin = QString::number(decimal, 2);
    uchar len = bin.length();
    if (len <= 8) {
        for (int i = 0; i < 8 - len; i++) {
            bin = "0" + bin;
        }
    }
    return bin;
}

QString myHelper::DecimalToStrBin2(int decimal) {
    QString bin = QString::number(decimal, 2);
    uchar len = bin.length();
    if (len <= 16) {
        for (int i = 0; i < 16 - len; i++) {
            bin = "0" + bin;
        }
    }
    return bin;
}

QString myHelper::DecimalToStrHex(int decimal) {
    QString temp = QString::number(decimal, 16);
    if (temp.length() == 1) {
        temp = "0" + temp;
    }
    return temp;
}

QString myHelper::HexAndSpacing(const QString & hex)
{
    QString str = "";
    for (int i = 0; i < hex.length(); i = i + 2) {
        str += hex.mid(i, 2) + " ";
    }
    return str.trimmed().toUpper();
}

void myHelper::Sleep(int sec) 
{

    QTime dieTime = QTime::currentTime().addMSecs(sec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents();
    }
}

void myHelper::SetSystemDateTime(int year, int month, int day, int hour, int min, int sec) {
    QProcess p(0);

    p.start("cmd");
    p.waitForStarted();
    p.write(QString("date %1-%2-%3\n").arg(year).arg(month).arg(day).toLatin1());
    p.closeWriteChannel();
    p.waitForFinished(1000);
    p.close();

    p.start("cmd");
    p.waitForStarted();
    p.write(QString("time %1:%2:%3.00\n").arg(hour).arg(min).arg(sec).toLatin1());
    p.closeWriteChannel();
    p.waitForFinished(1000);
    p.close();
}

void myHelper::FormInCenter(QWidget *frm, int deskWidth, int deskHeigth) {
    int frmX = frm->width();
    int frmY = frm->height();
    QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeigth / 2 - frmY / 2);
    frm->move(movePoint);
}

QString myHelper::GetFileName(QString filter) {
    return QFileDialog::getOpenFileName(0, QObject::tr("SelectFile"), QCoreApplication::applicationDirPath(), filter);//"选择文件"
}

QStringList myHelper::GetFileNames(QString filter) {
    return QFileDialog::getOpenFileNames(0, QObject::tr("SelectFile"), QCoreApplication::applicationDirPath(), filter);//"选择文件"
}

QString myHelper::GetFolderName() {
    return QFileDialog::getExistingDirectory();;
}

QString myHelper::GetSaveDirPath(const QString fileName, const QString &setFilter)
{
    //QString setFilter = "Worddocx(*.docx);;";//Worddocx(*.docx);;office(*.xls *.xlsx *.ppt *.pptx)

    QString tStr = QFileDialog::getSaveFileName(0, QObject::tr("SelectSaveFileDirection"), fileName, setFilter);//QString("选择保存的文件路径")

    return tStr.replace(QString("/"), QString("\\\\"));

}

QString myHelper::GetFileNameWithExtension(QString strFilePath) {
    QFileInfo fileInfo(strFilePath);
    return fileInfo.fileName();
}

QStringList myHelper::GetFolderFileNames(QStringList filter) {
    QStringList fileList;
    QString strFolder = QFileDialog::getExistingDirectory();
    if (!strFolder.length() == 0) {
        QDir myFolder(strFolder);
        if (myFolder.exists()) {
            fileList = myFolder.entryList(filter);
        }
    }
    return fileList;
}

bool myHelper::FolderIsExist(QString strFolder) {
    QDir tempFolder(strFolder);
    return tempFolder.exists();
}

bool myHelper::FileIsExist(QString strFile) {
    QFile tempFile(strFile);
    return tempFile.exists();
}

QString myHelper::ReadFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QString data =file.readAll();
        file.close();

        return data;
    }
    return QString();
}
QString myHelper::ReadFileToBase64(const QString & filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QString data = file.readAll().toBase64();
        file.close();

        return data;
    }
    return QString();
}
QString myHelper::ReadFileToCompress(const QString & filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = qCompress(file.readAll());
        file.close();
        return data.toBase64();
    }
    return QString();
}

bool myHelper::WriteFile(const QString &filePath, const QString &Data)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly))
    {
        file.write(Data.toLatin1());
        file.close();
        return true;
    }

    return false;
}

bool myHelper::WriteFile(const QString & filePath, const QByteArray & Data)
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly))
    {
        file.write(Data);
        file.close();
        return true;
    }
    return false;
}
bool myHelper::copyFile(QString sourceFile, QString toDir)
{
    toDir.replace("\\", "/");
    if (sourceFile == toDir)
    {
        return true;
    }
    if (!QFile::exists(sourceFile))
    {
        return false;
    }
    QDir *createfile = new QDir;
    bool exist = createfile->exists(toDir);
    if (exist) {
        createfile->remove(toDir);
    }//end if

    delete createfile;

    if (!QFile::copy(sourceFile, toDir))
    {
        return false;
    }
    return true;
}

QString myHelper::getXorEncryptDecrypt(QString str, char key) {
    QByteArray data = str.toLatin1();
    int size = data.size();
    for (int i = 0; i < size; i++) {
        data[i] = data[i] ^ key;
    }
    return QLatin1String(data);
}

int myHelper::getSrand()
{
    int seed = QDateTime::currentDateTime().time().second();
    srand(seed);
    return (rand() % 20) + 60;
}

QString myHelper::vFor(double v, bool nZero, quint8 pri)
{
    QString str;
    //用g的好处是会隐藏无效后面的0，
    if (nZero)
        str.setNum(v, 'g', pri);
    else
        str.sprintf(QString("%#.%1g").arg(pri).toLatin1(), v);

    return str;
}

QString myHelper::setStyleSheet(const QString qssFile)
{
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        file.close();
        return qss;
    }
    return QString();
}

QString myHelper::GetNetworkMac()
{
	static QString mac = "";
	if(mac.size() <= 0)
	{
		mac = myHelper::ReadFile("/param/mac.txt");
	}
	return mac;
}

QString myHelper::GetBaiduDeviceID()
{
	QString str = "";
	if(str.size() <= 0)
	{
		str = myHelper::ReadFile("/param/baidu_device_id.txt");
	}
	return str;
}

QString myHelper::GetBaiduLicenseKey()
{
	QString str = "";
	if(str.size() <= 0)
	{
		str = myHelper::ReadFile("/param/license.key");
	}
	return str;
}

QString myHelper::GetBaiduLicenseIni()
{
	QString str = "";
	if(str.size() <= 0)
	{
		str = myHelper::ReadFile("/param/license.ini");
	}
	return str;
}
