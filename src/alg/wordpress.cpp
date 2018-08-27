/**
 *@file wordpress.cpp
 *@brief wordpress算法的口令产生过程
 *@version 0.1
 */
#include "../include/extra_info.h"
#include "../include/bytevector.h"
#include "../include/wordpress.h"
#include <time.h>    //time(NULL)
#include <stdio.h>
#include <stdlib.h>    //atoi();
#include <string.h>    //memset();

#define _WORDPRESS_DEBUG

//! wordpress算法的盐字符集
static char wordpress_charset[256];

/**
 *@brief wordpress算法的初始化
 *@param extra 算法的附加信息 
 */
int wordpress_init_alg_desp(struct extra_info *extra)
{
	//! 1.清除所有extra信息
	clear_extra_valid(extra);
	//! 2.盐长度salt_len为64位或者32位，缺省为64为
    set_extra_intarray(extra, SALT_LEN_INDEX, "salt_len", std::vector<int>{64, 32}); 
	//! 3.迭代次数标识iter_pos，缺省为"B"
	set_extra_chararray(extra, ITER_POS_INDEX, "iter_pos", std::string("B"));
	//! 4.盐的字符集合为[a-z][0-9][A-Z][.-.][/-/]
	set_extra_chararray(extra, SALT_CHARSET_INDEX, "salt_charset", std::string("09azAZ./"));
	//! 5.扩展盐字符集合到wordpress_charset数组
	expand_charset(wordpress_charset, extra, 5);    //09 az AZ . /共5个子字符集
	
	return 0;
}
/**
 *@brief 根据输入修改wordpress算法的配置
 *@param extra wordpress算法的附加信息
 *@param extra_name_value 输入配置的名称和数值对
 */
int wordpress_check_cmdline(struct extra_info *extra, std::map<std::string, std::string> &extra_name_value)
{
	std::map<std::string, std::string>::iterator it;
	//! 1. 遍历输入配置的名称和数值对
	for (it = extra_name_value.begin(); it != extra_name_value.end(); ++it)
	{
		//! 1.1 设置盐长度salt_len
		if (it->first == std::string("salt_len"))
			extra[SALT_LEN_INDEX].cur_value.dint = atoi(it->second.c_str());
		//! 1.2	设置迭代次数标识iter_pos
		else if (it->first == std::string("iter_pos"))
			strcpy(extra[ITER_POS_INDEX].cur_value.dchar, it->second.c_str());
		//! 1.3	设置盐字符集salt_set
		else if (it->first == std::string("salt_charset"))
		{
			memset(extra[SALT_CHARSET_INDEX].values[0].dchar, '\0', 32);
			
			//! 1.3.1 设置子字符集个数
			extra[SALT_CHARSET_INDEX].optionvalue = it->second.size() / 2;
			
			//! 1.3.2 在extra_info.values[0].dchar[32]中设置每个子集，dchar[2n]代表下界，dchar[2n+1]代表上界
			strcpy (extra[SALT_CHARSET_INDEX].values[0].dchar, it->second.c_str()); 
			
			//! 1.3.3 根据用户输入的字符集，更新wordpress字符集数组wordpress_charset
			if (expand_charset(wordpress_charset, extra, extra[SALT_CHARSET_INDEX].optionvalue) != 0)
			{
				std::cout << "error: wordpress_check_cmdline().set_extra_charset() is wrong!" << std::endl;
				return -1;
			}
		}
		else
		{
			std::cout << "wordpress_check_cmdline(): " << it->first << " is not valid" << std::endl;
			return -1;
		}
	}
	
	return 0;
}
/**
 *@brief 产生指定长度的随机盐
 *@param extra wordpress的附加信息，盐的位数由extra[SALT_LEN_INDEX].cur_value.dint决定
 *@return 二进制盐值，存储于Bytevector对象中
 */
ByteVector wordpress_get_random_salt(struct extra_info *extra)
{
	ByteVector bv_salt;
	//! 1. 根据字符集合产生盐
	bv_salt = set_random_charset(extra[SALT_LEN_INDEX].cur_value.dint, wordpress_charset, strlen(wordpress_charset));
	#ifdef _WORDPRESS_DEBUG
		std::cout<<"salt_len_in_bit="<<extra[SALT_LEN_INDEX].cur_value.dint<<std::endl;
		std::cout<<"bv_salt="<<bv_salt<<std::endl;
	#endif
	//! 2. 返回得到的盐
	return bv_salt;
	
}
/**
 *@brief 对口令进行预处理
 *@param pwd 输入的口令字符串
 *@return 二进制口令值，存储于自定义Bytevector对象中
 */
ByteVector wordpress_prepare_pwd(std::string &pwd)
{
	ByteVector bv_pwd_raw;
	//! 1. 将string类型的口令转为二进制值，wordpress口令二进制值为口令ASCII码
	bv_pwd_raw = string2BV_raw(pwd);
	
	return bv_pwd_raw;
}

/**
 *@brief 根据pwd,salt(以字节为单位)，附加信息产生wordpress算法二进制hash值
 *@param pwd 二进制口令值
 *@param salt8 8个字节的二进制颜值
 *@param extra wordpress算法的附加信息
 *@return 16字节的二进制hash值，存储于ByteVector对象中
 */
ByteVector wordpress_hash_pwd(ByteVector &pwd, ByteVector &salt8, struct extra_info *extra)
{
	ByteVector result16;
	
	//! 1. 根据extra[ITER_POS_INDEX].cur_value.dchar[0]获取迭代次数iter_count
	uint32_t iter_count = 1u << tbl[extra[ITER_POS_INDEX].cur_value.dchar[0]];
	if (iter_count > 0x80000000)
	{
		std::cout<<"error:iter_count is wrong in wordpress"<<std::endl;
		return result16;
	}
	//! 2. result16=MD5(salt8+pwd);
	result16 = MD5(salt8 + pwd);
	//! 3. 循环iter_count次
	while (iter_count--)
		//! 3.1 result16 = MD5(result16 + pwd);
		result16 = MD5(result16 + pwd);	
    //! 4. 返回16字节的二进制hash值
    return result16;		
}
/**
 *@brief 根据hash,salt,附加信息产生符合hashcat规范的密文字符串
 *@param hash16 16字节hash值
 *@param salt8 8字节salt值
 *@param extra wordpress算法的附加信息
 *@return wordpress的密文字符串
 */
std::string wordpress_get_cipher(ByteVector &hash, ByteVector &salt, struct extra_info *extra)
{
	//$P$984478476IagS59wHZvyQMArzfx58u.
	//$P$ iter_pos 8个任意字符的盐  22个base64编码字符
	std::string cipher;
	
	std::string salt8(1, extra[ITER_POS_INDEX].cur_value.dchar[0]);    //string(size_t n, char c);
	
	//! 密文格式：$P$ 迭代次数标识 salt hash(base64编码)
	cipher = "$P$" + salt8 + BV2string_raw(salt) + BV2string_base64(hash);
	
	#ifdef _WORDPRESS_DEBUG
		std::cout<<"cipher="<<cipher<<std::endl;
	#endif
	
	return cipher;
}
	
//! wordpress算法的算法描述结构体定义
struct alg_desp wordpress_alg_desp = {
	wordpress_init_alg_desp,
	wordpress_check_cmdline,
	wordpress_get_random_salt,
	wordpress_prepare_pwd,
	wordpress_hash_pwd,
	wordpress_get_cipher,
	"wordpress"
};
