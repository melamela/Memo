#ifndef __SOCKET_CLIENT_H_
#define __SOCKET_CLIENT_H_


#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctype.h> 
#include <sstream>


class ServerInfo{
	public:
	std::string ip;
	int port;
};

class ServerRst
{
	public:
		int ret_len;
		std::string ret_str;

	public:
		ServerRst()
		{
			ret_len = -1;
			ret_str = "";
		};
		~ServerRst(){};
};

class SocketClient
{
	public:
		SocketClient(){m_dns_cache=0;m_rcvbuff=NULL;};
		~SocketClient(){
			if (m_rcvbuff) {
				delete[] m_rcvbuff;
				m_rcvbuff = NULL;
			}
		};
		bool init(const std::string& addr,const long& timeout,
					const int& buff_size,const int& write_resp);
		
	private:
		int read_timeout(int fd, char* buf ,int len, timeval *timeout);
		int readn_timeout(int fd, char* content, int need_to_read, timeval *timeout);
		int read_http_header_timeout(int fd, std::string& header, char* content, timeval *timeout);
		int recv_result(char* result, int& len, int fd,long time_out_us);
		bool string_split(const std::string& str, const std::string& pattern, std::vector<std::string>& v);

	public:
		std::string m_ip;
		int m_port;
		long m_timeout;
		unsigned int m_dns_cache;
		int m_buff_size;
		int m_write_resp;
		char* m_rcvbuff;

		bool getRstFromHost(const std::string& query,ServerRst& sr,unsigned short type=0);
};

#endif	//__SOCKET_CLIENT_H_
