#include "config.hpp"
#include "CommonFuc.h"
#include <fstream>
#include <iostream>


using namespace std;

bool Config::open(const std::string& filename){
	ifstream fin;
	fin.open(filename.c_str());
	if (!fin){
		cerr<<"[Error]:file "<<filename<<" not exist"<<endl;
		return false;
	}
	string line= "";
	while(!fin.eof()){
		getline(fin,line);
		if (line.length()==0 || line[0]=='#')
			continue;
		vector<string> tmp = COMMON::sepString(line,"=");
		_kv[tmp[0]] = tmp[1];
	}
	return true;
}

int Config::getInt(const std::string& key){
	std::map<std::string,std::string>::const_iterator it;
	it=_kv.find(key);
	if (it==_kv.end()){
		cerr<<"config key "<<key<<" not exist"<<endl;
		return 0;
	}else{
		return COMMON::Transfer<string,int>::convert(it->second);
	}
}
std::string Config::getString(const std::string& key){
	std::map<std::string,std::string>::const_iterator it;
	it=_kv.find(key);
	if (it==_kv.end()){
		cerr<<"config key "<<key<<" not exist"<<endl;
		return 0;
	}else{
		return it->second;
	}
}



