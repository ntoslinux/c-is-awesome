#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SEGMENT_SIZE 10
#define NUM_THREADS 10
int *array;
int par_sum;
pthread_mutex_t add_mutex = PTHREAD_MUTEX_INITIALIZER;

struct thread_ctx {
        pthread_t thread;
        int index;
};

void* segment_sum(void *args)
{
        int sum = 0;
        struct thread_ctx *thread = (struct thread_ctx *)args;
        int index = thread->index;

        for (int i = index * SEGMENT_SIZE; i < (index + 1) * SEGMENT_SIZE; i++) {
                sum += array[i];
        }

        pthread_mutex_lock(&add_mutex);
        par_sum += sum;
        pthread_mutex_unlock(&add_mutex);

        // this can also be replaced with return (void*)0;
        pthread_exit(0);

        // We cannot return the sum here. The thread start function(aka this
        // function) can only return the status of the function interms of
        // success(zero) or failure(non zero) but not the actual result of the
        // computation
}

int main()
{
        struct thread_ctx *threads;
        int res = 0;
        int seq_sum = 0;

        array = calloc(NUM_THREADS * SEGMENT_SIZE, sizeof(int));
        threads = calloc(NUM_THREADS, sizeof(struct thread_ctx));

        for (int i = 0; i < NUM_THREADS * SEGMENT_SIZE; i++) {
                array[i] = 1;
                seq_sum += 1;
        }

        // we are making thread index as long instead of int to make sure
        // casting from void * to long work without compiler warnings. using int
        // instead will cause compiler warnings and lead to runtime failures on
        // systems where sizeof(int) != sizeof(void*)
        for (long i = 0; i < NUM_THREADS; i++) {
                threads[i].index = i;
                res = pthread_create(&threads[i].thread, NULL, segment_sum, &threads[i]);
                if (res != 0) {
                        printf("\nError creating new thread");
                        goto exit;
                }
        }

        for (int i = 0; i < NUM_THREADS; i++) {
                void *thread_status;
                res = pthread_join(threads[i].thread, &thread_status);
                if (res != 0) {
                        printf("\nError in join");
                        goto exit;
                }
        }

        printf("\nmultithreaded sum: %d single threaded sum: %d\n", par_sum, seq_sum);

exit:
        free(array);
        free(threads);
        return 0;
}