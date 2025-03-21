#ifndef WIEGAND_H
#define WIEGAND_H

namespace YNH_LJX{
class Wiegand
{
public:
    static int Wiegand_OpenInputWiegand();
    static int Wiegand_ReadInputWiegand(unsigned char *pData, unsigned int nDataSize);
    static void Wiegand_CloseInputWiegand();
    static int Wiegand_OpenOutputWiegand();
    static void Wiegand_WriteOutputWiegand(unsigned char *pData, unsigned int nDataSize);
    static void Wiegand_CloseOutputWiegand();

    static void setWiegandReverse(const int state);
private:
    static int nWiegandReadFd;
    static int nWiegandWriteFd;
    static int nWiegandReverse;
};

}
#endif // WIEGAND_H
