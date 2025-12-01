#include <cuda.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <sys/time.h>
#include <inttypes.h>

#define K PARK
#define NUM_THREADS (1 << (N-K))
#define BLOCK_DIM (NUM_THREADS > 128 ? 128 : NUM_THREADS)
#define GRID_DIM (NUM_THREADS/BLOCK_DIM)
#define PRINT_SOL(X)
#define LOG(level, f_, ...)
#define COEF(I,J) ((((J)*((J)-1))>>1) + (I))

template <class Type>
class cudaData
{
public:
	cudaData(size_t len_host, size_t len_dev=0);
	~cudaData();

	size_t size_host();
	size_t size_dev();
	void clear();
	void write(size_t off_src=0, size_t size=0, size_t off_des=0);		
	void read(size_t off_src=0, size_t size=0, size_t off_des=0);

	Type *host;
	Type *dev;

private:
	size_t sz_host;
	size_t sz_dev;
};


template <class Type>
cudaData<Type>::cudaData(size_t len_host, size_t len_dev)
{
	if(len_dev == 0) len_dev = len_host;

	sz_host = len_host * sizeof(Type);
	sz_dev  = len_dev * sizeof(Type);

	host = (Type*) malloc(sz_host);	

	cudaMalloc((void**) &dev, sz_dev);
}

template <class Type>
cudaData<Type>::~cudaData()
{
    if(host)
        free(host);
    if(dev)
        cudaFree(dev);
}

template <class Type>
size_t cudaData<Type>::size_host()
{
	return sz_host;
}

template <class Type>
size_t cudaData<Type>::size_dev()
{
	return sz_dev;
}

template <class Type>
void cudaData<Type>::clear()
{
	memset(host, 0, sz_host);
}

template <class Type>
void cudaData<Type>::write(size_t off_src, size_t size, size_t off_des)
{
	if(size == 0) size = (sz_host <= sz_dev) ? sz_host : sz_dev;

	cudaMemcpy(&dev[off_src], &host[off_des], size, cudaMemcpyHostToDevice);
}

template <class Type>
void cudaData<Type>::read(size_t off_src, size_t size, size_t off_des)
{
	if(size == 0) size = (sz_host <= sz_dev) ? sz_host : sz_dev;

	cudaMemcpy(&host[off_src], &dev[off_des], size, cudaMemcpyDeviceToHost);
}

#ifndef HAVE_CNT
static const int MultiplyDeBruijnBitPosition[32] = 
{
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

static uint32_t cnt0(uint32_t v)
{
  return MultiplyDeBruijnBitPosition[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
}
#define HAVE_CNT
#endif


uint32_t* check_thread(uint32_t *deg2, uint32_t *deg1, uint32_t thread, uint32_t N)
{
  uint32_t rounds;
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  uint32_t tmp = 0;
  uint32_t count = 0;

  uint32_t diff[K];

  uint32_t* result = (uint32_t*) malloc( (1 << K) * sizeof(uint32_t) );

  diff[0] = deg1[0 * NUM_THREADS + thread];

  for (int i = 1; i < K; i++)
  {
    diff[i] = deg1[i * NUM_THREADS + thread] ^ deg2[COEF(i-1, i)];
  }

  uint32_t res = deg1[K * NUM_THREADS + thread];

  for( rounds = 1; rounds < (1 << K); rounds += 1)
  {
    tmp = (rounds & (rounds-1));
    y = rounds ^ tmp;
    x ^= y;
    z = tmp ^ (tmp & (tmp-1));

    uint32_t y_pos = cnt0(y);
    uint32_t z_pos = cnt0(z);

    if (z_pos > y_pos)
      diff[y_pos] ^= deg2[COEF(y_pos, z_pos)];

    res ^= diff[y_pos];
    if( res == 0 ) result[ count++ ] = x;
  }

  result[ count ] = 0;

  return result;
}


void deg0_coefs(uint32_t *deg2, uint32_t *deg1, uint32_t *result, uint32_t N)
{
	uint32_t rounds;
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t z = 0;
	uint32_t tmp = 0;

  uint32_t diff[N-K];

	diff[0] = deg1[0];

  for (int i = 1; i < (N-K); i++)
  {
    diff[i] = deg1[i] ^ deg2[COEF(i-1, i)];
  }

	uint32_t res = deg1[N-K];

	result[0] = res;

	for (rounds = 1; rounds < (1 << (N-K)); rounds += 1)
	{
		tmp = (rounds & (rounds-1));
		y = rounds ^ tmp;
		x ^= y;
		z = tmp ^ (tmp & (tmp-1));

    uint32_t y_pos = cnt0(y);
    uint32_t z_pos = cnt0(z);

    if (z_pos > y_pos)
      diff[y_pos] ^= deg2[COEF(y_pos, z_pos)];

    res ^= diff[y_pos];
    result[ x ] = res;
    tmp = (y_pos * (y_pos-1)) >> 1;
	}
}

void deg1_coefs(uint32_t *deg1, uint32_t *result, uint32_t N)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t res = deg1[N-K];

	result[0] = res;

	for(uint32_t rounds = 1; rounds < (1 << (N-K)); rounds += 1)
	{
		y = rounds ^ (rounds & (rounds-1));
		x ^= y;

    res ^= deg1[cnt0(y)];
    result[ x ] = res;
	}
}

void partial_eval(uint32_t *sys, uint32_t *deg1, uint32_t N)
{
	uint32_t deg1_sys[(N-K)+1];
	uint32_t deg2_sys[COEF(N-K, N-K)+1];
	uint64_t pos = 0;

	// deg2 part
	for(uint32_t i = 0; i < K; i++)
	{
    for(uint32_t j = 0; j <= (N-K); j++)
    {
      deg1_sys[j] = sys[COEF(0, j + K) + i];
    }

		deg1_coefs(deg1_sys, &deg1[pos], N);
		pos += (1 << (N-K));
	}

	// deg1 part
  for(uint32_t j = 1; j <= (N-K); j++)
  {
    for(uint32_t i = 0; i <= j; i++)
    {
      deg2_sys[COEF(i, j)] = sys[COEF(i + K, j + K)];
    }
  }

  deg0_coefs(deg2_sys, deg2_sys + COEF(0, N-K), &deg1[pos], N);
}



uint32_t check_sol(uint32_t *sys, uint64_t sol, uint32_t N, uint32_t M)
{
	uint32_t i, j, pos = 0;
	uint32_t x[N], check = 0;
   
  for (uint32_t b = 0; b < M; b +=32)
  {
    uint32_t mask = (M-b) >= 32 ? 0xffffffff : ((1 << (M-b))-1);

    for (i = 0; i < N; i++)
      x[i] = ((sol >> i) & 1) ? mask : 0;

    // computing quadratic part
    for(j = 1; j < N; j++)
      for(i = 0; i < j; i++) 
        check ^= sys[pos++] & x[i] & x[j];

    // computing linear part
    for(i = 0; i < N; i++) 
      check ^= sys[pos++] & x[i];

    // constant part
    check ^= sys[pos++];
  }

	return check;
}



double get_ms_time(void) {
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double) timev.tv_sec * 1000 + (double) timev.tv_usec / 1000;
}

extern uint32_t check_sol(uint32_t *sys, uint64_t sol, uint32_t N, uint32_t M);
extern void partial_eval(uint32_t *sys, uint32_t *deg1, uint32_t N);
extern uint32_t* check_thread(uint32_t *deg2, uint32_t *deg1, uint32_t thread, uint32_t N);

__device__ __constant__ uint32_t deg2_block[ K*(K-1)/2 ];

#include "kernel.inc"

uint32_t *pack_sys_data(uint32_t *data, uint32_t N, uint32_t M)
{
//  reduce input system - remove squares

  uint32_t num_blocks = ((M >> 5) + ((M & 31) == 0 ? 0 : 1));

  uint32_t *sys = (uint32_t*)malloc(sizeof(uint32_t)*(N*(N-1)/2 + N + 1)*num_blocks);
  uint32_t *sq0 = (uint32_t*)malloc(sizeof(uint32_t)*N*num_blocks);


  int sq_id = 0;

  int is = 0;
  int id = 0;
  
  for (int v0 = 0; v0 < N; v0++)
  {
    for (int v1 = 0; v1 <= v0; v1++)
    {
      for (uint32_t b = 0; b < M; b +=32)
      {
        uint32_t val = 0;

        for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
          val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

        if (v0 == v1)
          sq0[sq_id + N*(b >> 5)] = val;
        else
          sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val;
      }

      is += 1;

      if (v0 == v1)
        sq_id += 1;
      else
        id += 1;
    }
  }

  for (int v0 = 0; v0 < N; v0++)
  {
    for (uint32_t b = 0; b < M; b +=32)
    {
      uint32_t val = 0;

      for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
        val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

      sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val ^ sq0[v0 + N*(b >> 5)];
    }

    is += 1;
    id += 1;
  }

  {
    for (uint32_t b = 0; b < M; b +=32)
    {
      uint32_t val = 0;

      for (int j = (((M - b) >= 32) ? b + 31 : (M-1)); j >= (int)b; j--)
        val = (val << 1) | data[(N*(N-1)/2 + N + N + 1) * j + is];

      sys[(N*(N-1)/2 + N + 1) * (b >> 5) + id] = val;
    }
  }

  if(sq0)
    free(sq0);

  return sys;
}

int GetDeviceCount() {
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);
    if (err == cudaSuccess) {
      return deviceCount;
    }
    fprintf(stderr, "cudaGetDeviceCount error=%d\n", err);
    return -1;
}

void SetDevice(int device)
{
    cudaSetDevice(device);
}

uint64_t GPUSearchSolution(uint32_t* coefficients, unsigned int number_of_variables, 
                           unsigned int number_of_equations)
{
  uint64_t res = 0;

  uint32_t N = number_of_variables;
  uint32_t M = number_of_equations;


  if (N <= K)
  {
    fprintf(stderr, "N must be larger than K!\n");
    exit(-1);
  }

  uint32_t *sys = pack_sys_data(coefficients, N, M);


  cudaData<uint32_t> deg1((K + 1) * NUM_THREADS);

  partial_eval(sys, deg1.host, N);


  // initialize constant memory space for the quadratic part
  cudaMemcpyToSymbol(deg2_block, sys, sizeof(uint32_t) * K*(K-1)/2);

  // initialize global memory space for the linear parts
  deg1.write();

  // initialize global memory space for the results of each threads
  cudaData<uint32_t> result(NUM_THREADS);



  guess<<<GRID_DIM, BLOCK_DIM>>>(deg1.dev, result.dev, NUM_THREADS);



  result.read();



  int32_t ans; 

  for(uint64_t i = 0; i < NUM_THREADS; i++)
  {
    ans = result.host[i];

    if(ans)
    {

      if (ans & 0x80000000) // more than one solution 
      {
        uint32_t * sols = check_thread(sys, deg1.host, i, N);


        for(uint32_t j = 0; sols[j]; j++)
        {
          if (check_sol(sys, (i << K) | sols[j], N, M) == 1)
          {
            PRINT_SOL((i << K) | sols[j]);

            res = (i << K) | sols[j];
            free(sols);
            goto end;
           
          }
        }


        free(sols);
      }
      else // only one solution
      {
        if (check_sol(sys, (i << K) | ans, N, M) == 0)
        {
          PRINT_SOL((i << K) | ans);

          res = (i << K) | ans;

          goto end;
        }
      }
    }

    if(deg1.host[K * NUM_THREADS + i] == 0)
    {
      if (check_sol(sys, (i << K) | 0, N, M) == 0)
      {
        PRINT_SOL(i << K);

        res = (i << K);

        goto end;
      }
    }
  }

end:

    if(sys)
        free(sys);

  return res;
}
