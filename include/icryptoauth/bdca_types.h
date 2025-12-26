#ifndef __BDCA_TYPES_H__
#define __BDCA_TYPES_H__

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
namespace bdca {
#endif

#define PRINT_LOG(fmt, ...)                                         \
    do {                                                            \
        printf("%s[%d]: " fmt, __func__, __LINE__, ##__VA_ARGS__);   \
        printf("\n");                                               \
    } while (0)

#define I2C_ADDR_VALID(x) (x != 0x00)

/// the most frequently used constant value
enum {
    BDCA_NONCE_SIZE = 32,        //!< length of NONCE(i.e. number used once)
    BDCA_DIGEST_SIZE = 32,       //!< length of digest which is calculated by host software
    BDCA_MAC_SIZE = 32,          //!< length of MAC(i.e. message authentication code)
    BDCA_SECURE_KEY_SIZE = 32,   //!< length of secure key
    BDCA_SW_SHA_DATA_SIZE = 88,  //!< length of software sha data
    BDCA_SN_SIZE = 9,            //!< length of device SN
    BDCA_REV_SIZE = 4,           //!< length of device revision
    BDCA_MAX_I2C = 12,           //!< maximum I2C bus number the SDK supports
    BDCA_MAX_KEYS = 16,          //!< maximum secure key number
    BDCA_CONFIG_ZONE_SIZE = 128, //!< length of configuration zone in device
    BDCA_OTP_ZONE_SIZE = 64,     //!< length of OTP zone in device
};

typedef enum {
    BDCA_SUCCESS = 0,          //!< function works successfully
    BDCA_FAILURE = -1,         //!< some error occurs
    BDCA_NO_DEVICE = -2,       //!< no crypto auth device found
    BDCA_NO_RESPONSE = -3,     //!< no response from device
    BDCA_BAD_PARAM = -4,       //!< bad parameters you offered
    BDCA_COMM_FAIL = -5,       //!< communicate with device fail, the device must be detected first
    BDCA_CMP_DIGEST_FAIL = -6, //!< compare digest and MAC failure meaning authentication fail
    BDCA_TOO_MANY_KEYS = -7,   //!< key number is out of 16
    BDCA_KEY_TOO_SHORT = -8,   //!< key length is less than 32 bytes
    BDCA_NO_LEGAL_CHANCE = -9, //!< the permitted crypto auth chances are exhausted
} BDCA_STATUS;

typedef struct {
    uint8_t bus;  //!< i2c bus number, range from 0 to 7 that is total 8 i2c buses supported, 0xFF is invalid
    uint8_t addr; //!< i2c address of 8 bits wide, 0x00 is invalid
} I2cInfo_t;

struct CryptoInfo {
//    uint8_t key[32];
    uint8_t digest[32];
    uint8_t rng[32];
    uint8_t sn[9];
    uint8_t slot_id;
};

#ifdef __cplusplus
} // namespace bdca
}
#endif

#endif // __BDCA_TYPES_H__
