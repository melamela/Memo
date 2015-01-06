#include "Sender.hpp"
#include <sys/time.h>

using namespace std;


Sender::Sender(){
	m_processer = NULL;
	m_mode = 1;
	done_num = 0;
	fail_num = 0;
}
Sender::~Sender(){
	if (m_processer)
		delete m_processer;
	m_processer = NULL;
}

int Sender::putJob(Worker*& worker){
	if (m_mode==0){
		//同步模式,队列超过线程数目则阻塞队列入队
		while (m_task_list.len() >= m_thread_num){
			usleep(m_wait_time);
		}
	}else if (m_mode==1){
		while(m_task_list.len() >= 102400){
			cerr<<"[WARNING]:Send queue's Size > 102400, Wait 1s"<<endl;
			sleep(1);
		}
		if (m_wait_time>0){
			usleep(m_wait_time);
		}
	}

	gettimeofday(&worker->tv_recv,NULL);
	m_task_list.put(*worker);
	return 0;
}

void Sender::setParams(const int& mode,const int& waitUS,
						const int& qps){
	m_mode = mode;
	if (m_mode==0){
		m_wait_time = waitUS;
	}else if (m_mode==1){
		m_wait_time = 1000000/qps;
	}
	return;
}

int Sender::open(size_t thread_num, size_t stack_size,Config& config)
{
	if (NULL == m_processer)
		m_processer = new Processer;

	if (NULL == m_processer)
		return false;
	
	if (!m_processer->init(config))
	{
		cerr<<"[Error]:Processer->Init()!!"<<endl;
		return -1;
	}
	m_config = &config;
	cerr<<"thread num="<<thread_num<<" stack_size="<<stack_size<<std::endl;
	return task_base::open(thread_num, stack_size);
}

int Sender::stop()
{
	m_task_list.flush();
	join();
	cerr<<"process stop"<<std::endl;
	return 0;
}

int thread_num=0;

int Sender::svc()
{
	Worker * worker;
	int queueLen;
	int thread_id = ++thread_num;
	printf("Sender Thread<%d> started!",thread_id);
	
	Processer* processer = new Processer;
	if (!processer->init(*m_config))
	{
		cerr<<"[Error]:Processer->Init()!!"<<endl;
		return -1;
	}



	while ((worker = m_task_list.get()) != NULL)
	{
		queueLen = m_task_list.len();
		
		//m_processer->doProc(worker);
		processer->doProc(worker);
		
		//最后删除worker
		done_num++;
		if (worker->status==2)
			fail_num++;
		delete worker;
		worker=NULL;

	}//while

	delete processer;
	processer = NULL;

	printf("Sender Thread<%d> stoped!",thread_id);

	return 0;
}

