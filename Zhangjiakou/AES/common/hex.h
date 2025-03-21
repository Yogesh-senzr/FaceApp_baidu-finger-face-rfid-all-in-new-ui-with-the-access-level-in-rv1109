#ifndef HEX_H
#define HEX_H

#include <stdio.h>
#include <string>

std::string String2Hex(const unsigned char *src, size_t len);
std::string Hex2String(const std::string &hex);

#endif // HEX_H
