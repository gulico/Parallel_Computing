#include <stdio.h>
#include "/Users/wxy/desktop/并行计算/ExampleDir/FW_1.3.1_Mac64/fwBase.h"
#include "/Users/wxy/desktop/并行计算/ExampleDir/FW_1.3.1_Mac64/fwSignal.h"
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{

    int i,j;

    struct timeval T1, T2;

    long delta_ms;

    size_t N;
    if(argc > 1)
        N = (size_t) atoi(argv[1]);
    else
        N = 20;
    if(argc > 2) {
        fwSetNumThreads(atoi(argv[2]));
    } else {
        fwSetNumThreads(4);
    }

    gettimeofday(&T1, NULL); /* 记住当前时间T1 remember the current time T1 */

    unsigned int seed = 0;

    for (i = 0; i < 10; ++i) /* 50 experiments50 experiments */
    {
        double M1[N] ,M2[(int)(N/2)];
        srand(i); /* 初始化RNG的初始值 initialize the initial value of the RNG*/
        int A = 24;
        for(j = 0; j < N; j++){/* 用N个数据填充初始数组 Fill the initial data array with size N */
            M1[j] = 1 + rand_r(&seed) % A;
        }
        for(j = 0; j < N/2; j++){
            M2[j] = rand_r(&seed)%(A*9) + A;
        }
        /* Complete the task, fill in the array with the results */
        /* Map Stage. Apply Hyperbolic cotangent of number’s root to each element in the M1 array */
        fwsTanh_64f_A50	( M1, M1, N);
        fwsPowx_64f_A50	( M1, -1, M1, N);

        /* in the M2 array, add each element in turn with the previous one ,and apply Sine modulus to the result of the addition*/
        double M2_temp[(int)(N/2)+1];
        fwsCopy_64f(M2,M2_temp,N/2);
        fwsAdd_64f(&M2[1], M2_temp, &M2[1], N/2-1 );
		fwsSin_64f_A50(M2,M2,N/2);
        fwsAbs_64f(M2,M2,N/2);

        /* Merge Stage. In arrays M1 and M2 apply Raising to a power to all elements in pairs with the same indexes in */
        fwsPow_64f_A50(M1, M2, M2, N/2);

        /* Sort the array with the results by the specified method */
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
        for(j = 0; j < N/2; j++){/* Find the minimum */
            if(min > M2[j] && M2[j]!=0)
                min = M2[j];
        }
        double sum = 0;
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