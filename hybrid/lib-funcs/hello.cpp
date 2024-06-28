#include <cstdio>
#include <omp.h>
int main()
{
    printf("Hello world!\n");
#pragma omp parallel
    {
        int id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        printf("Hello from thread %d/%d\n", id+1, num_threads);
    }
    return 0;
}
