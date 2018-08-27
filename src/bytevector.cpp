/**
 *@file bytevector.cpp
 *@brief 自定义类类型ByteVector的实现文件
 *@version 0.1
 */
#include "include/bytevector.h"
#include "include/common.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

//! 默认构造函数
ByteVector::ByteVector(int cap, int sz)
    : byte_capacity(cap), byte_size(sz)
{
    this->byte_p = new Byte[byte_capacity];
}
//! 析构函数
ByteVector::~ByteVector()
{
	delete[] byte_p;
}

/**
 *@brief 重载的<<运算符
 * 输出运算符<<，因为第一个参数一定是ostream类型，所以只能重载为友元函数.
 */
std::ostream& operator<< (std::ostream& os, const ByteVector &bv)
{
	for (int i = 0; i < bv.size(); ++i)
		os << bv[i];
	return os;
}
//! 重载非常量版本的下标运算符
Byte& ByteVector::operator[] (int index)
{
    return this->byte_p[index];
}
//! 重载常量版本的下标运算符
Byte& ByteVector::operator[] (int index) const
{
    return this->byte_p[index];
}
//! 重载关系运算符==，双目运算符一般重载为友元函数
bool operator== (const ByteVector &lhs, const ByteVector &rhs)
{
	/*if (strcmp((char *)this->byte_p, (char*)rhs->getByte_p()) == 0)
		return true;*/   //‘\0' will stop cmp
	if (&lhs == &rhs && lhs.size() == rhs.size() && lhs.capacity() == rhs.capacity())
	{
		for (int i = 0; i < lhs.size(); ++i)
		{
			if (lhs[i] != rhs[i])
				return false;
		}
		return true;
	}
	else
		return false;
}
//! 重载关系运算符!=，应该成对实现
bool operator!= (const ByteVector &lhs, const ByteVector &rhs)
{
	if (rhs == rhs)
		return false;
	return true;
}
//! 重载拷贝赋值运算符
ByteVector& ByteVector::operator= (const ByteVector &rhs)
{
	if (*this == rhs)    //avoid copying itself, call operator==()
		return *this;
	delete[] byte_p;    //free old array space
	this->byte_size = rhs.size();
	this->byte_capacity = rhs.capacity();
	this->byte_p = new Byte[this->byte_size];    //allocate new array space
	for (int i = 0; i < this->byte_size; ++i)    //copy element of rhs to new array
		this->byte_p[i] = rhs[i];
	return *this;

}
//! 重载+运算符，双目运算符应为友元函数
ByteVector operator+ (const ByteVector &lhs, const ByteVector &rhs)
{
	ByteVector result;
	
	int llength = lhs.size();
	int rlength = rhs.size();
	
	if (llength + rlength > result.capacity())
		result.reserve(2 * (llength + rlength));
	
	int i, j;
	for (i = 0; i < llength; ++i)    //not using strcpy(), avoid '\0'
		result[i] = lhs[i];
	for (j = 0; j < rlength; ++j)
		result[i++] = rhs[j];
	
	result.resize(llength + rlength);
	
	return result;
}
//! 重载+=运算符
ByteVector& ByteVector::operator+= (const Byte &rhs)
{
	int length = this->byte_size;
	if (length + 1 < this->byte_capacity)
	{
		byte_p[length++] = rhs;
		resize(length);
	}
	else
	{
		reserve(2 * length);
		byte_p[length++] = rhs;
		resize(length);
	}
	return *this;
}

//! 改变容器ByteVector的size大小
void ByteVector::resize(int newSize)
{ 
	if (newSize < byte_size)
		return;
	else
		byte_size = newSize; 
}

//! 给容器ByteVector重新分配内存空间
void ByteVector::reserve(int newCapacity)
{ 
	if (newCapacity < byte_size)
		return;
	else
	{
	    uint8_t *temp = byte_p;    //copy old array to temporary array
		byte_p = new Byte[newCapacity];    //allocate a bigger new array
		for (int i = 0; i < byte_size; ++i)    //copy old data to new array, but byte_size is not changed!
			byte_p[i] = temp[i];
		
		byte_capacity = newCapacity;
		delete[] temp;    //free the old array
	}
}
//! 实现ByteVector到char数组的转换
void BV2chararray(char *hash, const ByteVector &bv)
{
	for (int i = 0; i < bv.size(); ++ i)
		hash[i] = bv[i];
}

//! 实现ByteVector到string的base64编码的转换
std::string BV2string_base64(const ByteVector &bv)
{
	unsigned char base64Code[22];
	char *hash = new char[bv.size()];
	
	BV2chararray(hash, bv);
	
	encode64((unsigned char *)hash , bv.size(), base64Code);    //hash.size() == 16
	
	std::string str = std::string((char *)base64Code, 22);
	return str;
}
//! 实现ByteVector到string的ASCII编码的转换
std::string BV2string_raw(const ByteVector &bv)
{
	std::string str((char *)bv.getByte_p(), bv.size());
	return str;
}
//! 实现从ASCII可见字符串到ByteVector的转换
ByteVector string2BV_raw(const std::string &s)
{
	ByteVector bv;
    for (int i = 0; i < s.size(); ++i)
	{
		Byte b = s[i];
		bv += b;
	}
	return bv;
}
//! 实现从ASCII的字符串到UCS-2的ByteVector的转化
ByteVector string2BV_UCS2(const std::string &s)
{
	ByteVector bv;
	Byte b = 0x00;
	for (int i = 0; i < s.size(); ++i)
	{	
		bv += b;
		bv += s[i];
	}
	for (int i = 0; i < 2 * s.size(); ++i)
	return bv;
}
/**
 *@brief 按照位数和字符集，实现获取一个随机位数的盐
 *@param bits 盐的位数
 *@param charset 盐的字符集数组
 *@param charset_size 盐字符集数组的大小
 */
ByteVector set_random_charset(int bits, char *charset, int charset_size)
{
	ByteVector bv;
	
	for (int i = 0; i < bits/8; ++i)
	{
		int rand_num = rand() & charset_size - 1;
		Byte b = charset[rand_num];
		bv += b;
	}
	
	return bv;
}
//! ByteVector的MD5算法，调用底层的md5函数
ByteVector MD5(const ByteVector &pwd)
{
	ByteVector result;
	unsigned char hash[16];
	memset(hash, '\0', 16);
	md5(hash, BV2string_raw(pwd));
	for (int i = 0; i < 16; ++i)
		result += hash[i];
	return result;
}