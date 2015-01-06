#include <sys/time.h>
#include "Processer.hpp"
#include "CommonFuc.h"


using namespace std;

bool Processer::init(Config& config){
	std::vector<string> addrs = COMMON::sepString(config.getString("DEST_SERVER_ADDR"),";");
	int timeout = config.getInt("PROCESSER_TIMEOUT");
	int buff_size = config.getInt("SOCKET_BUFF_SIZE");
	int write_resp = config.getInt("SOCKET_WRITE_RESP");
	for (int i=0;i<addrs.size();i++){
		SocketClient* socket = new SocketClient;
		socket->init(addrs[i],timeout,buff_size,write_resp);
		_sockets.push_back(socket);
	}

	return true;
}

unsigned char dest_cnt = 0;

int Processer::doProc(Worker*& worker){
	string query = worker->query;
	
	//接收时间
	gettimeofday(&worker->tv_deal, NULL);
	//worker->cost_wait = (worker->tv_deal.tv_sec-worker->tv_recv.tv_sec)*1000000+(worker->tv_deal.tv_usec-worker->tv_recv.tv_usec);
	
	//开始处理
	ServerRst resp;
	int idx = (int)(dest_cnt%(_sockets.size()));
	dest_cnt++;
	_sockets[idx]->getRstFromHost(query,resp,0);
	if (resp.ret_len<0)
		worker->status = 2;
	else
		worker->status = 1;

	//计时
	gettimeofday(&worker->tv_done, NULL);
	worker->cost_deal = (worker->tv_done.tv_sec-worker->tv_deal.tv_sec)*1000000+(worker->tv_done.tv_usec-worker->tv_deal.tv_usec);
	fprintf(stderr,"ProcessTime:%d us\n",worker->cost_deal);
	return 0;
}

