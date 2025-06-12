#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// 全局变量
float *a, *b;
double result = 0.0;
int thread_num;
long long N;
pthread_mutex_t lock;

typedef struct {
    long long start;
    long long end;
    double partial_sum;
} ThreadData;

void *dot_product(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double sum = 0.0;
    for (long long i = data->start; i < data->end; ++i) {
        sum += (double)a[i] * b[i];
    }

    // 线程安全地更新全局结果
    pthread_mutex_lock(&lock);
    result += sum;
    pthread_mutex_unlock(&lock);

    data->partial_sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s thread_num N\n", argv[0]);
        return 1;
    }

    thread_num = atoi(argv[1]);
    N = atoll(argv[2]);

    if (thread_num < 1 || thread_num > 16 || N < 100000) {
        fprintf(stderr, "Error: thread_num ∈ [1,16], N ≥ 100000\n");
        return 1;
    }

    // 初始化向量
    a = (float *)malloc(sizeof(float) * N);
    b = (float *)malloc(sizeof(float) * N);
    for (long long i = 0; i < N; ++i) {
        if (i % 3 == 0) {
            a[i] = 1.0;
            b[i] = 1.0;
        } else if (i % 3 == 1) {
            a[i] = 1.0;
            b[i] = -1.0;
        } else {
            a[i] = 0.0;
            b[i] = 0.0;
        }
    }

    pthread_t threads[16];
    ThreadData tdata[16];

    long long chunk_size = N / thread_num;

    pthread_mutex_init(&lock, NULL);
    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    // 创建线程
    for (int i = 0; i < thread_num; ++i) {
        tdata[i].start = i * chunk_size;
        tdata[i].end = (i == thread_num - 1) ? N : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, dot_product, &tdata[i]);
    }

    // 等待线程结束
    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end_time, NULL);
    long long elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000LL +
                           (end_time.tv_usec - start_time.tv_usec) / 1000LL;

    printf("s=%.0f t=%lld(ms)\n", result, elapsed_ms);

    // 释放资源
    free(a);
    free(b);
    pthread_mutex_destroy(&lock);
    return 0;
}
