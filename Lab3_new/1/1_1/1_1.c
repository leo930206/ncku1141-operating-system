#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

volatile int a = 0;
pthread_spinlock_t lock;

void *thread(void *arg) {  // race condition: 多個 Threads 同時存取並修改同一個變數，進而產生不正確或不可預測的計算結果 。
    for (int i=0; i<10000; i++) {
        /*YOUR CODE HERE*/
        pthread_spin_lock(&lock);    // 上鎖
        a = a + 1;
        pthread_spin_unlock(&lock);  // 解鎖
    }
    return NULL;
}

int main() {
    FILE *fptr;
    fptr = fopen("1.txt", "a");
    pthread_t t1, t2;

    pthread_spin_init(&lock, 0);
    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_spin_destroy(&lock);

    fprintf(fptr, "%d ", a);
    fclose(fptr);
}
