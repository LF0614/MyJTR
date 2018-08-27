/**
 *@file bytevector.h
 *@brief 自定义类类型ByteVector的声明文件
 *@version 0.1
 */
#ifndef _BYTEVECTOR_H
#define _BYTEVECTOR_H
#include <stdint.h>
#include <iostream>
#include <string>

#define DEFAULT_CAPACITY 64    //default ByteVector capacity
typedef unsigned char Byte;

class ByteVector
{
    public:
	    /*输出运算符<<，因为第一个参数是ostream类型，所以只能重载为友元函数。eg: cout << ByteVector，*/
	    friend std::ostream& operator<< (std::ostream& os, const ByteVector &bv);
		
	    /*Constructor function*/
		ByteVector(int cap = DEFAULT_CAPACITY, int sz = 0);
		
		/*override the operator [], v[index] ， 单目运算符最好重载为成员函数*/
		Byte& operator[] (int);
		
		Byte& operator[] (int) const;    // const
		
		/*override the operator ==, v1 == v2 */
		friend bool operator== (const ByteVector &lhs, const ByteVector &rhs);
		
		//== 与 != 运算符应该成对出现
		friend bool operator!= (const ByteVector &lhs, const ByteVector &rhs);
		
		/*override the operator =, v1 = v2 */
		ByteVector& operator= (const ByteVector &);
		
		/*override the operator +, v1 + v2, 双目运算符最好重载为友元函数，赋值运算符=除外 */
		friend ByteVector operator+ (const ByteVector &lhs, const ByteVector &rhs);
		
		/*override the operator +=, v1 += Byte */
		ByteVector& operator+= (const Byte &);
		
		/* destructor function */
		~ByteVector();

		int size() const { return byte_size; }
		int capacity() const { return byte_capacity; }
        void resize(int newSize);
		void reserve(int newCapacity);
		bool isEmpty() const { return byte_size ? false : true; }
		Byte *getByte_p() const { return byte_p; }
		
	private:
	    int byte_capacity;
		int byte_size;
		Byte *byte_p;    //element head pointer
};

void BV2chararray(char *hash, const ByteVector &bv);

//! 通用的基础函数，实现ByteVector到string的base64编码的转换
std::string BV2string_base64(const ByteVector &bv);

//! 实现从ByteVector到string的ASCII编码的转换
std::string BV2string_raw(const ByteVector &bv);

//! 通用的基础函数，实现从ASCII可见字符串到ByteVector的转换
ByteVector string2BV_raw(const std::string &s);

//! 实现从ASCII的字符串到UCS-2的ByteVector的转化
ByteVector string2BV_UCS2(const std::string &s);

/*构造一个通用的基础函数，实现get_random_bit()*/
//ByteVector set_random_bit(int bits, const std::string &salt_charset);

//! 通用的基础函数，实现set_random_charset
ByteVector set_random_charset(int bits, char *charset, int charset_size);
//! 通用的基础函数，实现MD5
ByteVector MD5(const ByteVector &pwd);

#endif
