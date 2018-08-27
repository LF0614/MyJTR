/**
 *@file getcipher.cpp
 *@brief 处理用户输入命令行的主函数文件
 *@version 0.1
 */
/*
 * 程序使用命令：
 *   ./getcipher alg_name pwd [extra_name=extra_value]
 * 输入：
 *   alg_name    算法名称    字符串(不超过31字节)    来自于标准算法名称表
 *   pwd         口令        口令                    一般为ASCII可输入字符
 *   extra_name  附加信息名称 字符串(不超过31字节)   典型的名称包括
 *               salt_len    salt长度    //0号位置 SALT_LEN_INDEX = 0
 *               iter_count  迭代次数    //1号位置 ITER_COUNT_INDEX = 1
 *               pwd_len     口令长度    //2号位置 PWD_LEN_INDEX = 2
 *               ssid        WPA的SSID
 *               其余的根据特定算法决定
 *   extra_value 附加信息数值 字符串(不超过31字节)或者整数    数据类型由extra_name决定
 *
 * author: liufeng
 * date:   2018/8/1
 */

#include "include/extra_info.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <regex>

#define _MAIN_DEBUG

/*struct ptrCmp
{
	bool operator() (const char *s1, const char *s2) const
	{
		return (strcmp(s1, s2) < 0);
	}
};*/
//加密算法名和算法描述结构体的映射
//static std::map<char *, struct alg_desp, ptrCmp> myAlgMap;
static std::map<std::string, struct alg_desp> myAlgMap;   

//存储附加信息和对应的正则表达式
static std::map<std::string, std::string> extra_value_pattern; 

//存储解析命令行后得到的附加信息和值
static std::map<std::string, std::string>extra_name_value;  

/**************引用外部算法描述，用来在main()中注册******************/

extern struct alg_desp wordpress_alg_desp;

/********************************************************************/

//根据用户输入的算法名得到的算法描述
static struct alg_desp cur_alg_desp;    

static std::ifstream file_in;
static std::ofstream file_out;

/**
 *@brief 注册所有加密算法描述结构体以及附加信息值的格式正则表达式
 */
int register_all()
{
	//! 1. 注册算法名和对应算法描述的映射
	myAlgMap.insert(std::make_pair(std::string("wordpress"), wordpress_alg_desp));
	//! 2. 注册附加信息的值对应的正则表达式规则
	extra_value_pattern.insert(std::make_pair(std::string("salt_len"), std::string("\\d+")));    //匹配任何一个数字字符
	extra_value_pattern.insert(std::make_pair(std::string("iter_pos"), std::string("\\S")));    //匹配任何一个可见字符
	//匹配[*-*]这样的1个或多个字符集
	extra_value_pattern.insert(std::make_pair(std::string("salt_charset"), std::string("(\\[[0-9A-Za-z\\./]{1}\\-[0-9A-Za-z\\./]{1}\\])+")));
	
	return 0;
}

/**
 *@brief 打开输入文件
 *@param file_in 文件指针
 *@param infile_path 输入文件路径
 */
int open_infile(std::ifstream &file_in, const char *infile_path)
{
	file_in.open(infile_path);    //默认以ifstream::in方式打开
	if (!file_in)
	{
		std::cout<<"error: open pwd_file " << infile_path << " failed!" << std::endl;
		return -1;
	}
}
/**
 *@brief 打开输出文件
 *@param file_out 文件指针
 *@param outfile_path 输出文件路径
 */
int open_outfile(std::ofstream &file_out, const char *outfile_path)
{
	file_out.open(outfile_path);    //默认以ofstream::out | ofstream::trunc方式打开
	if (!file_out)
	{
		std::cout<<"error: open cipher_file " << outfile_path << " failed!" << std::endl;
		return -1;
	}
}
/**
 *@brief 解析命令行，判断用户输入的信息格式是否正确
 *@param argc 命令行参数的个数
 *@param argv 命令行字符串参数数组
 */
int parse_cmdline(int argc, char **argv)
{
	//! 1. 判断用户输入命令行基本参数个数是否正确
	if (argc < 4)
	{
		printf("argc = %d, Usage: ./getcipher alg_name pwd_file cipher_file [extra_name=extra_value]\n", argc);
		return -1;
	}
	
	//! 2. 判断map中是否存在用户输入的算法名
	if (myAlgMap.count(argv[1]) <= 0)
	{
		printf("error: alg_name:%s is not exist!\n", argv[1]);
		return -1;
	}
	
	//! 3. 打开输入输出文件，不正确报错退出
	
    if (open_infile(file_in, argv[2]) == -1)
		return -1;
	
	if (open_outfile(file_out, argv[3]) == -1)
		return -1;
	//! 4. 处理用户输入的附加信息
    for (int i = 4; i < argc; ++i)
	{
		//! 4.1 利用‘=’切割获取到附加信息名称和值
		std::string str_extra_info = argv[i];
        int index = 0;
		index = str_extra_info.find('=');
		if (index == std::string::npos)
		{
			std::cout<<"error: [no '='], the format of extra_info" << str_extra_info << " is wrong!"<<std::endl;
			return -1;
		}
		std::string str_extra_name = str_extra_info.substr(0, index);
		std::string str_extra_value = str_extra_info.substr(index + 1);
		
		//! 4.2 首先检查附加信息名称是否在名称-模式map中存在
		if (extra_value_pattern.count(str_extra_name) <= 0)
		{
			std::cout << "error: extra_name: " << str_extra_name << " is not valid!" << std::endl;
			return -1;
		}
		//! 4.3 再利用c++正则表达式，检查值的格式是否正确
		try {
			std::regex r(extra_value_pattern[str_extra_name]);
			if (!std::regex_match(str_extra_value, r))
			{
				std::cout << "error: extra_value: " << str_extra_value << " is not valid!" << std::endl;
				return -1;
			}
		} catch (std::regex_error e) {
			std::cout << e.what() << "\nerror code: " << e.code() << std::endl;
			return -1;
		}
		//! 4.4 如果附加信息是salt_charset，就先处理成09az这种形式
		if (str_extra_name == std::string("salt_charset"))
		{
			//! 4.4.1 计算字符集的子集个数
			int optionValue = 0;   
			for (int i = 0; i < str_extra_value.size(); ++i)
			{
				if (str_extra_value[i] == '[')
					++optionValue;
			}
			//! 4.4.2 把附加信息值(eg:[0-9][a-z])解析成09az存储在std::string中
			std::string str_value_tmp = str_extra_value;
			int index;
			std::string charset;
			for (int i = 0; i < optionValue; ++i)
			{
				index = -1;
				index = str_value_tmp.find('-');
				charset.push_back(str_value_tmp[index - 1]);
				charset.push_back(str_value_tmp[index + 1]);
				str_value_tmp = str_value_tmp.substr(index + 3);    //[0-9][a-z]，从”-“跳到”[a-z]",越过3个字符每次
			}
			//！4.4.3 把附加信息名称和值添加到名称-值map中
			extra_name_value.insert(std::make_pair(str_extra_name, charset));
			continue;
		}
		//! 4.5 把正确识别的附加信息名称和值插入名称-值map中
		extra_name_value.insert(std::make_pair(str_extra_name, str_extra_value));    
	}
	
	return 0;
}
/**
 *@brief main()函数，处理用户输入命令行参数，调用相应加密算法产生密文文件
 */
int main(int argc, char **argv)
{
	
	//在main()中注册所有算法，以及附加信息与其对应值的正则表达式
	if (register_all() != 0)
    {
		std::cout<<"error: register_all() is wrong!"<<std::endl;
		return -1;
	}
	
	if (parse_cmdline(argc, argv) != 0)
	{
		std::cout<<"error: parse_cmdline() is wrong!"<<std::endl;
		return -1;
	}
	
	#ifdef _MAIN_DEBUG
		std::map<std::string, std::string>::iterator it;
		for (it = extra_name_value.begin(); it != extra_name_value.end(); ++it)
		{
			std::cout << "map extra_name_value: key = " << it->first << ", value = " << it->second << std::endl;
		}
	#endif
	
	//根据算法名得到算法描述结构体
	cur_alg_desp = myAlgMap[argv[1]];
	
	//初始化算法描述结构体
	if (cur_alg_desp.init_alg_desp(cur_alg_desp.extra) != 0)
	{
		std::cout << "error: init_alg_desp() is wrong!" << std::endl;
		return -1;
	}
	
	//再次检查用户输入的信息，设置算法描述结构体的当前值
	if (cur_alg_desp.check_cmdline(cur_alg_desp.extra, extra_name_value) != 0)
	{
		std::cout << "error: check_cmdline() is wrong!" << std::endl;
		return -1;
	}
	
	std::string pwd;
	while (getline(file_in, pwd))    //每次从口令文件读取一行口令进行加密处理
	{
		/*********************************算法过程*********************************/
		ByteVector bv_salt;
		bv_salt = cur_alg_desp.get_random_salt(cur_alg_desp.extra);
		if (bv_salt.isEmpty())
		{
			std::cout<<"error: get_random_salt() is wrong!"<<std::endl;
			return -1;
		}

		ByteVector bv_pwd;
		bv_pwd = cur_alg_desp.prepare_pwd(pwd); 
		if (bv_pwd.isEmpty())
		{
			std::cout<<"error: prepare_pwd() is wrong!"<<std::endl;
			return -1;
		}

		ByteVector bv_hash;
		bv_hash = cur_alg_desp.hash_pwd(bv_pwd, bv_salt, cur_alg_desp.extra); 
		if (bv_hash.isEmpty())
		{
			std::cout<<"error: hash_pwd() is wrong!"<<std::endl;
			return -1;
		}
		
		std::string cipher;
		cipher = cur_alg_desp.get_cipher(bv_hash, bv_salt, cur_alg_desp.extra);
		if (cipher.empty())
		{
			std::cout<<"error: get_cipher() is wrong!"<<std::endl;
			return -1;
		}
		#ifdef _MAIN_DEBUG
			file_out << cipher << std::endl;
		#endif
	}
	file_in.close();
	file_out.close();
	
	return 0;
}
