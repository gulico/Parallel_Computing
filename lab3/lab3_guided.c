#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#ifdef _OPENMP
#include "omp.h"
#else
	double omp_get_wtime() 
	{
		struct timeval T;
		gettimeofday(&T, NULL);
		double delta = 1000 * T.tv_sec + T.tv_usec / 1000;
		return delta / 1000;
	}
#endif

void Stupid_sort(double array[], int start,int length);

int main(int argc, char *argv[])
{

    int i, N,j;
    int M,thread;
    double T0,T1,T2 = -1.0;
    long delta_ms;

    thread=4;
    N = atoi(argv[1]); /* N等于第一个命令行参数N equals the first command-line parameter */
    M = atoi(argv[2]);
    omp_set_nested(1);
    if(argc > 2){
        omp_set_num_threads(M);
    }
    else{
        omp_set_num_threads(thread);
    }

    T1 = omp_get_wtime();
    
    #pragma omp parallel for num_threads(thread)
    {
        if(omp_get_thread_num()==0)
		{	
			T0 = T1;
			while(i<=50)
			{
				if( omp_get_wtime( ) - T0>0.01) 
				{
					printf("%lf%%\n", i/50.0*100);
					T0=omp_get_wtime();
				}
			}
		}
		else 
		{
            for (i = 0; i < 50; i++) /* 50 experiments50 experiments */
            {
                unsigned int seed = 0;
                double M1[N] ,M2[(int)(N/2)];
                srand(i);
                int A = 24;
        #pragma omp parallel for private(j) shared(M1)  schedule(guided,M)

                for(j = 0; j < N; j++){
                    M1[j] = rand_r(&seed)%A;
                    while(M1[j]==0.0){
                        M1[j] = rand_r(&seed)%A;
                    }
                }
        #pragma omp parallel for private(j) shared(M2) schedule(guided,M)
                for(j = 0; j < N/2; j++){
                    M2[j] = rand_r(&seed)%(A*10);
                }
                /* Map Stage. */
        #pragma omp parallel for private(j) shared(M1) schedule(guided,M)
                for(j = 0; j < N; j++){
                    M1[j] = sqrt(1.0/tanh(M1[j]));
                }
                /* in the M2 array, add each element in turn with the previous one ,and apply Sine modulus to the result of the addition*/
                double M2_temp[(int)(N/2)+1];
                M2_temp[0] = 0;
        #pragma omp parallel for private(j) shared(M2_temp,M2)  schedule(guided,M)
                for(j = 1; j < N/2+1; j++){
                    M2_temp[j] = M2[j-1];
                }
        #pragma omp parallel for private(j) shared(M2) schedule(guided,M)
                for(j = 0; j < N/2; j++){
                    M2[j] = fabs(sin(M2_temp[j]+M2_temp[j+1]));
                    
                }
                /* Merge Stage. In arrays M1 and M2 apply Raising to a power to all elements in pairs with the same indexes in */
        #pragma omp parallel for private(j) shared(M2,M1) schedule(guided,M)
                for(j = 0; j < N/2; j++){
                    //#pragma omp atomic
                    M2[j] = pow(M1[j],M2[j]);
                }

                /* Stupid sort */
                #pragma omp parallel sections
				{
					#pragma omp section
					{
						Stupid_sort(M2, 0, N/4);
					}
					#pragma omp section
					{
						Stupid_sort(M2, N/4, N/2);
					}
				}
                Stupid_sort(M2, 0, N/2);

                /* Reduce Stage. determining parity */
                double min = A*10;
        #pragma omp parallel for private(j) shared(M2,min) schedule(guided,M)
                for(j = 0; j < N/2; j++){/* Find the minimum */
                    if(min > M2[j] && M2[j]!=0)
        #pragma omp critical
                        min = M2[j];
                }
                double sum = 0;
                #pragma omp parallel for private(j) reduction(+:sum)  shared(M2) schedule(guided,M)
                for(j = 0; j < N/2; j++){
                    int tmp = (int)(M2[j]/min);
                    if(tmp%2==0.0)
                        sum = sum + sin(M2[j]);  
                }
            }
        }
    }    
    printf("\n");
    //printf("the X: %lf\n",X);
	T2=omp_get_wtime();
	delta_ms = 1000*(T2 - T1) + (T2 - T1)/1000;
	printf("N=%d. Milliseconds passed: %ld\n", N, delta_ms); /* T2 -T1 */
    
    return 0;
}


void Stupid_sort(double array[], int start,int end){
    int x = start;
    while (start - 1 < x < end) 
    {
        if (x == start || array[x - 1] <= array[x]) 
            x++;
        else 
        {
            double tmp = array[x]; 
            array[x] = array[x - 1];
            array[--x] = tmp;
        }
    }
}
