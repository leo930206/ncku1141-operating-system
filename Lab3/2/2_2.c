#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define matrix_row_x 1234
#define matrix_col_x 250

#define matrix_row_y 250
#define matrix_col_y 4

pthread_spinlock_t lock;
FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
int **x;
int **y;
int **z;

// Put file data intp x array
void data_processing(void){
    int tmp;
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_x; j++){
            if (fscanf(fptr1, "%d", &x[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }

    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
     for(int i=0; i<matrix_row_y; i++){
        for(int j=0; j<matrix_col_y; j++){
            if (fscanf(fptr2, "%d", &y[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }   
}

// 保護 z[i][j] 這個共享變數，避免發生 Race Condition 。

void *thread1(void *arg){     // 計算 A row 與 B column 的前半段乘積，並加進 z[i][j] 。
    for(int i=0; i<matrix_row_x; i++){        // 遍歷 Matrix A 的所有 row
        for(int j=0; j<matrix_col_y; j++){    // 遍歷 Matrix B 的所有 column
            for(int k=0; k<matrix_row_y/2; k++){
                /*YOUR CODE HERE*/
                pthread_spin_lock(&lock);
                z[i][j] += x[i][k] * y[k][j];
                pthread_spin_unlock(&lock);
            }      
        }
    }
    return NULL;
}

void *thread2(void *arg) {     // 計算 A row 與 B column 的後半段乘積，並加進 z[i][j] 。
    for(int i=0; i<matrix_row_x; i++){        // 遍歷 Matrix A 的所有 row
        for(int j=0; j<matrix_col_y; j++){    // 遍歷 Matrix B 的所有 column
            for(int k=matrix_row_y/2; k<matrix_row_y; k++){
                /*YOUR CODE HERE*/
                pthread_spin_lock(&lock);
                z[i][j] += x[i][k] * y[k][j];
                pthread_spin_unlock(&lock);
            }     
        }
    } 
    return NULL;
}



int main() {
    x = malloc(sizeof(int *) * matrix_row_x);
    for (int i = 0; i < matrix_row_x; i++) {
        x[i] = malloc(sizeof(int) * matrix_col_x);
    }
    y = malloc(sizeof(int *) * matrix_row_y);
    for (int i = 0; i < matrix_row_y; i++) {
        y[i] = malloc(sizeof(int) * matrix_col_y);
    }
    z = malloc(sizeof(int *) * matrix_row_x);
    for (int i = 0; i < matrix_row_x; i++) {
        z[i] = malloc(sizeof(int) * matrix_col_y);
    }
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("2.txt", "a");
    pthread_t t1, t2;
    data_processing();
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);

    pthread_spin_init(&lock, 0);
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_spin_destroy(&lock);

    // Write output matrix into file.
    for (int i = 0; i < matrix_row_x; i++) {
        for (int j = 0; j < matrix_col_y; j++) {
            fprintf(fptr3, "%d ", z[i][j]);
            if (j == matrix_col_y - 1) fprintf(fptr3, "\n");
        }
    }
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
}
