/**
 *@file extra_info.h
 *@brief 各种算法附加信息基础函数声明文件
 *@version 0.1
 */
#ifndef _EXTRA_INFO_H
#define _EXTRA_INFO_H

#include "bytevector.h"
#include <string>
#include <vector>
#include <map>

#define SALT_LEN_INDEX 0
#define ITER_POS_INDEX 1    //迭代次数标识
#define PWD_LEN_INDEX 2
#define SALT_CHARSET_INDEX 3

union extra_data {    //附加信息数值类型
	char dchar[32];
	int dint;
};

/**
 *@brief 附加信息结构体
 */
struct extra_info {    //附加信息
	int valid;    //1:有效， 0：无效
	std::string extra_name;    //附加信息名称
	union extra_data def_value;    //命令行的参数默认缺省值
	union extra_data cur_value;    //命令行设定的参数当前值
	int optionvalue;    //指示当前values有多少个效值
	union extra_data values[8];    //一个附加信息名称最多有8个可选值
	union extra_data min_value;    //最小值
	union extra_data max_value;    //最大值
};

/**
 *@brief 每一个算法都由一个struct alg_desp结构定义
 * 包括一系列函数指针以及算法名称和附加信息数组
 */
struct alg_desp {
	
	//! 初始化当前的算法描述
	int (*init_alg_desp)(struct extra_info *extra);    
	
	//! 再次检查用户输入的信息，设置算法描述的当前值
	int (*check_cmdline)(struct extra_info *extra, std::map<std::string, std::string> &extra_name_value);    
	
	//! 产生salt_len_in_bit位随机盐
	ByteVector (*get_random_salt)(struct extra_info *extra);     
	
	//! 对pwd进行预处理，例如ASCII码到UCS2的转换    
	ByteVector (*prepare_pwd)(std::string &pwd);
	
	//! 根据pwd,salt，附加信息产生hash值
	ByteVector (*hash_pwd)(ByteVector &pwd, ByteVector &salt, struct extra_info *extra);  

	//! 根据hash,salt,附加信息产生符合hashcat规范的密文字符串
	std::string (*get_cipher)(ByteVector &hash, ByteVector &salt, struct extra_info *extra);	
	//! 算法名称
	std::string alg_name;  
	//! 最多32个有效信息	
	struct extra_info extra[32];    
};

//! 清除extra的所有有效位
int clear_extra_valid(struct extra_info *extra);

//! 设置当前算法描述结构体的extra数组的extra_index位置的默认附加信息，该附加信息的值是整形数组	
int set_extra_intarray(struct extra_info *extra, int extra_index, const std::string&extra_name, std::vector<int> intarray);

//! 附加信息的值是字符数组
int set_extra_chararray(struct extra_info *extra, int extra_index, const std::string&extra_name, const std::string &chararray);

//! 根据extra设置算法的全局的字符集
int expand_charset(char *charset, struct extra_info *extra, int subcharset_size);

#endif