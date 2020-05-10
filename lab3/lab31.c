#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

int main(int argc, char *argv[])
{

    int i, N,j;

    struct timeval T1, T2;

    long delta_ms;

    N = atoi(argv[1]); /* N等于第一个命令行参数N equals the first command-line parameter */

    //N = 10000;

    gettimeofday(&T1, NULL); /* 记住当前时间T1 remember the current time T1 */

    unsigned int seed = 0;

    for (i = 0; i < 50; i++) /* 50 experiments50 experiments */
    {
        double M1[N] ,M2[(int)(N/2)];
        srand(i); /* 初始化RNG的初始值 initialize the initial value of the RNG*/
        int A = 24;
        #pragma omp parallel for default(none) private(j,seed)shared(A,M1,N)
        for(j = 0; j < N; j++){/* 用N个数据填充初始数组 Fill the initial data array with size N */
            M1[j] = 1 + rand_r(&seed)%A;
        }
        #pragma omp parallel for default(none) private(j,seed)shared(A,M2,N)
        for(j = 0; j < N/2; j++){
            M2[j] = rand_r(&seed)%(A*9) + A;
        }
        /* 完成任务，用结果填充数组 Complete the task, fill in the array with the results */
        /* Map Stage. Apply Hyperbolic cotangent of number’s root to each element in the M1 array */
        #pragma omp parallel for default(none) private(j)shared(M1,N)
        for(j = 0; j < N; j++){
            M1[j] = sqrt(1.0/tanh(M1[j]));
        }
        /* in the M2 array, add each element in turn with the previous one ,and apply Sine modulus to the result of the addition*/
        double M2_temp[(int)(N/2)+1];
        M2_temp[0] = 0;
        #pragma omp parallel for default(none) private(j)shared(M2_temp,M2,N)
        for(j = 1; j < N/2+1; j++){
            M2_temp[j] = M2[j-1];
        }
        #pragma omp parallel for default(none) private(j)shared(M2_temp,M2,N)
        for(j = 0; j < N/2; j++){
            M2[j] = fabs(sin(M2_temp[j]+M2_temp[j+1]));
            
        }
        /* Merge Stage. In arrays M1 and M2 apply Raising to a power to all elements in pairs with the same indexes in */
        #pragma omp parallel for default(none) private(j)shared(M1,M2,N)
        for(j = 0; j < N/2; j++){
            M2[j] = pow(M1[j],M2[j]);
        }

        /* 通过指定的方法对数组和结果进行排序 Sort the array with the results by the specified method */
        /* Stupid sort */
        int x = 0;
        while (x < N/2) 
        {
            if (x == 0 || M2[x - 1] <= M2[x]) 
                x++;
            else 
            {
                double tmp = M2[x]; 
                M2[x] = M2[x - 1];
                M2[--x] = tmp;
            }
        }
        /* Reduce Stage. determining parity */
        double min = A*10;
        #pragma omp parallel for default(none) private(j)shared(min,M2,N)
        for(j = 0; j < N/2; j++){/* Find the minimum */
            #pragma omp critical 
            {
                if(min > M2[j] && M2[j]!=0)
                    min = M2[j];
            }
            
        }
        double sum = 0;
        #pragma omp parallel for default(none) private(j)shared(M2,N,min)reduction(+:sum)
        for(j = 0; j < N/2; j++){
            int tmp = (int)(M2[j]/min);
            if(tmp%2==0)
                sum = sum + sin(M2[j]);  
        }
        printf("i->%d,X->%f\n",i,sum);
    }
    

    gettimeofday(&T2, NULL);

    /* remember the current time T2 */

    delta_ms = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;

    printf("\nN=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 -T1 */
    return 0;
}