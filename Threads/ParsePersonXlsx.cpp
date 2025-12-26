#include "ParsePersonXlsx.h"

#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellrange.h"
#include "xlsxworksheet.h"
#include "xlsxworkbook_p.h"
#include "xlsxcellformula.h"

#include "BaiduFace/BaiduFaceManager.h"

#include "DB/RegisteredFacesDB.h"
#include "Application/FaceApp.h"

#include <QDebug>
#include <QDateTime>
#include <QHash>
#include <QThread>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QDateTime>

static inline int queryRowCount(QSqlQuery &query)
{
    int initialPos = query.at();
    // Very strange but for no records .at() returns -2
    int pos = 0;
    if (query.last()) {
        pos = query.at() + 1;
    }
    else {
        pos = 0;
    }
    // Important to restore initial pos
    query.seek(initialPos);
    return pos;
}
class ParsePersonXlsxPrivate
{
    Q_DECLARE_PUBLIC(ParsePersonXlsx)
public:
    ParsePersonXlsxPrivate(ParsePersonXlsx *dd);
private:
    void InitXlsx(QXlsx::Worksheet *);
    bool DealPersonInfo(const QVector<QString> &info);
    bool AddFaceDataToUser(const QString &imagePath);
    void AddFaceDataFromExcel();

private:
    QMap<int, QString>mPersonXlsxHead;
    QMap<int, QVector<QString>>mPersonXlsxData;
private:
    ParsePersonXlsx *const q_ptr;
};

ParsePersonXlsxPrivate::ParsePersonXlsxPrivate(ParsePersonXlsx *dd)
    : q_ptr(dd)
{
}

ParsePersonXlsx::ParsePersonXlsx(QObject *parent)
    : QObject(parent)
    , d_ptr(new ParsePersonXlsxPrivate(this))
{
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

ParsePersonXlsx::~ParsePersonXlsx()
{

}

void ParsePersonXlsxPrivate::InitXlsx(QXlsx::Worksheet *workSheet)
{
    mPersonXlsxHead.insert(0, workSheet->read("A1").toString());
    mPersonXlsxHead.insert(1, workSheet->read("B1").toString());
    mPersonXlsxHead.insert(2, workSheet->read("C1").toString());
    mPersonXlsxHead.insert(3, workSheet->read("D1").toString());
    mPersonXlsxHead.insert(4, workSheet->read("E1").toString());
    mPersonXlsxHead.insert(5, workSheet->read("F1").toString());
    mPersonXlsxHead.insert(6, workSheet->read("G1").toString());
    mPersonXlsxHead.insert(7, workSheet->read("H1").toString());
    mPersonXlsxHead.insert(8, workSheet->read("I1").toString());
    mPersonXlsxHead.insert(9, workSheet->read("J1").toString());
    mPersonXlsxHead.insert(10, workSheet->read("K1").toString());
    mPersonXlsxHead.insert(11, workSheet->read("L1").toString());
    mPersonXlsxHead.insert(12, workSheet->read("M1").toString());
    mPersonXlsxHead.insert(13, workSheet->read("N1").toString());
    mPersonXlsxHead.insert(14, workSheet->read("O1").toString());
    mPersonXlsxHead.insert(15, workSheet->read("P1").toString());
    mPersonXlsxHead.insert(16, workSheet->read("Q1").toString());
    mPersonXlsxHead.insert(17, workSheet->read("R1").toString());
    mPersonXlsxHead.insert(18, workSheet->read("S1").toString());
    mPersonXlsxHead.insert(19, workSheet->read("T1").toString());
    mPersonXlsxHead.insert(20, workSheet->read("U1").toString());
    mPersonXlsxHead.insert(21, workSheet->read("V1").toString());
    mPersonXlsxHead.insert(22, workSheet->read("W1").toString());
    mPersonXlsxHead.insert(23, workSheet->read("X1").toString());
    mPersonXlsxHead.insert(24, workSheet->read("Y1").toString());
    mPersonXlsxHead.insert(25, workSheet->read("Z1").toString());
}

bool ParsePersonXlsxPrivate::DealPersonInfo(const QVector<QString> &info)
{
    if(info.count() != mPersonXlsxHead.count())return false;

    // Check duplicate ID card
    QString idCardNum = info.at(mPersonXlsxHead.key("身份证"));
    QSqlQuery checkQuery(QSqlDatabase::database("isc_arcsoft_face"));
    checkQuery.prepare("SELECT COUNT(*) FROM person WHERE idcardnum = :idcard");
    checkQuery.bindValue(":idcard", idCardNum);
    
    if (!checkQuery.exec()) {
        qDebug() << "Failed to query database for duplicate ID card:" << checkQuery.lastError().text();
        return false;
    }
    
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "Duplicate ID card found:" << idCardNum;
        return false;
    }

    // Check duplicate card number
    QString cardNo = info.at(mPersonXlsxHead.key("IC卡号"));
    if (!cardNo.isEmpty()) {
        checkQuery.prepare("SELECT COUNT(*) FROM person WHERE iccardnum = :cardno");
        checkQuery.bindValue(":cardno", cardNo);
        
        if (!checkQuery.exec() || (checkQuery.next() && checkQuery.value(0).toInt() > 0)) {
            qDebug() << "Duplicate card number found:" << cardNo;
            return false;
        }
    }

#ifdef Q_OS_WIN
    QString path = QString("C:\\Users\\63279\\Desktop\\人脸产品\\测试资源\\测试导入2万人脸\\") + info.at(mPersonXlsxHead.key("人脸图片"));
#else
    QString path = QString("/udisk/") + info.at(mPersonXlsxHead.key("人脸图片"));
#endif

    QImage img = QImage(QString::fromUtf8(path.toLatin1()));
    if(img.isNull()) {
        qDebug() << "img.isNull() " << path;
        return false;
    }

    int faceNum = 0;
    double threshold = 0;
    int ret = -1;
    QByteArray FaceFeature;

#ifdef Q_OS_LINUX
    ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(path, faceNum, threshold, FaceFeature);
#endif

    if((ret == 0) && (faceNum == 1) && (threshold >= 0.85)) {
        // Check for duplicate faces
        QSqlQuery faceQuery(QSqlDatabase::database("isc_arcsoft_face"));
        faceQuery.prepare("SELECT feature FROM person");
        
        if (faceQuery.exec()) {
            while (faceQuery.next()) {
                QByteArray dbFeature = faceQuery.value("feature").toByteArray();
                if (!dbFeature.isEmpty()) {
                    double similarity = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare_baidu(
                        (unsigned char*)FaceFeature.data(),
                        FaceFeature.size(),
                        (unsigned char*)dbFeature.data(),
                        dbFeature.size()
                    );
                    if(similarity > 0.8) {
                        qDebug() << "Duplicate face found";
                        return false;
                    }
                }
            }
        }

        // If no duplicates found, proceed with registration

        return RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(info.at(mPersonXlsxHead.key("姓名")), info.at(mPersonXlsxHead.key("性别")),
                                                               info.at(mPersonXlsxHead.key("民族")), info.at(mPersonXlsxHead.key("身份证")),
                                                               info.at(mPersonXlsxHead.key("婚否")), info.at(mPersonXlsxHead.key("学历")),
                                                               info.at(mPersonXlsxHead.key("地址")), info.at(mPersonXlsxHead.key("电话")),
                                                               info.at(mPersonXlsxHead.key("政治面貌")), info.at(mPersonXlsxHead.key("出生日期")),
                                                               info.at(mPersonXlsxHead.key("编号")),  info.at(mPersonXlsxHead.key("部门")),
                                                               info.at(mPersonXlsxHead.key("入职时间")), info.at(mPersonXlsxHead.key("分机号")),
                                                               info.at(mPersonXlsxHead.key("邮箱")), info.at(mPersonXlsxHead.key("状态")),
                                                               info.at(mPersonXlsxHead.key("员工类型")), info.at(mPersonXlsxHead.key("IC卡号")),
                                                               FaceFeature);
    }else qDebug()<<"(ret == 0) && (faceNum == 1) && (threshold >= 0.7) "<<ret <<" "<<faceNum<<"\n";

    return false;
}


void ParsePersonXlsx::ImportPersonToDB()
{
    Q_D(ParsePersonXlsx);
    int succeedCnt = 0;//记录成功的
    int failCnt = 0;//记录失败的
    int Index = 0;
    int total = d->mPersonXlsxData.count();
    auto it = d->mPersonXlsxData.constBegin();
    while(it != d->mPersonXlsxData.constEnd())
    {
        ++Index;
        if(d->DealPersonInfo(it.value()))
            ++succeedCnt;
        else ++failCnt;

        emit sigImportProgressShell(false, total, Index, succeedCnt, failCnt);
        ++it;
    }
    emit sigImportProgressShell(true, total, Index, succeedCnt, failCnt);
}

void ParsePersonXlsx::slotParseXlsxPath(const QString Path)
{
    Q_D(ParsePersonXlsx);
    d->mPersonXlsxData.clear();
    d->mPersonXlsxHead.clear();
    QXlsx::Document xlsx(Path);
    QXlsx::Workbook *workBook = xlsx.workbook();
    QXlsx::Worksheet *workSheet = static_cast<QXlsx::Worksheet*>(workBook->sheet(0));
    //读取表头信息
    d->InitXlsx(workSheet);

    int rowCount = workSheet->CellTabelCount();
    int columnCount = d->mPersonXlsxHead.size();
    for (int row = 2; row <= rowCount; row++)
    {
        QVector<QString>Data(columnCount, QString());
        for (int column = 0; column < columnCount; column++)
        {
            QXlsx::Cell *cell = workSheet->cellAt(row, column + 1);
            if (cell != NULL)
            {
                Data[column] = cell->value().toString();
            }
        }
        d->mPersonXlsxData[row] = Data;
    }

    this->ImportPersonToDB();
}

void ParsePersonXlsx::slotExportPersons()
{
    QXlsx::Document xlsx;
    xlsx.write(QString("%1%2").arg("A").arg(1), "姓名");
    xlsx.write(QString("%1%2").arg("B").arg(1), "性别");
    xlsx.write(QString("%1%2").arg("C").arg(1), "民族");
    xlsx.write(QString("%1%2").arg("D").arg(1), "身份证");
    xlsx.write(QString("%1%2").arg("E").arg(1), "婚否");
    xlsx.write(QString("%1%2").arg("F").arg(1), "学历");
    xlsx.write(QString("%1%2").arg("G").arg(1), "地址");
    xlsx.write(QString("%1%2").arg("H").arg(1), "电话");
    xlsx.write(QString("%1%2").arg("I").arg(1), "政治面貌");
    xlsx.write(QString("%1%2").arg("J").arg(1), "出生日期");
    xlsx.write(QString("%1%2").arg("K").arg(1), "编号");
    xlsx.write(QString("%1%2").arg("L").arg(1), "部门");
    xlsx.write(QString("%1%2").arg("M").arg(1), "入职时间");
    xlsx.write(QString("%1%2").arg("N").arg(1), "分机号");
    xlsx.write(QString("%1%2").arg("O").arg(1), "邮箱");
    xlsx.write(QString("%1%2").arg("P").arg(1), "状态");
    xlsx.write(QString("%1%2").arg("Q").arg(1), "员工类型");
    xlsx.write(QString("%1%2").arg("R").arg(1), "IC卡号");
    xlsx.write(QString("%1%2").arg("S").arg(1), "添加时间");
    xlsx.write(QString("%1%2").arg("T").arg(1), "UUID");

    int index = 2;
    int writeCnt = 0;
	int totalCnt=0;	
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("select * from person");
    query.exec();
	if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        totalCnt = query.size();
    }
    else
    {
        totalCnt = queryRowCount(query);
    }
    while (query.next())
    {//取出对应数据
        xlsx.write(QString("%1%2").arg("A").arg(index), query.value("name"));
        xlsx.write(QString("%1%2").arg("B").arg(index), query.value("sex"));
        //xlsx.write(QString("%1%2").arg("C").arg(index), query.value("nation"));
        xlsx.write(QString("%1%2").arg("D").arg(index), query.value("idcardnum"));
        //        xlsx.write(QString("%1%2").arg("E").arg(index), query.value("Marital"));
        //        xlsx.write(QString("%1%2").arg("F").arg(index), query.value("education"));
        //        xlsx.write(QString("%1%2").arg("G").arg(index), query.value("location"));
        //        xlsx.write(QString("%1%2").arg("H").arg(index), query.value("phone"));
        //        xlsx.write(QString("%1%2").arg("I").arg(index), query.value("politics_status"));
        //        xlsx.write(QString("%1%2").arg("J").arg(index), query.value("date_birth"));
        //        xlsx.write(QString("%1%2").arg("K").arg(index), query.value("number"));
        //        xlsx.write(QString("%1%2").arg("L").arg(index), query.value("branch"));
        //        xlsx.write(QString("%1%2").arg("M").arg(index), query.value("hiredate"));
        //        xlsx.write(QString("%1%2").arg("N").arg(index), query.value("extension_phone"));
        //        xlsx.write(QString("%1%2").arg("O").arg(index), query.value("mailbox"));
        //        xlsx.write(QString("%1%2").arg("P").arg(index), query.value("status"));
        xlsx.write(QString("%1%2").arg("Q").arg(index), query.value("persontype"));
        xlsx.write(QString("%1%2").arg("R").arg(index), query.value("iccardnum"));
        xlsx.write(QString("%1%2").arg("S").arg(index), query.value("createtime"));
        xlsx.write(QString("%1%2").arg("T").arg(index++), query.value("uuid"));

        ++writeCnt;
        //emit sigExportProgressShell(false, false, -1, writeCnt);
		emit sigExportProgressShell(true, false, totalCnt, writeCnt);		
    }

#ifdef Q_OS_WIN
    bool saveState = xlsx.saveAs(QString("./%1_export_person.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
#else
    bool saveState = xlsx.saveAs(QString("/udisk/%1_export_person.xlsx").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
    system("sync");
#endif
	if (totalCnt<20) 
	{
		
		QTime dieTime = QTime::currentTime().addMSecs(2 * 1000 * 1000);
		while (QTime::currentTime() < dieTime)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
		}	  
	
	}
    //emit sigExportProgressShell(true, saveState, totalCnt, writeCnt);
}

bool ParsePersonXlsxPrivate::AddFaceDataToUser(const QString &imagePath)
{
    Q_Q(ParsePersonXlsx);
    
    // Extract employee ID from image filename (assuming format like "SZR1001.jpg")
    QString empId = imagePath.split('.').first();
    
    // Find user by employee ID (assuming empId is stored in personid or another field)
    QSqlQuery userQuery(QSqlDatabase::database("isc_arcsoft_face"));
    userQuery.prepare("SELECT uuid, name, personid FROM person WHERE personid = ? OR uuid = ?");
    userQuery.bindValue(0, empId);
    userQuery.bindValue(1, empId);
    
    if (!userQuery.exec() || !userQuery.next()) {
        qDebug() << "No user found with ID:" << empId;
        return false;
    }

    QString userUuid = userQuery.value("uuid").toString();
    QString userName = userQuery.value("name").toString();

    // Construct full image path
    QString fullPath;
#ifdef Q_OS_WIN
    fullPath = QString("C:\\Users\\63279\\Desktop\\人脸产品\\测试资源\\测试导入2万人脸\\") + imagePath;
#else
    fullPath = QString("/udisk/") + imagePath;
#endif

    // Verify image exists
    QImage img = QImage(QString::fromUtf8(fullPath.toLatin1()));
    if(img.isNull()) {
        qDebug() << "Image not found or invalid:" << fullPath;
        return false;
    }

    // Extract face features
    int faceNum = 0;
    double threshold = 0;
    int ret = -1;
    QByteArray faceFeature;

#ifdef Q_OS_LINUX
    ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(fullPath, faceNum, threshold, faceFeature);
#endif

    if((ret != 0) || (faceNum != 1) || (threshold < 0.85)) {
        qDebug() << "Face extraction failed. ret:" << ret << "faceNum:" << faceNum << "threshold:" << threshold;
        return false;
    }

    // Check if user already has face data
    QSqlQuery checkQuery(QSqlDatabase::database("isc_arcsoft_face"));
    checkQuery.prepare("SELECT feature FROM person WHERE uuid = ? AND feature IS NOT NULL AND length(feature) > 0");
    checkQuery.bindValue(0, userUuid);
    
    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "User already has face data:" << userName;
        return false;
    }

    // Check for duplicate faces in existing registered users
    QSqlQuery faceQuery(QSqlDatabase::database("isc_arcsoft_face"));
    faceQuery.prepare("SELECT feature, uuid, name FROM person WHERE feature IS NOT NULL AND length(feature) > 0 AND uuid != ?");
    faceQuery.bindValue(0, userUuid);
    
    if (faceQuery.exec()) {
        while (faceQuery.next()) {
            QByteArray dbFeature = faceQuery.value("feature").toByteArray();
            if (!dbFeature.isEmpty()) {
                double similarity = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare_baidu(
                    (unsigned char*)faceFeature.data(),
                    faceFeature.size(),
                    (unsigned char*)dbFeature.data(),
                    dbFeature.size()
                );
                if(similarity > 0.8) {
                    qDebug() << "Face already exists for user:" << faceQuery.value("name").toString();
                    return false;
                }
            }
        }
    }

    // Update the user with face data
    QSqlQuery updateQuery(QSqlDatabase::database("isc_arcsoft_face"));
    updateQuery.prepare("UPDATE person SET feature = ?, featuresize = ? WHERE uuid = ?");
    updateQuery.bindValue(0, faceFeature);
    updateQuery.bindValue(1, faceFeature.size());
    updateQuery.bindValue(2, userUuid);

    if (!updateQuery.exec()) {
        qDebug() << "Failed to update user with face data:" << updateQuery.lastError().text();
        return false;
    }

    // Update in-memory data in RegisteredFacesDB
    if (!RegisteredFacesDB::GetInstance()->UpdatePersonFaceFeature(userUuid, faceFeature)) {
        qDebug() << "Failed to update in-memory face data for user:" << userName;
        return false;
    }

    qDebug() << "Successfully added face data to user:" << userName << "UUID:" << userUuid << "Image:" << imagePath;
    
    // Emit signal to update UI
    emit q->sigFaceDataUpdated(userUuid);
    
    return true;
}

void ParsePersonXlsxPrivate::AddFaceDataFromExcel()
{
    Q_Q(ParsePersonXlsx);
    
    int successCount = 0;
    int failCount = 0;
    int current = 0;
    int total = mPersonXlsxData.count();

    auto it = mPersonXlsxData.constBegin();
    while(it != mPersonXlsxData.constEnd()) {
        ++current;
        
        // Get the image path from the first (and only) column
        if (!it.value().isEmpty()) {
            QString imagePath = it.value().at(0);
            
            if (AddFaceDataToUser(imagePath)) {
                ++successCount;
            } else {
                ++failCount;
            }
        } else {
            ++failCount;
        }

        emit q->sigAddFaceProgressShell(false, total, current, successCount, failCount);
        ++it;
    }
    
    emit q->sigAddFaceProgressShell(true, total, current, successCount, failCount);
}

void ParsePersonXlsx::slotAddFaceDataFromXlsx(const QString Path)
{
    Q_D(ParsePersonXlsx);
    
    d->mPersonXlsxData.clear();
    d->mPersonXlsxHead.clear();
    
    QXlsx::Document xlsx(Path);
    QXlsx::Workbook *workBook = xlsx.workbook();
    QXlsx::Worksheet *workSheet = static_cast<QXlsx::Worksheet*>(workBook->sheet(0));
    
    // For single column excel, we only read column A
    d->mPersonXlsxHead.insert(0, "Image Path"); // Set a default header name
    
    int rowCount = workSheet->CellTabelCount();
    
    for (int row = 1; row <= rowCount; row++) { // Start from row 1 (assuming no header)
        QVector<QString> data(1, QString()); // Only one column
        QXlsx::Cell *cell = workSheet->cellAt(row, 1); // Column A
        if (cell != NULL) {
            data[0] = cell->value().toString();
        }
        
        if (!data[0].isEmpty()) { // Only add non-empty paths
            d->mPersonXlsxData[row] = data;
        }
    }

    d->AddFaceDataFromExcel();
}
