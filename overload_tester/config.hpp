#ifndef __MYCONFIG_H__
#define __MYCONFIG_H__

#include <string>
#include <map>


class Config{
public:
	std::map<std::string,std::string> _kv;
public:
	bool open(const std::string& filename);
	int getInt(const std::string& key);
	std::string getString(const std::string& key);
};



#endif	//__MYCONFIG_H__


