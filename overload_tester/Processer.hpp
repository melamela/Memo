#ifndef __PROCESSER_HPP__
#define __PROCESSER_HPP__

#include "worker.hpp"
#include "config.hpp"
#include "SocketClient.h"
#include <string>
#include <vector>

class Processer{
public:
	Processer(){};
	~Processer(){
		for (int i=0;i<_sockets.size();i++){
			delete _sockets[i];
		}
		_sockets.clear();
	}
public:
	int doProc(Worker*& worker);
	bool init(Config& config);
public:
	std::vector<SocketClient*> _sockets;
};


#endif		//__PROCESSER_HPP__

