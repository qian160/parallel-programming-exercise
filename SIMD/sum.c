// lscpu | grep avx
// gcc -mavx2
// https://www.linuxjournal.com/content/introduction-gcc-compiler-intrinsics-vector-processing
// https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html?wapkw=intel%20intrinsics%20guide

#include<immintrin.h>
#include<stdio.h>
#include<assert.h>
#include<stdalign.h>

void asm_test(){
	alignas(32) double a[4] = {1,2,3,4}, b[4] = {5,6,7,8}, c[4] = {0} ;
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
// aligned tends to be faster
int sum_int_avx2_aligned(int *array, int n){
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

float sum_float_avx2_aligned(float *array, int n){
	assert((n & 0b111) == 0);
	float sum = 0.0f;
	__m256 vec = _mm256_load_ps(array);
	for(int i = 8; i < n; i += 8)
	{
		__m256 data = _mm256_load_ps((array + i));
		vec = _mm256_add_ps(vec, data);
	}
	float sum_array[8] __attribute__((aligned(32)));
	_mm256_store_ps(sum_array, vec);
	for(int i = 0; i < 8; i++)
		sum += sum_array[i];
	return sum;
}

double sum_double_avx2_aligned(double *array, int n){
	__m256d vec = _mm256_load_pd(array);
	for(int i = 4; i < n; i += 4)
	{
		__m256d data = _mm256_loadu_pd(array + i);
		vec = _mm256_add_pd(vec, data);
	}
	alignas(32) double sum_array[4], sum = 0;
	_mm256_store_pd(sum_array, vec);
	for(int i = 0; i < 4; i++)
		sum += sum_array[i];
	return sum;
}

int sum_int_avx2_unaligned(int *array, int n){
	assert((n & 0b111) == 0);
	int sum = 0;
	__m256i vec = _mm256_loadu_si256((__m256i*)array);
	for(int i = 8; i < n; i += 8)
	{
		__m256i data = _mm256_loadu_si256((__m256i*)(array + i));
		vec = _mm256_add_epi32(vec, data);
	}
	int sum_array[8];
	_mm256_storeu_si256((__m256i*)sum_array, vec);
	for(int i = 0; i < 8; i++)
		sum += sum_array[i];
	return sum;
}

#define SUM_ALIGNED(arr, n)							\
	_Generic((arr),									\
		int*:	sum_int_avx2_aligned(arr, n),		\
		float*:	sum_float_avx2_aligned(arr, n),		\
		double*:sum_double_avx2_aligned(arr, n),	\
		default:	0								\
	)

int main()
{
	//int a[1024] __attribute__((aligned(32)));
	alignas(32) int a[1024];
	alignas(32) float f[1024];
	alignas(32) double d[1024];
	int b[1024];
	for(int i = 0; i < 1024; i++)
	{
		a[i] = b[i] = 1;
		f[i] = d[i] = 1.0;
	}
	asm_test();
	printf("sum = %d\n", SUM_ALIGNED(a, 1024));
	printf("sum = %f\n", SUM_ALIGNED(f, 1024));
	printf("sum = %lf\n", SUM_ALIGNED(d, 1024));
	printf("sum = %d\n", sum_int_avx2_unaligned(a, 1024));
}
