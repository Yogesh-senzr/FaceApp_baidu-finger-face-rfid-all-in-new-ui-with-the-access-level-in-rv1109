#ifndef HASHMD5_H
#define HASHMD5_H

#include <string>
// ---- md5摘要哈希 ---- //
void Hashmd5(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr);
// ---- sha256摘要哈希 ---- //
void Hashsha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr);

#endif // HASHMD5_H
