#ifndef I2C_H_
#define I2C_H_

namespace YNH_LJX{
class I2C
{
public:
    static void data_enc(const unsigned char * psrc, unsigned char * pdes);
    static int I2C_Open_EX();
    static int I2C_Write_EX(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nSrcBuflen, unsigned char* szSrcBuf);
    static int I2C_Read_EX(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nDstBufLen, unsigned char* szDstBuf);
    static int I2C_OpenI2C(int nI2CNum);
    static int I2C_CloseI2C(int nFd);
    static int I2C_WriteI2C(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, unsigned int nSrcBuflen, unsigned int* szSrcBuf);
    static int I2C_ReadI2C(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nDstBufLen, unsigned int* szDstBuf);
    static int I2C_CheckAuthorized();
};
}

#endif /* I2C_H_ */
