#include "SocketClient.h"
#include "CommonFuc.h"
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;
using namespace COMMON;

#define SEG_BUF 50


bool SocketClient::init(const std::string& addr,const long& timeout,
                        const int& buff_size,const int& write_resp){
	int pos = addr.find(":");
	if (pos==string::npos)
		return false;
	m_ip = addr.substr(0,pos);
	m_port = Transfer<string,int>::convert(addr.substr(pos+1));
	m_timeout = timeout;
    m_buff_size = buff_size;
    m_write_resp = write_resp;
    m_rcvbuff = new char[m_buff_size];
    return true;
}

int SocketClient::read_timeout(int fd, char* buf ,int len, timeval *timeout) 
{
    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(fd, &wset);
    if (select(fd + 1, &wset, NULL, NULL, timeout) <= 0) {
        return -1;
    }
    return read(fd, buf, len);
}


int SocketClient::readn_timeout(int fd, char* content, int need_to_read, timeval *timeout)
{
    char buf[SEG_BUF + 1];
    int n, left;
    int len;
    int ptr = 0;
    left = need_to_read;
    while (left > 0) {
        len = left > SEG_BUF ? SEG_BUF : left;
        n = read_timeout(fd, buf, len, timeout);
        if (n <= 0 ) {
            return need_to_read - left;
        }
        buf[n] = '\0';
        memcpy(content + ptr, buf, len);
        ptr = ptr + n;
        left = left - n;
    }
    return 0;
}

int SocketClient::read_http_header_timeout(int fd, string& header, char* content, timeval *timeout) 
{
    char buf[SEG_BUF + 1];
    int n, pos;
    string pkg="";
    while (1) {
        pos = pkg.find("\r\n\r\n");
        if (pos != string::npos) {
            header = pkg.substr(0, pos);
            memcpy(content, buf + (pos + 4) % SEG_BUF, n - (pos + 4) % SEG_BUF);
            return n - (pos + 4) % SEG_BUF;
        }
        n = read_timeout(fd, buf, SEG_BUF, timeout);
        if (n <= 0) {
            return -1;
        }
        buf[n] = '\0';
        pkg += buf;
    }
    return -1;
}


int SocketClient::recv_result(char* result, int& len, int fd, long time_out_us)
{
    int ret;
    len = 0;
    struct timeval timeout = {0, time_out_us};
    string header;
    ret = read_http_header_timeout(fd, header, result, &timeout);
    if (ret < 0) {
        return -1;
    }

    int idx = header.find("Content-Length:");
    if (idx == string::npos) {
        return -2;
    }
    int sep_index = header.find("\r\n", idx);
    int body_len = atoi(header.substr(idx + strlen("Content-Length:"), sep_index - idx - strlen("Content-Length:")).c_str());
    int need_to_read = body_len - ret;
    ret = readn_timeout(fd, result + ret, need_to_read, &timeout);
    if (ret != 0) {
        return -3;
    }
    len = body_len;
    return 0;

}

bool SocketClient::string_split(const string& str, const string& pattern, vector<string>& v)
{
    v.clear();
    size_t bpos = 0;
    
    while(str.find(pattern, bpos) != std::string::npos)
    {
        size_t epos = str.find(pattern, bpos);
        if(epos == 0)
        {
            bpos = epos + pattern.size();
            continue;
        }
        v.push_back(str.substr(bpos, epos - bpos));
        bpos = epos + pattern.size();
        if(bpos >= str.size())
            break;
    }
    
    if(bpos < str.size())
        v.push_back(str.substr(bpos, str.size() - bpos));
    return true;
}
bool SocketClient::getRstFromHost(const std::string& query,ServerRst& sr,unsigned short type)
{
    int sock;
    struct sockaddr_in server;
    struct hostent *hp;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        return  false;
    }
    server.sin_family = AF_INET;
    
    //DNS缓存 CACHE查询
	if(m_dns_cache!=0){
		server.sin_addr.s_addr = m_dns_cache;
	}else{
        struct hostent *pHost,host;
        memset(&host,0, sizeof(hostent));
        int tmp_errno=0;
        char host_buff[2048];
        memset(host_buff,0, sizeof(host_buff));
		if (gethostbyname_r(m_ip.c_str(),
					&host, host_buff, sizeof(host_buff),
					&pHost, &tmp_errno)) {
			printf("get host Fail!\n");
			return false;
		} else {
			if (pHost && AF_INET == pHost->h_addrtype) {
				memcpy(&server.sin_addr, pHost->h_addr_list[0], sizeof(server.sin_addr));
				//Cache录入
				m_dns_cache = server.sin_addr.s_addr;
                printf("DNS Cached\n");
			} else {
				printf("host create Fail!\n");
				return false;
			}
		}
	}
    
    
    server.sin_port = htons(m_port);
    int ret;

    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);//设置为非阻塞模式
	fd_set fdset;
	bool bSucc = false;
	struct timeval tm = {0, m_timeout};
    int nErr = -1;

    if ((ret = connect(sock, (struct sockaddr*)&server, sizeof(server))) < 0) {
        //设置连接超时
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        if (select(sock+1, NULL, &fdset, NULL, &tm) > 0)
			bSucc = true;
	    else{
			bSucc = false;
		}
    }
	else{
		bSucc = true;
	}

     ul = 0;
     ioctl(sock, FIONBIO, &ul);//设置为阻塞模式
     if (!bSucc)
     {
               close(sock);
               fprintf(stderr,"ERROR:connecting stream socket\n");
               fprintf(stderr,"err:%d(%s),ret:%d\n",errno,strerror(errno),ret);
               fprintf(stderr,"EINPROGRESS:%d, EALREADY:%d\n",EINPROGRESS,EALREADY);
               return false;
     }

    ostringstream oss_qlen;
    oss_qlen << query.size();

	string use_host = "default";
	string sendmsg_str;
	if (type==0){
        sendmsg_str = (string)"GET /" + query + " HTTP/1.1\r\n"+
				(string)"Accept: */*\r\n" +
    	        (string)"Accept-Language: zh-cn\r\n" +
				(string)"Host: " + use_host + "\r\n"+
				(string)"Content-Length: "+ oss_qlen.str() + (string)" \r\n"+ 
				(string)"Content-Type: application/x-www-form-urlencoded;charset=utf-8\r\n\r\n";
	}else if (type==2){
        sendmsg_str = (string)"GET /" + query + " HTTP/1.1\r\n"+
				(string)"Accept: */*\r\n" +
    	        (string)"Accept-Language: zh-cn\r\n" +
				(string)"Host: " + use_host + "\r\n"+
				(string)"Content-Type: application/x-www-form-urlencoded;charset=gbk\r\n\r\n";
	}
	else if (type==1){
        sendmsg_str = (string)"POST /" + "servermanager" + " HTTP/1.1\r\n"+
            (string)"Accept: */*\r\n" +
            (string)"Accept-Language: zh-cn\r\n" +
            (string)"Host: " + use_host + "\r\n"+
            (string)"Content-Type: application/x-www-form-urlencoded;charset=gbk\r\n"+
        	(string)"Content-Length: "+ oss_qlen.str() + (string)" \r\n\r\n"+ query;
	}
  
    if (send(sock, sendmsg_str.c_str(), sendmsg_str.size(), 0) < 0)
    {
        perror("Socket Sending");
        //fprintf(stderr,"err:%d(%s)\n",errno,strerror(errno));
        close(sock);
        return false;
    }

    memset(m_rcvbuff, 0, m_buff_size);
    int buf_len=m_buff_size;
    int rt = recv_result(m_rcvbuff, buf_len, sock,m_timeout);
    if(rt != 0)
    {
    	printf("Return Error:%d\n",rt);
        fprintf(stderr, "Socket Timeout or Error\n");
        close(sock);
        return false;
    }
 
    sr.ret_len = buf_len;
    if (m_write_resp)
        sr.ret_str = m_rcvbuff;

    close(sock);
    return true;
}


