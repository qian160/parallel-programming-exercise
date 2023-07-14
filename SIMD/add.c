// lscpu | grep avx
// gcc -mavx2
// data must be aligned, or these intrincs may cause a seg fault
#include<immintrin.h>
#include<stdio.h>
#include<assert.h>

void asm_test(){
        double a[4] __attribute__((aligned(32)))={1,2,3,4} ,
                b[4] __attribute__((aligned(32))) ={5,6,7,8},
                c[4] __attribute__((aligned(32))) ={0} ;
                __asm__ volatile(
                        "vmovapd (%1),%%ymm0\n"
                        "vmovapd (%2),%%ymm1\n"
                        "vaddpd  %%ymm0,%%ymm1,%%ymm2\n"
                        "vmovapd %%ymm2,%0\n"
                        :"=m"(c)
                        :"r"(a),"r"(b)
                );
                printf("%.0f\t%.0f\t%.0f\t%.0f\t\n",c[0],c[1],c[2],c[3]);

}

/* not supported on my computer
int array_sum_int_avx512(int *array, int n){
	assert(n % 16 == 0);
	__m512i vec = _mm512_load_si512(array);
	for(int i = 16; i < n; i += 16)
	{
		__m512i data = _mm512_load_si512(array + i);
		vec = _mm512_add_epi32(vec, data);
	}

	return _mm512_reduce_add_epi32(vec);	// more convenient, but only supported by avx512, not avx2
}
*/
int array_sum_int_avx2(int *array, int n){
	assert((n & 0b111) == 0);
	int sum = 0;
	__m256i vec = _mm256_load_si256((__m256i*)array);
	for(int i = 8; i < n; i += 8)
	{
		__m256i data = _mm256_load_si256((__m256i*)(array + i));
		vec = _mm256_add_epi32(vec, data);
	}
	int sum_array[8] __attribute__((aligned(32)));
	_mm256_store_si256((__m256i*)sum_array, vec);
	for(int i = 0; i < 8; i++)
		sum += sum_array[i];
	return sum;
}

int main()
{
	int a[1024] __attribute__((aligned(32)));
	for(int i = 0; i < 1024; i++)
		a[i] = 1;
	asm_test();
	printf("sum of a = %d\n", array_sum_int_avx2(a, 1024));
}
