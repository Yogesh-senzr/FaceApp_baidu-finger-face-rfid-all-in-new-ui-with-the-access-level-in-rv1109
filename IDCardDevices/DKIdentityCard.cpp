#include "DKIdentityCard.h"
#include "PCIcore/RkUtils.h"
#include "PCIcore/Audio.h"

#include <QWaitCondition>
#include <QDebug>
#include <QMutex>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MessageHandler/Log.h"

extern "C"
{
#include "derkiot/uart.h"
#include "derkiot/DKReader.h"
#include "derkiot/utf.h"
#include "derkiot/tcp_client.h"
#include "derkiot/dk_utils.h"
#include "derkiot/id_data_decode.h"
}

#define UART_PORT  ("/dev/ttyS4")
extern char DK_IDCardNumber[20];
extern char DK_Name[15];
extern char DK_Sex[15];

class DKIdentityCardPrivate
{
    Q_DECLARE_PUBLIC(DKIdentityCard)
public:
    DKIdentityCardPrivate(DKIdentityCard *dd);
private:
    void DKCheckIdCard();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
private:
    bool mExit;
private:
    DKIdentityCard *const q_ptr;
};


DKIdentityCardPrivate::DKIdentityCardPrivate(DKIdentityCard *dd)
    : q_ptr(dd)
    , mExit(false)
{ 
}

DKIdentityCard::DKIdentityCard(QObject *parent)
    : QThread(parent)
    , d_ptr(new DKIdentityCardPrivate(this))
{
    if (open_uart_device(UART_PORT) == UART_INIT_FAIL)
    {
        LogV("%s %s[%d] open %s fail \n", __FILE__, __FUNCTION__, __LINE__, UART_PORT);
    }else this->start();
}

DKIdentityCard::~DKIdentityCard()
{
    Q_D(DKIdentityCard);
    Q_UNUSED(d);
    this->requestInterruption();
    d->mExit = true;
    d->pauseCond.wakeOne();

    this->quit();
    this->wait();
}

//在显示屏上可以增加身份证解析进度,提醒用户不要马上拿开身份证
static inline void schedule_printf(uint8_t rate)
{
    printf("%s %s[%d] analysis rate : %%%d \n", __FILE__, __FUNCTION__, __LINE__, rate);
}

static inline int Utils_getFileSize(const char *path)
{
    int size = 0;
    struct stat buf;
    if (stat(path, &buf) < 0)
    {
        return 0;
    }
    size = buf.st_size;
    return size;
}

void DKIdentityCardPrivate::DKCheckIdCard()
{
    int ret;
    uint8_t out_data[4096];
    uint16_t out_len;
    uint8_t id_info[256], id_bitmap[4096];
    uint16_t wordMsgBytesLen;
    uint16_t imageMsgBytesLen;
    uint16_t useful_message_index;
    //阻塞等待，没有读到身份证超时为3秒
    ret = get_idcard_data_with_block(schedule_printf, out_data, &out_len);
    switch (ret)
    {
    case ANALYSIS_OK:
    {
        //printf("%s %s[%d] ANALYSIS_OK \n", __FILE__, __FUNCTION__, __LINE__);
        useful_message_index = DN_ANALYSIS_EXTRA_LEN;
        wordMsgBytesLen = ((out_data[useful_message_index] & 0xff) << 8) | (out_data[useful_message_index + 1] & 0xff);
        imageMsgBytesLen = ((out_data[useful_message_index + 2] & 0xff) << 8) | (out_data[useful_message_index + 3] & 0xff);
        //       LogV("%s %s[%d] wordMsgBytesLen:%d,imageMsgBytesLen:%d \n", __FILE__, __FUNCTION__, __LINE__, wordMsgBytesLen,
        //               imageMsgBytesLen);
        if (wordMsgBytesLen > 220)
        {
            memcpy(id_info, out_data + useful_message_index + 4, wordMsgBytesLen); //拷贝文字信息
        }
        if (imageMsgBytesLen > 512)
        {
            memcpy(id_bitmap, out_data + useful_message_index + 4 + wordMsgBytesLen, imageMsgBytesLen); //拷贝照片信息
        }
        //LogV("%s %s[%d] id_data_decode start \n", __FILE__, __FUNCTION__, __LINE__);
        id_data_decode(id_info, wordMsgBytesLen, id_bitmap, imageMsgBytesLen);
        //printf("%s %s[%d] id_data_decode end \n", __FILE__, __FUNCTION__, __LINE__);
        //到这一步才算解析完成，才能拿开身份证，此时打开蜂鸣器提示
        //YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "Buzzer.wav", true);//不放在这里嘀

        char *szImageName = "/mnt/user/facedb/idcard.jpeg";
        unlink(szImageName);

        int nBmpFileSize = Utils_getFileSize("/mnt/user/photo.bmp");
        int nRgbSize = 126 * 102 * 3;
        int nBmpFd = open("/mnt/user/photo.bmp", O_RDWR, 0666);
        if (szImageName > 0 && nBmpFileSize > nRgbSize)
        {
            unsigned char *rgbData = (unsigned char*) malloc(nRgbSize);
            unsigned char *imageData = (unsigned char*) malloc(nRgbSize);
            if (rgbData == 0 || imageData == 0)
            {
                close(nBmpFd);
                break;
            }

            lseek(nBmpFd, nBmpFileSize - nRgbSize , SEEK_SET);
            read(nBmpFd, imageData, nBmpFileSize);
            //将身份证数据转换为纯RGB
            for (int i = 0; i < 126; i++)
            {
                for (int j = 0; j < 102 * 3; j++)
                {
                    rgbData[(125 - i) * 102 * 3 + j] = imageData[i * 102 * 3 + j + 54 + 2 * i];
                }
            }
            YNH_LJX::RkUtils::RGBtoJPEG(szImageName, rgbData, 102, 126);
            printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            free(rgbData);
            free(imageData);
            close(nBmpFd);
            printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            emit q_func()->sigIdCardInfo(QString::fromLocal8Bit(DK_Name), QString::fromLocal8Bit(DK_IDCardNumber), QString::fromLocal8Bit(DK_Sex), QString::fromLocal8Bit(szImageName));
        }
    }
        break;
    case ANALYSIS_NO_CARD:
        //printf("没有检测到身份证\r\n");
        break;
    case ANALYSIS_FAIL:
        YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
    default:
        //LogV("analysis fail\r\n");
        break;
    }
}

void DKIdentityCard::run()
{
    Q_D(DKIdentityCard);
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        if(!d->mExit) d->DKCheckIdCard();
        d->pauseCond.wait(&d->sync, 200);
        d->sync.unlock();
    }
    close_uart_device();
}

