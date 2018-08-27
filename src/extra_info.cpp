/**
 *@file extra_info.cpp
 *@brief 各种算法附加信息基础函数实现文件
 *@version 0.1
 */
#include "include/extra_info.h"
#include <string.h>

/**
 *@brief 清除extra的所有有效位
 *@param extra 算法附加信息
 */
int clear_extra_valid(struct extra_info *extra)
{
	for (int i = 0; i < 32; ++i)
		extra[i].valid = 0;
	
	return 0;
}

/**
 *@brief 设置当前算法描述结构体的extra数组的extra_index位置的默认附加信息
 *@param extra 算法附加信息
 *@param extra_index 附加信息的位置
 *@param extra_name 附加信息的名称
 *@param intarray 附加信息是一个整形数组
 */
int set_extra_intarray(struct extra_info *extra, int extra_index, const std::string &extra_name, std::vector<int> intarray)
{
	extra[extra_index].valid = 1;
	extra[extra_index].extra_name = extra_name;
	extra[extra_index].def_value.dint = intarray[0];
	extra[extra_index].cur_value.dint = extra[extra_index].def_value.dint;
	extra[extra_index].optionvalue = intarray.size();
	for (int i = 0; i < intarray.size(); ++i)
		extra[extra_index].values[i].dint = intarray[i];	
	
	return 0;
}

/**
 *@brief 设置当前算法描述结构体的extra数组的extra_index位置的默认附加信息
 *@param extra 算法附加信息
 *@param extra_index 附加信息的位置
 *@param extra_name 附加信息的名称
 *@param chararray 附加信息是一个字符数组
 */
int set_extra_chararray(struct extra_info *extra, int extra_index, const std::string &extra_name, const std::string &chararray)
{
	extra[extra_index].valid = 1;
	extra[extra_index].extra_name = extra_name;
	strcpy(extra[extra_index].def_value.dchar, chararray.c_str());
	extra[extra_index].cur_value = extra[extra_index].def_value;
	extra[extra_index].optionvalue = 1;
	strcpy(extra[extra_index].values[0].dchar, chararray.c_str());	
	
	return 0;
}
/**
 *@brief 根据extra设置算法的全局的字符集
 *@param charset 算法的全局字符集数组
 *@param extra 算法附加信息
 *@param subcharset_size 子字符集的个数
 */
int expand_charset(char *charset, struct extra_info *extra, int subcharset_size)
{
	int k = 0;
	memset(charset, '\0', 256);
	//初始化全局字符集数组
	for (int i = 0; i < subcharset_size; ++i)
	{
		for (int j = 0; j <= extra[SALT_CHARSET_INDEX].values[0].dchar[2*i+1] - extra[SALT_CHARSET_INDEX].values[0].dchar[2*i]; ++j)
		{
			charset[k++] = extra[SALT_CHARSET_INDEX].values[0].dchar[2*i] + j;
		}
	}
	return 0;
}