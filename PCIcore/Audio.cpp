#include "Audio.h"
#include "SharedInclude/GlobalDef.h"
#include "Config/ReadConfig.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <string.h>
#include <pthread.h>
#include <string>

#define USE_APLAY (1)

#define MAX_FRAME_SIZE (2*1024)
#define AUDIO_DEV_TYPE_OUTPUT (0)
#define AUDIO_DEV_TYPE_INPUT (1)

#define RECOGNITION_FAILED              "/isc/res/en/recognition_failed.wav"
#define PEOPLE_STRANGER                 "/isc/res/en/stranger.wav"
#define TEMP_NORMAL_WELCOME             "/isc/res/en/temp_normal_welcom.wav"
#define TEMP_WARNING                    "/isc/res/en/temp_warning.wav"
#define TEMP_ABNORMAL                   "/isc/res/en/temp_abnormal.wav"
#define UN_BTMASK                       "/isc/res/en/un_btmask.wav"
#define RECORNIZED                      "/isc/res/en/recognized.wav"
#define STRANGER_TEMP_NORMAL            "/isc/res/en/stranger_normal_temp.wav"
#define STRANGER_TEMP_ABNORMAL          "/isc/res/en/stranger_abnormal_temp.wav"
#define CUSTOMER_PCM_NAME               "/isc/res/%s/%s"


static pthread_t stPlayPcmFileAsyncThread = 0;
static bool bIsPlayPcmFileAsyncFinish = true;
static AUDIO_DEV_S mstAsyncAudioDev;
static char mszAsyncPcmFile[128] = { 0 };
int g_nSpkVol = 10;
static pthread_mutex_t mAudioDevMutex;
static pthread_mutex_t mMicDevMutex;
static pthread_mutex_t mAudioDevMutexAsync;

void YNH_LJX::Audio::setVolume(const int vol)
{
    g_nSpkVol = vol;
}

void YNH_LJX::Audio::Audio_InitDev()
{
    pthread_mutex_init(&mAudioDevMutex, ISC_NULL);
    pthread_mutex_init(&mMicDevMutex, ISC_NULL);
    pthread_mutex_init(&mAudioDevMutexAsync, ISC_NULL);
}

void YNH_LJX::Audio::Audio_ExitDev()
{
    printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
    pthread_mutex_destroy(&mAudioDevMutex);
    pthread_mutex_destroy(&mMicDevMutex);
    pthread_mutex_destroy(&mAudioDevMutexAsync);
}

static inline void Utils_ExecCmd(const char* szCmd)
{
    char buf[64] = { 0 };
    if (szCmd != NULL)
    {
        FILE *pFile = popen(szCmd, "r");
        if (pFile)
        {
            while (fgets(buf, sizeof(buf), pFile) != NULL)
            {
            }
            pclose(pFile);
        }
    }
}

static inline AUDIO_DEV_S* Audio_OpenAudioDev(BIT_WIDTH_E enBitwidth, SAMPLE_RATE_E enSampleRate, SOUND_MODE_E enSoundMode)
{
    (void)enBitwidth;
    (void)enSampleRate;
    (void)enSoundMode;

    AUDIO_DEV_S *pAudioDev = ISC_NULL;
    pthread_mutex_lock(&mAudioDevMutex);
#if USE_APLAY
    pAudioDev = (AUDIO_DEV_S*) malloc(sizeof(AUDIO_DEV_S));
    pAudioDev->nAudioDevType = AUDIO_DEV_TYPE_OUTPUT;
#endif

    pthread_mutex_unlock(&mAudioDevMutex);
    return pAudioDev;
}

static inline AUDIO_DEV_S* Audio_OpenMicDev(BIT_WIDTH_E enBitwidth, SAMPLE_RATE_E enSampleRate, SOUND_MODE_E enSoundMode)
{
    (void)enBitwidth;
    (void)enSampleRate;
    (void)enSoundMode;
    AUDIO_DEV_S *pAudioDev = ISC_NULL;
    //printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
    pthread_mutex_lock(&mMicDevMutex);

    pthread_mutex_unlock(&mMicDevMutex);
    return pAudioDev;
}

static inline void Audio_PlayPCM(AUDIO_DEV_S *pAudioDev, unsigned char *pPCMBuf, unsigned int nPCMSize)
{
    pthread_mutex_lock(&mAudioDevMutex);

    if (pAudioDev == ISC_NULL || pPCMBuf == ISC_NULL || nPCMSize > MAX_FRAME_SIZE || pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_INPUT)
    {
        //		printf("%s %s[%d]  pPCMBuf %x nPCMSize %d \n", __FILE__, __FUNCTION__, __LINE__, pPCMBuf, nPCMSize);
        pthread_mutex_unlock(&mAudioDevMutex);
        return;
    }
    pthread_mutex_unlock(&mAudioDevMutex);
}

static inline int Audio_ReadMicPCM(AUDIO_DEV_S *pAudioDev, unsigned char **ppPCMBuf, unsigned int *nPCMSize)
{
    (void)pAudioDev;
    (void)ppPCMBuf;
    (void)nPCMSize;

    int nRet = 0;
    pthread_mutex_lock(&mMicDevMutex);
    if (pAudioDev == ISC_NULL)
    {
        pthread_mutex_unlock(&mMicDevMutex);
        return nRet;
    }
    if (pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_INPUT)
    {
    }
    pthread_mutex_unlock(&mMicDevMutex);
    return nRet;
}

static inline void Audio_CloseAudioDev(AUDIO_DEV_S *pAudioDev)
{
    //	printf("%s %s[%d] pAudioDev %p \n", __FILE__, __FUNCTION__, __LINE__, pAudioDev);
    if (pAudioDev != ISC_NULL)
    {
        pthread_mutex_lock(&mAudioDevMutex);
        if (pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_INPUT)
        {
            pthread_mutex_lock(&mMicDevMutex);

            //打开SPK MUTE
            //			GPIO_SetGPIOValue(3, 11, 0);

            pthread_mutex_unlock(&mMicDevMutex);
        } else
        {
        }

        pthread_mutex_unlock(&mAudioDevMutex);
    }

    //	printf("%s %s[%d] pAudioDev %p \n", __FILE__, __FUNCTION__, __LINE__, pAudioDev);
}

static inline int Audio_SetAudioVolume(AUDIO_DEV_S *pAudioDev, int nVolumeDb)
{
    //printf("%s %s[%d] nVolumeDb %d \n",__FILE__,__FUNCTION__,__LINE__,nVolumeDb);
    if (pAudioDev != ISC_NULL)
    {
        if (pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_OUTPUT)
        {
            pthread_mutex_lock(&mAudioDevMutex);

            nVolumeDb = (((float) nVolumeDb / 100) * 200);
            if(nVolumeDb > 200)
            {
                nVolumeDb = 200;
            }
            std::string strVol = std::string("amixer set 'Master' ")+std::to_string(nVolumeDb);
            //printf("%s %s[%d] %s \n",__FILE__,__FUNCTION__,__LINE__,strVol.c_str());
            Utils_ExecCmd(strVol.c_str());

            pthread_mutex_unlock(&mAudioDevMutex);
        } else if (pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_INPUT)
        {
            pthread_mutex_lock(&mMicDevMutex);
        }
    }
    return ISC_ERROR;
}

static inline int Audio_GetAudioVolume(AUDIO_DEV_S *pAudioDev)
{
    if (pAudioDev != ISC_NULL)
    {
        pthread_mutex_lock(&mAudioDevMutex);
        pthread_mutex_unlock(&mAudioDevMutex);
    }
    return ISC_ERROR;
}

static inline void Audio_PlayPCMFile(AUDIO_DEV_S *pAudioDev, const char *szFile)
{
    if (pAudioDev->nAudioDevType == AUDIO_DEV_TYPE_INPUT)
    {
        return;
    }
//    printf("%s %s[%d] pAudioDev %p szFile %s \n", __FILE__, __FUNCTION__, __LINE__, pAudioDev, szFile);
    if (pAudioDev != ISC_NULL && szFile != ISC_NULL)
    {
#if USE_APLAY
        Audio_SetAudioVolume(pAudioDev, g_nSpkVol);
        char cmd[512] = {0};
        //snprintf(cmd, sizeof(cmd), "aplay %s --channels=1 --rate=48000 --format=S16_LE", szFile);
	snprintf(cmd, sizeof(cmd), "aplay %s ", szFile);
        Utils_ExecCmd(cmd);
#else
        int fd = open(szFile, O_RDONLY);
        //		printf("%s %s[%d] open %s fd %d \n", __FILE__, __FUNCTION__, __LINE__, szFile, fd);
        if (fd > 0)
        {
            unsigned char buf[1024] =
            {	0};
            while (true)
            {
                int ret = read(fd, buf, sizeof(buf));
                if (ret <= 0)
                {
                    break;
                }
                Audio_PlayPCM(pAudioDev, buf, ret);
            }
            close(fd);
            sleep(1);
        }
#endif
    }
}

static void* PlayPcmFileAsyncThread(void *arg)
{
    (void)arg;
    bIsPlayPcmFileAsyncFinish = false;
    pthread_detach(pthread_self());

    AUDIO_DEV_S *pAudioDev = Audio_OpenAudioDev(mstAsyncAudioDev.enBitwidth, mstAsyncAudioDev.enSampleRate, mstAsyncAudioDev.enSoundMode);
    if (pAudioDev != ISC_NULL)
    {
        Audio_PlayPCMFile(pAudioDev, mszAsyncPcmFile);
        Audio_CloseAudioDev(pAudioDev);
    }
    //	printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
    bIsPlayPcmFileAsyncFinish = true;
    pthread_exit(0);
    return ISC_NULL;
}

void Audio_PlayPCMFileAsync(BIT_WIDTH_E enBitwidth, SAMPLE_RATE_E enSampleRate, SOUND_MODE_E enSoundMode, const char *szFile, bool waited)
{

    while (waited)
    {
        pthread_mutex_lock(&mAudioDevMutexAsync);
        if (bIsPlayPcmFileAsyncFinish == false)
        {
            pthread_mutex_unlock(&mAudioDevMutexAsync);
            usleep(200 * 1000);
        } else
        {
            pthread_mutex_unlock(&mAudioDevMutexAsync);
            break;
        }
    }
    pthread_mutex_lock(&mAudioDevMutexAsync);
    //	printf("%s %s[%d] bIsPlayPcmFileAsyncFinish %d enBitwidth %x  enSampleRate %x  enSoundMode %x szFile %s \n",
    //	__FILE__, __FUNCTION__, __LINE__, bIsPlayPcmFileAsyncFinish, enBitwidth, enSampleRate, enSoundMode, szFile);
    if (bIsPlayPcmFileAsyncFinish == false || szFile == ISC_NULL)
    {
        pthread_mutex_unlock(&mAudioDevMutexAsync);
        return;
    }

    mstAsyncAudioDev.enBitwidth = enBitwidth;
    mstAsyncAudioDev.enSampleRate = enSampleRate;
    mstAsyncAudioDev.enSoundMode = enSoundMode;
    memset(mszAsyncPcmFile, 0, sizeof(mszAsyncPcmFile));
    strncpy(mszAsyncPcmFile, szFile, sizeof(mszAsyncPcmFile));
    if (pthread_create(&stPlayPcmFileAsyncThread, ISC_NULL, PlayPcmFileAsyncThread, ISC_NULL))
    {
        //		printf("%s %s[%d] pthread_create error  %s \n", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
    }

    pthread_mutex_unlock(&mAudioDevMutexAsync);
}

void YNH_LJX::Audio::Audio_PlayRecognitionFailedPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, RECOGNITION_FAILED, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, RECOGNITION_FAILED, "en");
    else if (iMode==1)
      sprintf(pcmFile, RECOGNITION_FAILED, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayPeopleStrangerPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, PEOPLE_STRANGER, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, PEOPLE_STRANGER, "en");
    else if (iMode==1)
      sprintf(pcmFile, PEOPLE_STRANGER, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayTempnormalWelcomePcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, TEMP_NORMAL_WELCOME, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, TEMP_NORMAL_WELCOME, "en");
    else if (iMode==1)
      sprintf(pcmFile, TEMP_NORMAL_WELCOME, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, true);
}

void YNH_LJX::Audio::Audio_PlayTempabnormalPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, TEMP_ABNORMAL, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, TEMP_ABNORMAL, "en");
    else if (iMode==1)
      sprintf(pcmFile, TEMP_ABNORMAL, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayTempWaringPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, TEMP_WARNING, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, TEMP_WARNING, "en");
    else if (iMode==1)
      sprintf(pcmFile, TEMP_WARNING, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayUnbtMaskPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));

    //sprintf(pcmFile, UN_BTMASK, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, UN_BTMASK, "en");
    else if (iMode==1)
      sprintf(pcmFile, UN_BTMASK, "en");
    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayRecognizedPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, RECORNIZED, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();              
    if (iMode==0)
      sprintf(pcmFile, RECORNIZED, "en");
    else if (iMode==1)
      sprintf(pcmFile, RECORNIZED, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, true);
}
void YNH_LJX::Audio::Audio_PlayStrangerTempNormalPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, STRANGER_TEMP_NORMAL, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, STRANGER_TEMP_NORMAL, "en");
    else if (iMode==1)
      sprintf(pcmFile, STRANGER_TEMP_NORMAL, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayStrangerTempAbnormalPcm(const char *language)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, STRANGER_TEMP_ABNORMAL, language);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, STRANGER_TEMP_ABNORMAL, "en");
    else if (iMode==1)
      sprintf(pcmFile, STRANGER_TEMP_ABNORMAL, "en");    
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, false);
}

void YNH_LJX::Audio::Audio_PlayCustomerPcm(const char *language, char *pcmName, bool sync)
{
    char pcmFile[256];
    memset(pcmFile, 0, sizeof(pcmFile));
    //sprintf(pcmFile, CUSTOMER_PCM_NAME, language, pcmName);
    int iMode=ReadConfig::GetInstance()->getLanguage_Mode();
    if (iMode==0)
      sprintf(pcmFile, CUSTOMER_PCM_NAME, "en",pcmName);
    else if (iMode==1)
      sprintf(pcmFile, CUSTOMER_PCM_NAME, "en",pcmName);
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, pcmFile, sync);
}

void YNH_LJX::Audio::Audio_PlayMedia_volume()
{
    Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, "/isc/res/gui/media_volume.wav", true);
}
