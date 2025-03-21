#ifndef AUDIO_H_
#define AUDIO_H_

namespace YNH_LJX
{
class Audio
{
public:
    static void setVolume(const int);
    static void Audio_InitDev();
    static void Audio_ExitDev();
    /*识别失败*/
    static void Audio_PlayRecognitionFailedPcm(const char *language);
    /*陌生人*/
    static void Audio_PlayPeopleStrangerPcm(const char *language);
    /*温度正常*/
    static void Audio_PlayTempNormalPcm(const char *language);
    /*温度正常，欢迎*/
    static void Audio_PlayTempnormalWelcomePcm(const char *language);
    /*温度异常*/
    static void Audio_PlayTempabnormalPcm(const char *language);
    /*警报声*/
    static void Audio_PlayTempWaringPcm(const char *language);
    /*提示戴口罩*/
    static void Audio_PlayUnbtMaskPcm(const char *language);
    /*识别到成功播报欢迎*/
    static void Audio_PlayRecognizedPcm(const char *language);
    /*陌生人温度正常播报*/
    static void Audio_PlayStrangerTempNormalPcm(const char *language);
    /**/
    static void Audio_PlayStrangerTempAbnormalPcm(const char *language);
    /*客户自己决定播放的pcm*/
    static void Audio_PlayCustomerPcm(const char *language, char *pcmName, bool sync);
    /**/
    static void Audio_PlayMedia_volume();
};
}

#endif /* AUDIO_H_ */
