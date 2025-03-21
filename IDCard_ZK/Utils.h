#ifndef GUI_INCLUDE_UTILS_H_
#define GUI_INCLUDE_UTILS_H_

#include "Common.h"
#include "json-cpp/json.h"

bool  CheckUsbStorageExist();
int get_GpioValue(unsigned int gpio_chip_num,unsigned int gpio_offset_num);
int set_GpioValue(unsigned int gpio_chip_num,unsigned int gpio_offset_num,unsigned int gpio_out_val);

std::string Utils_DoPostRawJson(std::string url, Json::Value &data);

#endif /* GUI_INCLUDE_UTILS_H_ */
