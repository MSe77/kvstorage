#include <iostream>
#include "skiplist.h"
#include<string>
#include<pthread.h>
#include<chrono>
using std::string;

#define THREAD_COUNT 1
#define TEST_COUNT 100000

Skiplist<int,string> skp(18);
void* insert(void* args)
{
   int tmp=TEST_COUNT/THREAD_COUNT;

   for(int count=0;count<tmp;++count)
   {
    skp.insert_element(rand()%TEST_COUNT,"a");
   }
   pthread_exit(NULL);
}

void* get(void* args)
{
   int tmp=TEST_COUNT/THREAD_COUNT;

    for(int count=0;count<tmp;++count)
   {
    skp.search_element(rand()%TEST_COUNT);
   }
   pthread_exit(NULL);
}

int main(int, char**) {
    // Skiplist<int,string> skl=Skiplist<int,string>(6);

    // skl.insert_element(1,"a");
    // skl.insert_element(2,"b");
    // skl.insert_element(3,"c");

    // skl.display_list();
    // skl.dump_file();
    // skl.load_file();

    // skl.display_list();

    skp.load_file();
    srand(time(NULL)); //使用当前时间作为随机数种子；
    pthread_t threads[THREAD_COUNT];
    int rc;
    auto start=std::chrono::high_resolution_clock::now(); //当前高精度时间
    
    for(int i=0;i<THREAD_COUNT;++i)
    {
        rc=pthread_create(&threads[i],NULL,get,NULL);

        if(rc)
        {
        std::cout<<"创建线程失败"<<std::endl;
        exit(-1);
        }
    }
    void* ret;

    for(int i=0;i<THREAD_COUNT;++i)
    {
        if(pthread_join(threads[i],&ret)!=0)//等待线程结束
        {
             exit(3);
        }  
    }
    auto finish=std::chrono::high_resolution_clock::now(); 
    
    std::chrono::duration<double> elapsed=finish-start;
    std::cout<<"insert elapsed:"<<elapsed.count()<<std::endl;
    std::cout<<"skp size:"<<skp.size()<<std::endl;
    std::cout<<"skp level:"<<skp.level()<<std::endl;

    // skp.dump_file();


    return 0;


}
