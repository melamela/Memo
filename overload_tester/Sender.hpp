#ifndef __SENDER_H__
#define __SENDER_H__

#include "task_base.hpp"
#include <string>
#include "config.hpp"
#include "wait_list.hpp"
#include "worker.hpp"
#include "Processer.hpp"



class Sender:public task_base{
public:
	Sender();
	~Sender();
	//初始化定制处理的函数
	int open(size_t thread_num, size_t stack_size,Config& config);
	int stop();	
	int svc();
	int putJob(Worker*& worker);
	void setParams(const int& mode,const int& waitUS,
					const int& qps);
protected:
	wait_list_t<Worker, &Worker::task_list_node>  m_task_list;
	Processer* m_processer;	//定制的处理对象
	Config* m_config;
	int m_mode;	//发送请求的mode 0是同步 1是异步
	/*同步异步相关参数*/
	int m_wait_time;	//同步时:线程占满时 等待时间 单位us  异步时:每个请求间的时间间隔

public:
	/*输出成员变量*/
	unsigned int done_num;	//已处理的请求数
	unsigned int fail_num;	//失败的请求数
};




#endif		//__SENDER_H__
