#ifndef  QO_WORKER_HPP
#define  QO_WORKER_HPP
#include "linked_list.hpp"
#include <string>

#define MAX_HTTP_CONTENT_LENGTH   1024*1024 

class Worker
{
	public:
		Worker()
		{
			status = 0;
			id = 0;
		}
		virtual ~Worker()
		{
		}

	public:
		std::string query;
		struct timeval tv_recv;
		struct timeval tv_deal;
		struct timeval tv_done;
		long cost_wait;
		long cost_deal;
		int status;	//0:未处理完 1：正常处理完毕 2：请求任务失败
		int id;
		linked_list_node_t task_list_node;
		
};

#endif //QO_WORKER_HPP

