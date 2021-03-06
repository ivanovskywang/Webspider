#include "Webspider.h"

INT main(INT args, CHAR *argv[])
{
/*******************initialize**********************/
//The automachine
	Automachine match;

	Httpreadconf(match);
#ifdef DEBUG
	printf("################Automachine init done\n");
#endif

//The database
	Mydb Db;
	
	Db.Initdb();
#ifdef DEBUG
	printf("################Database init done\n");
#endif

//The bloom filter
	BloomFilter Bf(MAXSIZE);
#ifdef DEBUG
	printf("################Bloom Filter init done\n");
#endif

//The queue
	Queue *Urlqueue[THREAD_NUM];
	Queue *Visitedqueue[THREAD_NUM];
	for(INT i = 0; i < THREAD_NUM; i++)
	{
		Urlqueue[i] = new Queue(1024, 512);
		Visitedqueue[i] = new Queue(1024, 512);
	}

	Db.Urlenqueue(Urlqueue, Bf);
#ifdef DEBUG
	printf("################URL Queue init done\n");
#endif

//The regex
	Regex regex;
	regex.Construct();
#ifdef DEBUG
	printf("################Regex init done\n");
#endif
	
//The thread pool
	Manager manager(THREAD_NUM);
	manager.Init();
#ifdef DEBUG
	printf("################Thread Pool init done\n");
#endif
/*****************************************************/

//handle the visited queue
	pthread_t tid;
	INT ret = pthread_create(&tid, NULL, Visitedurlhandle, (VOID *)Visitedqueue);
	if(ret != 0)
	{
		printf("pthread create error!\n");
		return -1;
	}

//Begin to request use threadpool  
//can epoll used here? Let's see later
	pthread_t pid[THREAD_NUM];
	for(INT i = 0; i < THREAD_NUM; i++)
	{
		struct Webspider *webspider = (struct Webspider *)malloc(sizeof(struct Webspider));
		memset(webspider, 0, sizeof(struct Webspider));
		webspider->match = &match;
		webspider->Bf = &Bf;
		webspider->Urlqueue = Urlqueue[i];
		webspider->Visitedqueue = Visitedqueue[i];
		webspider->regex = &regex;
		manager.AddWorker(myprogress, webspider);
		//if(pthread_create(&pid[i], NULL, myprogress, (VOID*)webspider))
			//printf("Thread %d create fail!\n", i);
	}

/***********************destroy************************/
	manager.Join();
	pthread_join(tid, NULL);
	match.Machine_destroy();
	//for(INT i = 0; i < THREAD_NUM; i++)
		//pthread_join(pid[i], NULL);
	for(INT i = 0; i < THREAD_NUM; i++)
	{
		delete Urlqueue[i];
		delete Visitedqueue[i];
	}
	regex.Destroy();	
/*****************************************************/

	return 0;
}


