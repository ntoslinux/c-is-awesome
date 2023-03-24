#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARRAY_SIZE 100

int array[ARRAY_SIZE];
volatile int count;
int head;
int tail;

pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *producer(void *args)
{
        int res = 0;
        while (1) {
                pthread_mutex_lock(&mutex);
                if (count == ARRAY_SIZE) {
                        printf("\nNo space for new items waiting for consumer to consume");

                        // Spurious wakeups from the pthread_cond_timedwait() or
                        // pthread_cond_wait() functions may occur. Since the
                        // return from pthread_cond_timedwait() or
                        // pthread_cond_wait() does not imply anything about the
                        // value of this predicate, the predicate should be
                        // re-evaluated upon such return.

                        while (count == ARRAY_SIZE)
                                pthread_cond_wait(&empty, &mutex);
                }

                head %= ARRAY_SIZE;
                count++;
                array[head] = head;
                printf("\nproduced %d/%d", head, count);
                head++;
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&full);
        }

        return NULL;
}

void *consumer(void *args)
{
        int res = 0;
        while (1) {
                pthread_mutex_lock(&mutex);
                if (count == 0) {
                        printf("\nNo items available waiting for producer to produce");

                        // Spurious wakeups from the pthread_cond_timedwait() or
                        // pthread_cond_wait() functions may occur. Since the
                        // return from pthread_cond_timedwait() or
                        // pthread_cond_wait() does not imply anything about the
                        // value of this predicate, the predicate should be
                        // re-evaluated upon such return.

                        while (count == 0)
                                pthread_cond_wait(&full, &mutex);
                }

                tail %= ARRAY_SIZE;
                int ele = array[tail];
                count--;
                printf("\nconsumed %d/%d", tail, count);
                tail++;
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&empty);
        }

        return NULL;
}

int main()
{
        pthread_t producer_thread;
        pthread_t consumer_thread;
        int ret = 0;

        setbuf(stdout, NULL);

        ret = pthread_create(&producer_thread, NULL, producer, NULL);
        if (ret != 0) {
                printf("\nUnable to create producer thread %d", ret);
                goto exit;
        }

        ret = pthread_create(&consumer_thread, NULL, consumer, NULL);
        if (ret != 0) {
                printf("\nUnable to create consumer thread %d", ret);
                goto exit;
        }

        pthread_join(producer_thread, NULL);
        pthread_join(consumer_thread, NULL);

exit:
        return ret;
}