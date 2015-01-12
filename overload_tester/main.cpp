#include "Sender.hpp"
#include "config.hpp"
#include <malloc.h>
#include <signal.h>
#include <iostream>
#include <sys/time.h>

using namespace std;

void sigterm_handler(int signo) {}
void sigint_handler(int signo) {}

void makeQuery(Worker* worker,const int& cnt,const std::vector<std::string>& test_case){
	int idx = cnt%test_case.size();
	worker->query = test_case[idx];
	worker->id = cnt;
	return;
}

int main(int argc, char* argv[])
{
	if (argc<2){
		printf("[command error]\n");
		printf("./overload test_case_filename\n");
		return 1;
	}
	mallopt(M_MMAP_THRESHOLD, 64*1024);

	close(STDIN_FILENO);
	signal(SIGPIPE, SIG_IGN);
	//signal(SIGTERM, &sigterm_handler);
	//signal(SIGINT, &sigint_handler);



	printf("[Server starting...]");
	{
		Config config;
		config.open("test.cfg");
		
		const char* filename = argv[1];
		ifstream fin;
		fin.open(filename);
		if (!fin){
			cerr<<"Can not open File "<<filename<<endl;
			exit(1);
		}
		
		std::vector<string> test_case;
		string line;
		while(!fin.eof()){
			getline(fin,line);
			if (line.length()==0 || line[0]=='#')
				continue;
			test_case.push_back(line);
		}
		fin.close();
		
		
		Sender sender;
		int ret;
		if((ret = sender.open(config.getInt("SENDER_THREAD_NUM"),
								config.getInt("SENDER_THREAD_SIZE"),
								config)) < 0)
		{
			printf("open processor error! ret:%d,thread:%d\n",ret,config.getInt("SENDER_THREAD_NUM"));
			exit(-1);
		}
		sender.activate();
		int mode = config.getInt("SEND_MODE");
		sender.setParams(mode,config.getInt("WAIT_USECOND"),config.getInt("QPS"));
		printf("Server initialized OK\n");
		printf("[Server started after 3 second]\n");
		printf("[Mode]:%d\n",mode);
		fprintf(stderr,"压力测试倒计时...3");
		sleep(1);
		fprintf(stderr,"...2");
		sleep(1);
		fprintf(stderr,"...1");
		sleep(1);
		fprintf(stderr,"...开始!\n");
		sleep(1);
		int sender_thread_num = config.getInt("SENDER_THREAD_NUM");
		int display_interval = config.getInt("DISPLAY_INTERVAL");
		int cnt = 0;
		struct timeval tv_start,tv_end;
		unsigned int total_time,done_num,fail_num,total_deal_time;
		unsigned int total_time_ms;
		gettimeofday(&tv_start, NULL);

		while(1){
			Worker* worker = new Worker;
			makeQuery(worker,cnt,test_case);
			sender.putJob(worker);
			cnt++;
			if (cnt%display_interval==0 && mode==0){
				gettimeofday(&tv_end, NULL);
				total_time = (tv_end.tv_sec-tv_start.tv_sec)*1000000+(tv_end.tv_usec-tv_start.tv_usec);
				if (total_time>1000){
					total_time_ms = total_time/1000;
					done_num = sender.done_num;
					fail_num = sender.fail_num;
					total_deal_time = sender.total_time_ms;
					fprintf(stderr,"QPS:%f 已处理:%d 平均时间:%dms 每个请求平均处理时间:%dms 失败超时占比%d%%\n",
							done_num*1000.0/total_time_ms,
							done_num,
							total_time/1000/done_num,
							total_deal_time/done_num,
							fail_num*100/done_num);
				}
			}
		}
		
		pause();
		sender.stop();
	}
	printf("[Server stop]\n");

	return 0;
}

