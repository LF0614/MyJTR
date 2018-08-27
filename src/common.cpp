/**
 *@file common.cpp
 *@brief 供各种算法使用的通用基础函数
 *@version 0.1
 */
#include "include/common.h"
#include <stdint.h>
#include <openssl/md5.h>

//! wordpress算法base64加密算法的字符集
unsigned char base64Char2[]= "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/**
 *@brief base64加密算法实现
 *@param hash 要加密的hash值
 *@param count hash值的字节数
 *@param base64Code base64加密后的base64密文
 */
void encode64(const unsigned char *hash, int count, unsigned char *base64Code)
{
	uint32_t value;
	int i = 0, j = 0;
	do {
		value = hash[i++];
		base64Code[j++] = base64Char2[value & 0x3f];
		if (i < count)
			value |= hash[i] << 8;
		base64Code[j++] = base64Char2[(value >> 6) & 0x3f];
		if (i++ >= count)
			break;
		if (i < count)
			value |= hash[i] << 16;
		base64Code[j++] = base64Char2[(value >> 12) & 0x3f];
		if (i++ >= count)
			break;
		base64Code[j++] = base64Char2[(value >> 18) & 0x3f];
	} while (i < count);
}
/**
 *@brief md5加密算法实现
 *@param hash 存储md5哈希值的16字节数组
 *@param pwd 需要加密的口令
 */
void md5(unsigned char*hash, const std::string &pwd)
{
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, pwd.c_str(), pwd.size());
	MD5_Final(hash, &ctx);
}