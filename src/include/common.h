/**
 *@file common.h
 *@brief 共各种算法使用的通用函数声明
 *@version 0.1
 */

#ifndef _COMMON_H
#define _COMMON_H
#include <string>
//! wordpress算法base64加密算法的字符集声明
extern unsigned char base64Char2[65];    //wordpress

//! base64加密算法声明
void encode64(const unsigned char *hash, int count, unsigned char *base64Code);
//! md5加密算法声明
void md5(unsigned char*hash, const std::string &pwd);

#endif