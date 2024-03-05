#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

int termination_flag = 0;

void* producer(void *args){
    int item = 0; 
    while(!termination_flag){
        sleep(rand()%3);
        item = rand()%100; 
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        buffer[in]  = item;
        in = (in+1)%BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&full);

    }
    return NULL;
}

void* consumer(void *args){
    int item;
    while(!termination_flag){
        sleep(rand()%3);
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        item = buffer[out];
        out = (out+1)%BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty);

        printf("Consumed: %d\n", item);
    }
    return NULL;
}

void handle_sigint(int sig){
    termination_flag = 1;
}

int main(int argc , char *argv[]){
    if(argc < 4){
        printf("Usage: %s <producer_count> <consumer_count> <termination_time>\n", argv[0]);
        return 1;
    }
    int producer_threads_count = atoi(argv[1]);
    int consumer_threads_count = atoi(argv[2]);
    int termination_time = atoi(argv[3]);

    pthread_t producer_threads[producer_threads_count], consumer_thread[consumer_threads_count];

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full,0,0);
    pthread_mutex_init(&mutex,NULL);

    signal(SIGINT, handle_sigint);

    for (int i = 0; i < producer_threads_count; i++)
    {
        pthread_create(&producer_threads[i],NULL,producer,NULL);
    }
    
    for (int i = 0; i < consumer_threads_count; i++)
    {
        pthread_create(&consumer_thread[i],NULL,consumer,NULL);
    }
    
    sleep(termination_time);
    termination_flag = 1;

    for (int i = 0; i < producer_threads_count; i++)
    {
        pthread_join(producer_threads[i],NULL);
    }
    
    for (int i = 0; i < consumer_threads_count; i++)
    {
        pthread_join(consumer_thread[i],NULL);
    }
    

    sem_destroy(&empty);
    sem_destroy(&full);

    pthread_mutex_destroy(&mutex);

    return 0;
}
