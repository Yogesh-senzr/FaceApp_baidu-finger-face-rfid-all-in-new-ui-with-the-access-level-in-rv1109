#ifndef __BDCA_INTERFACE_H__
#define __BDCA_INTERFACE_H__

#include <stdint.h>
#include "bdca_types.h"
#include <vector>
#ifdef __cplusplus
namespace bdca {
#else
extern "C" {
#endif
/**
 * \brief get bdca libray version, the string length is less than 50 bytes 
 * \param[out] version the string of bdca library e.g. 1.35.2
 * \return BDCA_SUCCESS on success, otherwise an error code
 * \attention it is user's responsbility to ensure that the length of version
 *            is longer than 20 bytes
 */
BDCA_STATUS bdca_get_version(char version[20]);

/**
 * \brief set i2c information manually, used to specify which i2c bus you want to work on
 * \param[in] bus bus number, ranges from [0, BDCA_MAX_I2C)
 * \param[in] addr slave i2c address, 0x00 indicates that using default i2c address(0xC8)
 * \return BDCA_SUCCESS on success, otherwise an error code
 */
BDCA_STATUS bdca_set_i2c_bus(uint8_t bus, uint8_t addr = 0x00);

/**
 * \brief get device sn, automatically detecting device if no device has ever been detected
 * \param[out] sn the device sn which is 9 bytes long
 * \return BDCA_SUCCESS on success, otherwise an error code
 */
BDCA_STATUS bdca_get_sn(uint8_t sn[BDCA_SN_SIZE]);

/**
 * \brief do hardware crypto authentication with user specified secure key
 * \param[in]  secure_key 32 bytes long key, take care of it and don't reveal
 * \param[in]  key_len the length of secure_key, it must be 32 bytes
 * \return BDCA_SUCCESS on authentication success, otherwise an error code
 */
BDCA_STATUS bdca_do_crypto_auth(struct CryptoInfo* crypto_info_ptr,int bus_num = -1);
BDCA_STATUS bdca_do_crypto_auth(int bus_num = -1);
BDCA_STATUS bdca_do_crypto_auth(int slot_id, struct CryptoInfo* crypto_info_ptr);

/**
 * \brief get the max number of hardware crypto authentication
 * \param[out] max the maximum number, -1 indicates unlimited number
 * \return BDCA_SUCCESS on success, otherwise an error code
 */
BDCA_STATUS bdca_get_max_auth_num(int* max);

/**
 * \brief get the available number of hardware crypto authentication left at the moment
 * \param[out] available current available authentication number, -1 indicates unlimited number
 * \return BDCA_SUCCESS on success, otherwise an error code
 */
BDCA_STATUS bdca_get_available_auth_num(int* available);

/**
 * \brief set the hardware crypto authentication number unlimited, this is
 *        useful during debug phase
 * \return BDCA_SUCCESS on success, otherwize an error code
 */
BDCA_STATUS bdca_set_unlimited_auth_num(void);

BDCA_STATUS bdca_sha256(const uint8_t rng[32], const uint8_t key[32],
    const uint8_t sn[9], int slot_id, bool use_sn, uint8_t digest[32]);
#ifdef __cplusplus
} // namespace bdca
#else
}
#endif

#endif // __BDCA_INTERFACE_H__
