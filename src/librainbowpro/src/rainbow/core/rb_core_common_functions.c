#include "rb_core_config.h"
#include "rb_core_RAINBOW_PRO_API.h"

#ifdef RB_SAFE_MEM
int rb_malloc_cnt = 0;
int rb_free_cnt = 0;
int rb_current_mem = 0;
int rb_max_mem = 0;

#define MM01	512
#define MM02	(MM01/2)
#define MM03    4

#define MM04	 ((unsigned long long)0xffffffffffffffff<<(6))


void* rb_safe_malloc(unsigned int length)
{
	unsigned long* addr = NULL;
	unsigned long* addr2 = NULL;
	unsigned long* p = NULL;
	unsigned long offset = 0;
	unsigned long offset_p = 0;
	unsigned long long temp = MM04;
	unsigned long  new_len = (length + MM01 + sizeof(unsigned long)) / sizeof(unsigned long) * sizeof(unsigned long);

	
	addr = (unsigned long*)malloc(new_len);
	if(addr)
		memset((unsigned char*)addr, 0, new_len);
	
	p = (unsigned long*)(((unsigned long)addr + MM02) & (unsigned long)temp);
	
	rb_malloc_cnt++;
	rb_current_mem += length;
	
	offset   = ((unsigned long)(p + MM03)   - (unsigned long)addr)/sizeof(unsigned long);
	offset_p = ((unsigned long)(p +    0)   - (unsigned long)addr)/sizeof(unsigned long);


	p[0] = (unsigned long)addr;
	p[1] = (unsigned long)offset;
	p[2] = (unsigned long)length;
	p[3] = (unsigned long)rb_malloc_cnt;

	if (rb_current_mem > rb_max_mem)
		rb_max_mem = rb_current_mem;
	
#ifdef rb_debug_printf
	printf("\n Malloc: %d: f_addr: 0x%x, a_addr is: 0x%x, v_addr: 0x%x, length = %d, current = %d, offset:%d, rb_max_mem:%d \n", 
		rb_malloc_cnt,
		(unsigned long)(addr),
		(unsigned long)(p),
		(unsigned long)(p + MM03),
		length, 
		rb_current_mem, 
		(unsigned long)offset,
		rb_max_mem);
#endif

	return (void*)(p + MM03);

}


void* rb_safe_calloc(unsigned int num, unsigned int size)
{
	unsigned long* addr=NULL;
	unsigned long* p=NULL;
	unsigned long offset = 0;
	unsigned long offset_p = 0;
	unsigned long length = num * size;
	unsigned long long temp = MM04;
	unsigned long  new_len = (length + MM01 + sizeof(unsigned long)) / sizeof(unsigned long) * sizeof(unsigned long);

	addr = (unsigned long*)malloc(new_len);
	if (addr)
		memset((unsigned long*)addr, 0, new_len / sizeof(unsigned long));

	p = (unsigned long*)(((unsigned long)addr + MM02) & (unsigned long)temp);


	rb_malloc_cnt++;
	rb_current_mem += length;
	offset   = ((unsigned long)(p + MM03) - (unsigned long)addr) / sizeof(unsigned long);
	offset_p = ((unsigned long)(p +    0) - (unsigned long)addr) / sizeof(unsigned long);

	
	p[0] = (unsigned long)addr;
	p[1] = (unsigned long)offset;
	p[2] = (unsigned long)length;
	p[3] = (unsigned long)rb_malloc_cnt;

	if (rb_current_mem > rb_max_mem)
		rb_max_mem = rb_current_mem;


#ifdef rb_debug_printf
	printf("\n Calloc: %d: f_addr: 0x%x, a_addr is: 0x%x, v_addr: 0x%x, length = %d, current = %d, offset:%d, rb_max_mem:%d \n",
		rb_malloc_cnt,
		(unsigned long)(addr),
		(unsigned long)(p),
		(unsigned long)(p + MM03),
		length,
		rb_current_mem,
		(unsigned long)offset,
		rb_max_mem);
#endif

	return (void*)(p + MM03);

}




void rb_safe_free(void* p)
{
	if (p)
	{
		unsigned long addr = 0;
		unsigned long offset = 0;
		unsigned long length = 0;
		unsigned long rb_malloc_cnt0 = 0;
		unsigned long* p1=NULL;


		p1 = &(((unsigned long*)p)[-MM03]);
		addr = p1[0];
		offset = p1[1];
		length = p1[2];
		rb_malloc_cnt0 = p1[3];

		rb_free_cnt++;
		rb_current_mem -= length;



#ifdef rb_debug_printf
		printf("\n Free: %d: f_addr: 0x%x, a_addr is: 0x%x, v_addr: 0x%x, length = %d, current = %d, offset:%d, m_cnt:%d, rb_max_mem:%d \n",
			rb_free_cnt,
			(unsigned long)(addr),
			(unsigned long)(p1),
			(unsigned long)(p),
			length,
			rb_current_mem,
			(unsigned long)offset,
			rb_malloc_cnt0,
			rb_max_mem);
#endif

		free((void*)addr);
		p = NULL;

	}

	return;

}


void* rb_safe_realloc(void* p, unsigned int _NewSize)
{

	unsigned long* new_addr = NULL;
	unsigned long old_len = 0;
	unsigned long* p1 = &(((unsigned long*)p)[-MM03]);
	old_len = p1[2];

	if (old_len < _NewSize)
	{
		new_addr = rb_safe_malloc(_NewSize);
		if(new_addr)
			memset((unsigned char*)new_addr, 0, _NewSize);

		memcpy(new_addr, p, old_len);
		rb_safe_free(p);
	}
	else
		new_addr = p;
#ifdef rb_debug_printf
	printf("\n Realloc:\n");
#endif

	return new_addr;


}

#else

void* rb_safe_malloc(unsigned int length)
{
	void* temp = malloc(length);
	if(temp)
		memset(temp, 0, length);
	return temp;
}

void* rb_safe_calloc(unsigned int num, unsigned int size)
{
	return calloc(num,size);
}


void rb_safe_free(void* p)
{
	if (p)
	{
		free(p);
		p = NULL;
	}
	return;
}


void* rb_safe_realloc(void* p, unsigned int _NewSize)
{
	return realloc(p, _NewSize);
}
#endif//RB_SAFE_MEM

//#define display_cpu_info

#ifdef RB_SIMD_X86
	rb_cpu_s rb_cpu_su;
#endif


#if defined(LINUX_VERSION_X86) && defined(RB_SIMD_X86)
struct CacheTLBIndex {
	unsigned char Index;
	char STRU[200];
};
struct CacheTLBIndex CacheTLB[110] = {
	// [0:66]: Cache
	// [67:107]: TLB
	// [108:109]: Prefetch
	{0x06, "L1 Instruction Cache:\t8 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x08, "L1 Instruction Cache:\t16 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x09, "L1 Instruction Cache:\t32 KBytes, 4-way set associative, 64 byte line size\n"},
	{0x0A, "L1 Data Cache:       \t8 KBytes, 2-way set associative, 32 byte line size\n"},
	{0x0C, "L1 Data Cache:       \t16 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x0D, "L1 Data Cache:       \t16 KBytes, 4-way set associative, 64 byte line size\n"},
	{0x0E, "L1 Data Cache:       \t24 KBytes, 6-way set associative, 64 byte line size\n"},
	{0x1D, "L2 Cache:            \t128 KBytes, 2-way set associative, 64 byte line size\n"},
	{0x21, "L2 Cache:            \t256 KBytes, 8-way set associative, 64 byte line size\n"},
	{0x22, "L3 Cache:            \t512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x23, "L3 Cache:            \t1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x24, "L2 Cache:            \t1 MBytes, 16-way set associative, 64 byte line size\n"},
	{0x25, "L3 Cache:            \t2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x29, "L3 Cache:            \t4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x2C, "L1 Data Cache:       \t32 KBytes, 8-way set associative, 64 byte line size\n"},
	{0x30, "L1 Instruction Cache:\t32 KBytes, 8-way set associative, 64 byte line size\n"},
	{0x40, "No L2 Cache or, if processor contains a valid L2 Cache, no L3 Cache\n"},
	{0x41, "L2 Cache:            \t128 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x42, "L2 Cache:            \t256 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x43, "L2 Cache:            \t512 KBytes, 4-way set associative, 32 byte line size\n"},
	{0x44, "L2 Cache:            \t1 MByte, 4-way set associative, 32 byte line size\n"},
	{0x45, "L2 Cache:            \t2 MByte, 4-way set associative, 32 byte line size\n"},
	{0x46, "L3 Cache:            \t4 MByte, 4-way set associative, 64 byte line size\n"},
	{0x47, "L3 Cache:            \t8 MByte, 8-way set associative, 64 byte line size\n"},
	{0x48, "L2 Cache:            \t3 MByte, 12-way set associative, 64 byte line size\n"},
	{0x49, "L3 Cache:            \t4 MByte, 16-way set associative, 64 byte line size (Intel Xeon processor MP, Family 0FH, Model 06H); L2 Cache: 4 MByte, 16-way set associative, 64 byte line size\n"},
	{0x4A, "L3 Cache:            \t6 MByte, 12-way set associative, 64 byte line size\n"},
	{0x4B, "L3 Cache:            \t8 MByte, 16-way set associative, 64 byte line size\n"},
	{0x4C, "L3 Cache:            \t12 MByte, 12-way set associative, 64 byte line size\n"},
	{0x4D, "L3 Cache:            \t16 MByte, 16-way set associative, 64 byte line size\n"},
	{0x4E, "L2 Cache:            \t6 MByte, 24-way set associative, 64 byte line size\n"},
	{0x60, "L1 Data Cache:       \t16 KByte, 8-way set associative, 64 byte line size\n"},
	{0x66, "L1 Data Cache:       \t8 KByte, 4-way set associative, 64 byte line size\n"},
	{0x67, "L1 Data Cache:       \t16 KByte, 4-way set associative, 64 byte line size\n"},
	{0x68, "L1 Data Cache:       \t32 KByte, 4-way set associative, 64 byte line size\n"},
	{0x70, "Trace Cache:         \t12 K-¦Ìop, 8-way set associative\n"},
	{0x71, "Trace Cache:         \t16 K-¦Ìop, 8-way set associative\n"},
	{0x72, "Trace Cache:         \t32 K-¦Ìop, 8-way set associative\n"},
	{0x78, "L2 Cache:            \t1 MByte, 4-way set associative, 64 byte line size\n"},
	{0x79, "L2 Cache:            \t128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x7A, "L2 Cache:            \t256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x7B, "L2 Cache:            \t512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x7C, "L2 Cache:            \t1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector\n"},
	{0x7D, "L2 Cache:            \t2 MByte, 8-way set associative, 64 byte line size\n"},
	{0x7F, "L2 Cache:            \t512 KByte, 2-way set associative, 64 byte line size\n"},
	{0x80, "L2 Cache:            \t512 KByte, 8-way set associative, 64 byte line size\n"},
	{0x82, "L2 Cache:            \t256 KByte, 8-way set associative, 32 byte line size\n"},
	{0x83, "L2 Cache:            \t512 KByte, 8-way set associative, 32 byte line size\n"},
	{0x84, "L2 Cache:            \t1 MByte, 8-way set associative, 32 byte line size\n"},
	{0x85, "L2 Cache:            \t2 MByte, 8-way set associative, 32 byte line size\n"},
	{0x86, "L2 Cache:            \t512 KByte, 4-way set associative, 64 byte line size\n"},
	{0x87, "L2 Cache:            \t1 MByte, 8-way set associative, 64 byte line size\n"},
	{0xD0, "L3 Cache:            \t512 KByte, 4-way set associative, 64 byte line size\n"},
	{0xD1, "L3 Cache:            \t1 MByte, 4-way set associative, 64 byte line size\n"},
	{0xD2, "L3 Cache:            \t2 MByte, 4-way set associative, 64 byte line size\n"},
	{0xD6, "L3 Cache:            \t1 MByte, 8-way set associative, 64 byte line size\n"},
	{0xD7, "L3 Cache:            \t2 MByte, 8-way set associative, 64 byte line size\n"},
	{0xD8, "L3 Cache:            \t4 MByte, 8-way set associative, 64 byte line size\n"},
	{0xDC, "L3 Cache:            \t1.5 MByte, 12-way set associative, 64 byte line size\n"},
	{0xDD, "L3 Cache:            \t3 MByte, 12-way set associative, 64 byte line size\n"},
	{0xDE, "L3 Cache:            \t6 MByte, 12-way set associative, 64 byte line size\n"},
	{0xE2, "L3 Cache:            \t2 MByte, 16-way set associative, 64 byte line size\n"},
	{0xE3, "L3 Cache:            \t4 MByte, 16-way set associative, 64 byte line size\n"},
	{0xE4, "L3 Cache:            \t8 MByte, 16-way set associative, 64 byte line size\n"},
	{0xEA, "L3 Cache:            \t12 MByte, 24-way set associative, 64 byte line size\n"},
	{0xEB, "L3 Cache:            \t18 MByte, 24-way set associative, 64 byte line size\n"},
	{0xEC, "L3 Cache:            \t24 MByte, 24-way set associative, 64 byte line size\n"},
	{0x6A, "Unified TLB:         \t4 KByte pages, 8-way set associative, 64 entries\n"},
	{0x6B, "Data TLB:            \t4 KByte pages, 8-way set associative, 256 entries\n"},
	{0x6C, "Data TLB:            \t2 MByte/4 MByte pages, 8-way set associative, 128 entries\n"},
	{0x6D, "Data TLB:            \t1 GByte pages, fully associative, 16 entries\n"},
	{0x0B, "Instruction TLB:     \t4 MByte pages, 4-way set associative, 4 entries\n"},
	{0x01, "Instruction TLB:     \t4 KByte pages, 4-way set associative, 32 entries\n"},
	{0x02, "Instruction TLB:     \t4 MByte pages, fully associative, 2 entries\n"},
	{0x03, "Data TLB:            \t4 KByte pages, 4-way set associative, 64 entries\n"},
	{0x04, "Data TLB:            \t4 MByte pages, 4-way set associative, 8 entries\n"},
	{0x05, "L2 Data TLB:         \t4 MByte pages, 4-way set associative, 32 entries\n"},
	{0x4F, "Instruction TLB:     \t4 KByte pages, 32 entries\n"},
	{0x50, "Instruction TLB:     \t4 KByte and 2 MByte or 4 MByte pages, 64 entries\n"},
	{0x51, "Instruction TLB:     \t4 KByte and 2 MByte or 4 MByte pages, 128 entries\n"},
	{0x52, "Instruction TLB:     \t4 KByte and 2 MByte or 4 MByte pages, 256 entries\n"},
	{0x55, "Instruction TLB:     \t2 MByte/4 MByte pages, fully associative, 7 entries\n"},
	{0x56, "L1 Data TLB:         \t4 MByte pages, 4-way set associative, 16 entries\n"},
	{0x57, "L1 Data TLB:         \t4 KByte pages, 4-way associative, 16 entries\n"},
	{0x59, "L1 Data TLB:         \t4 KByte pages, fully associative, 16 entries\n"},
	{0x5A, "L1 Data TLB:         \t2 MByte/4 MByte pages, 4-way set associative, 32 entries\n"},
	{0x5B, "Data TLB:            \t4 KByte and 4 MByte pages, 64 entries\n"},
	{0x5C, "Data TLB:            \t4 KByte and 4 MByte pages,128 entries\n"},
	{0x5D, "Data TLB:            \t4 KByte and 4 MByte pages,256 entries\n"},
	{0x61, "Instruction TLB:     \t4 KByte pages, fully associative, 48 entries\n"},
	{0x63, "Data TLB:            \t2 MByte/4 MByte pages, 4-way set associative, 32 entries and a separate array with 1 GByte pages, 4-way set associative, 4 entries\n"},
	{0x64, "Data TLB:            \t4 KByte pages, 4-way set associative, 512 entries\n"},
	{0x76, "Instruction TLB:     \t2 MByte/4 MByte pages, fully associative, 8 entries\n"},
	{0xA0, "Data TLB:            \t4 KByte pages, fully associative, 32 entries\n"},
	{0xB0, "Instruction TLB:     \t4 KByte pages, 4-way set associative, 128 entries\n"},
	{0xB1, "Instruction TLB:     \t2 MByte pages, 4-way, 8 entries or 4M pages, 4-way, 4 entries\n"},
	{0xB2, "Instruction TLB:     \t4 KByte pages, 4-way set associative, 64 entries\n"},
	{0xB3, "Data TLB:            \t4 KByte pages, 4-way set associative, 128 entries\n"},
	{0xB4, "L2 Data TLB:         \t4 KByte pages, 4-way associative, 256 entries\n"},
	{0xB5, "Instruction TLB:     \t4 KByte pages, 8-way set associative, 64 entries\n"},
	{0xB6, "Instruction TLB:     \t4 KByte pages, 8-way set associative, 128 entries\n"},
	{0xBA, "L2 Data TLB:         \t4 KByte pages, 4-way associative, 64 entries\n"},
	{0xC0, "Data TLB:            \t4 KByte and 4 MByte pages, 4-way associative, 8 entries\n"},
	{0xC1, "L2 Shared TLB:       \t4 KByte/2MByte pages, 8-way associative, 1024 entries\n"},
	{0xC2, "Data TLB:            \t4 KByte/2 MByte pages, 4-way associative, 16 entries\n"},
	{0xC3, "L2 Shared TLB:       \t4 KByte/2 MByte pages, 6-way associative, 1536 entries. Also 1 GBbyte pages, 4-way, 16 entries.\n"},
	{0xC4, "Data TLB:            \t2 MByte/4 MByte pages, 4-way associative, 32 entries\n"},
	{0xCA, "L2 Shared TLB:       \t4 KByte pages, 4-way associative, 512 entries\n"},
	{0xF0, "Prefetch:            \t64 Byte prefetching\n"},
	{0xF1, "Prefetch:            \t128 Byte prefetching\n"}
};
void FindCache_Intel()
{
	// 04H
	unsigned int CacheInfoA_Intel;	// [4:0]: Cache Type, 0:NULL;1:Data Cache;2:Instruction Cache;3:Unified Cache
									// [7:5]: Cache Level
									// [8]: Self Initializing cache level (does not need SW initialization)
									// [9]: Fully Associative cache
									// [25:14]: Maximum number of addressable IDs for logical processors sharing this cache
									// [31:26]: Maximum number of addressable IDs for processor cores in the physical package
	unsigned int CacheInfoB_Intel;	// [11:0]: System Coherency Line Size
									// [21:12]: Physical Line partitions
									// [31:22]: Ways of associativity
	unsigned int CacheInfoC_Intel;	// Number of Sets
	unsigned int CacheInfoD_Intel;	// [0]: Write-Back 0=VD/1=INVD
									// [1]: Cache Inclusiveness
									// [2]: Complex Cache Indexing
	unsigned int CacheInfoC2_Intel;	// [7:0]: Cache Line size in bytes
									// [15:12]: L2 Associativity field
									// [31:16]: Cache size in 1K units
	unsigned int i = 0;
	unsigned int Level, LineSize, CacheWays, CacheSize;
	unsigned int K;
	char* CacheType[4];
	char* CacheAssoc[16];
	CacheType[0] = "NULL"; CacheType[1] = "Data Cache:       "; CacheType[2] = "Instruction Cache:"; CacheType[3] = "Unified Cache:    ";
	CacheAssoc[0] = "DIsabled"; CacheAssoc[1] = "1-way set"; CacheAssoc[2] = "2-way set"; CacheAssoc[4] = "4-way set";
	CacheAssoc[6] = "8-way set"; CacheAssoc[8] = "16-way set"; CacheAssoc[10] = "32-way set"; CacheAssoc[11] = "48-way set";
	CacheAssoc[12] = "64-way set"; CacheAssoc[13] = "96-way set"; CacheAssoc[14] = "128-way set"; CacheAssoc[15] = "fully";
	printf("\nCPU Cache Information:\n");
	printf("------------------------------------------------------------\n");
	while (1) {
		__asm__(
			"movl $4, %%eax;"
			"movl %4, %%ecx;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			"movl %%edx, %3;"
			:"=a"(CacheInfoA_Intel), "=b"(CacheInfoB_Intel), "=c"(CacheInfoC_Intel), "=d"(CacheInfoD_Intel)
			: "c"(i)
			: "memory"
		);
		if (!(CacheInfoA_Intel & 0x1F)) break;
		Level = ((CacheInfoA_Intel & 0xE0) >> 5);
		LineSize = (CacheInfoB_Intel & 0xFFF) + 1;
		CacheWays = ((CacheInfoB_Intel & 0xFFC00000) >> 22) + 1;
		CacheSize = (CacheInfoC_Intel + 1) * LineSize * CacheWays;
		K = !(CacheSize >> 20);
		CacheSize = K ? (CacheSize >> 10) : (CacheSize >> 20);
		if (CacheInfoA_Intel & 0x200)
			printf("L%d %s\t%d %sytes, fully associative, %d byte line size\n",
				Level, CacheType[CacheInfoA_Intel & 0xF], CacheSize, K ? "KB" : "MB", LineSize);
		else
			printf("L%d %s\t%d %sytes, %d-way set associative, %d byte line size\n",
				Level, CacheType[CacheInfoA_Intel & 0xF], CacheSize, K ? "KB" : "MB", CacheWays, LineSize);
		i++;
	}
	if (!i) {
		__asm__(
			"movl $0x80000006, %%eax;"
			"cpuid;"
			"movl %%ecx, %0;"
			:"=c"(CacheInfoC2_Intel)
			:
			: "memory"
		);
		if (CacheInfoC2_Intel) {
			CacheSize = (CacheInfoC2_Intel & 0xFFFF0000) >> 16;
			K = !(CacheSize >> 10);
			CacheSize = K ? (CacheSize) : (CacheSize >> 10);
			printf("L2 Cache:            \t%d %sytes, %s associative, %d byte line size\n",
				CacheSize, K ? "KB" : "MB", CacheAssoc[(CacheInfoC2_Intel & 0xF000) >> 12], CacheInfoC2_Intel & 0xFF);
		}
	}
}
void FindTLB_Intel()
{
	// 18H
	unsigned int TLBInfoA_Intel;	// Maximum input value of supported sub-leaf
	unsigned int TLBInfoB_Intel;	// [0]: 4K page size entries supported by this structure
									// [1]: 2MB page size entries supported by this structure
									// [2]: 4MB page size entries supported by this structure
									// [3]: 1 GB page size entries supported by this structure
									// [10:8]: Partitioning (0: Soft partitioning between the logical processors sharing this structure)
									// [31:16]: Ways of associativity
	unsigned int TLBInfoC_Intel;	// Number of Sets
	unsigned int TLBInfoD_Intel;	// [4:0]: Translation cache type field, 0:NULL;1:Data TLB;2:Instruction TLB;3:Unified TLB
									// [7:5]: Translation cache level (starts at 1)
									// [8]: Fully associative structure
									// [25:14]: Maximum number of addressable IDs for logical processors sharing this translation cache
	unsigned int i = 0;
	unsigned int Level, TLBWays;
	char* TLBPage[16];
	char* TLBType[4];
	TLBPage[0] = "NULL"; TLBPage[1] = "4 KByte"; TLBPage[2] = "2 MByte"; TLBPage[3] = "4 KByte/2 MByte";
	TLBPage[4] = "4 MByte"; TLBPage[5] = "4 KByte/4 MByte"; TLBPage[6] = "2 MByte/4 MByte"; TLBPage[7] = "4 KByte/2 MByte/4 MByte";
	TLBPage[8] = "1 GByte"; TLBPage[9] = "4 KByte/1 GByte"; TLBPage[10] = "2 MByte/1 GByte"; TLBPage[11] = "4 KByte/2 MByte/1 GByte";
	TLBPage[12] = "4 MByte/1 GByte"; TLBPage[13] = "4 KByte/4 MByte/1 GByte"; TLBPage[14] = "2 MByte/4 MByte/1 GByte"; TLBPage[15] = "4 KByte/2 MByte/4 MByte/1 GByte";
	TLBType[0] = "NULL"; TLBType[1] = "Data"; TLBType[2] = "Instruction"; TLBType[3] = "Unified";
	printf("\nCPU TLB Information:\n");
	printf("------------------------------------------------------------\n");
	while (1) {
		__asm__(
			"movl $0x18, %%eax;"
			"movl %4, %%ecx;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			"movl %%edx, %3;"
			:"=a"(TLBInfoA_Intel), "=b"(TLBInfoB_Intel), "=c"(TLBInfoC_Intel), "=d"(TLBInfoD_Intel)
			: "c"(i)
			: "memory"
		);
		if (!(TLBInfoD_Intel & 0x1F)) break;
		Level = (TLBInfoD_Intel & 0xE0) >> 5;
		TLBWays = (TLBInfoB_Intel & 0xFFFF0000) >> 16;
		if (TLBInfoD_Intel & 0x100)
			printf("L%d %s TLB:\t%s pages, fully associative, %d entries\n",
				Level, TLBType[TLBInfoD_Intel & 0x3], TLBPage[TLBInfoB_Intel & 0xF], TLBInfoC_Intel);
		else
			printf("L%d %s TLB:\t%s pages, %d-way associative, %d entries\n",
				Level, TLBType[TLBInfoD_Intel & 0x3], TLBPage[TLBInfoB_Intel & 0xF], TLBWays, TLBInfoC_Intel);
		i++;
	}
}
void ListCacheTLB_Intel(unsigned int CacheTLBInfo_Intel[4])
{
	unsigned int i, j, k;
	unsigned char p[16];
	CacheTLBInfo_Intel[0] &= 0xFFFFFF00;
	for (j = 0; j < 4; j++) if (CacheTLBInfo_Intel[j] & 0x80000000) CacheTLBInfo_Intel[j] = 0x0;
	memcpy(p, CacheTLBInfo_Intel, 16);
	printf("\nCPU Cache Information:\n");
	printf("------------------------------------------------------------\n");
	for (k = 0; k < 16; k++) {
		for (i = 0; i < 67; i++) {
			if (CacheTLB[i].Index == p[k]) printf("%s", CacheTLB[i].STRU);
		}
	}
	printf("\nCPU TLB Information:\n");
	printf("------------------------------------------------------------\n");
	for (k = 0; k < 16; k++) {
		for (i = 67; i < 108; i++) {
			if (CacheTLB[i].Index == p[k]) printf("%s", CacheTLB[i].STRU);
		}
	}
	printf("\nCPU Prefetch Information:\n");
	printf("------------------------------------------------------------\n");
	for (k = 0; k < 16; k++) {
		for (i = 108; i < 110; i++) {
			if (CacheTLB[i].Index == p[k]) printf("%s", CacheTLB[i].STRU);
		}
	}
}
void OtherCacheTLB_Intel()
{

}
void FindL1_AMD()
{
	// 80000005H 
	unsigned int CacheTLBInfo1A_AMD;// [7:0]: Instruction TLB number of entries for 2-MB and 4-MB pages
									// [15:8]: Instruction TLB associativity for 2-MB and 4-MB pages
									// [23:16]: Data TLB number of entries for 2-MB and 4-MB pages
									// [31:24]: Data TLB associativity for 2-MB and 4-MB pages
	unsigned int CacheTLBInfo1B_AMD;// [7:0]: Instruction TLB number of entries for 4 KB pages
									// [15:8]: Instruction TLB associativity for 4 KB pages
									// [23:16]: Data TLB number of entries for 4 KB pages
									// [31:24]: Data TLB associativity for 4 KB pages
	unsigned int CacheTLBInfo1C_AMD;// [7:0]: L1 data cache line size in bytes
									// [15:8]: L1 data cache lines per tag
									// [23:16]: L1 data cache associativity
									// [31:24]: L1 data cache size in KB
	unsigned int CacheTLBInfo1D_AMD;// [7:0]: L1 instruction cache line size in bytes
									// [15:8]: L1 instruction cache lines per tag
									// [23:16]: L1 instruction cache associativity
									// [31:24]: L1 instruction cache size KB
	unsigned int DTLBAssocA, DTLBEntriesA, ITLBAssocA, ITLBEntriesA;
	unsigned int DTLBAssocB, DTLBEntriesB, ITLBAssocB, ITLBEntriesB;
	unsigned int DCacheSize, DCacheAssoc, DCacheLines, DCacheLineSize;
	unsigned int ICacheSize, ICacheAssoc, ICacheLines, ICacheLineSize;
	printf("\nCPU L1 Cache and TLB Information:\n");
	printf("------------------------------------------------------------\n");
	__asm__(
		"movl $0x80000005, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(CacheTLBInfo1A_AMD), "=b"(CacheTLBInfo1B_AMD), "=c"(CacheTLBInfo1C_AMD), "=d"(CacheTLBInfo1D_AMD)
		:
		: "memory"
	);
	DTLBAssocA = (CacheTLBInfo1A_AMD & 0xFF000000) >> 24;
	DTLBEntriesA = (CacheTLBInfo1A_AMD & 0xFF0000) >> 16;
	ITLBAssocA = (CacheTLBInfo1A_AMD & 0xFF00) >> 8;
	ITLBEntriesA = (CacheTLBInfo1A_AMD & 0xFF);
	DTLBAssocB = (CacheTLBInfo1B_AMD & 0xFF000000) >> 24;
	DTLBEntriesB = (CacheTLBInfo1B_AMD & 0xFF0000) >> 16;
	ITLBAssocB = (CacheTLBInfo1B_AMD & 0xFF00) >> 8;
	ITLBEntriesB = (CacheTLBInfo1B_AMD & 0xFF);
	DCacheSize = (CacheTLBInfo1C_AMD & 0xFF000000) >> 24;
	DCacheAssoc = (CacheTLBInfo1C_AMD & 0xFF0000) >> 16;
	DCacheLines = (CacheTLBInfo1C_AMD & 0xFF00) >> 8;
	DCacheLineSize = (CacheTLBInfo1C_AMD & 0xFF);
	ICacheSize = (CacheTLBInfo1D_AMD & 0xFF000000) >> 24;
	ICacheAssoc = (CacheTLBInfo1D_AMD & 0xFF0000) >> 16;
	ICacheLines = (CacheTLBInfo1D_AMD & 0xFF00) >> 8;
	ICacheLineSize = (CacheTLBInfo1D_AMD & 0xFF);
	if (DTLBAssocA == 0xFF)
		printf("L1 Data TLB:         \t2 MByte/4 MByte pages, fully associative, %d entries\n", DTLBEntriesA);
	else
		printf("L1 Data TLB:         \t2 MByte/4 MByte pages, %d-way set associative, %d entries\n", DTLBAssocA, DTLBEntriesA);
	if (ITLBAssocA == 0xFF)
		printf("L1 Instruction TLB:  \t2 MByte/4 MByte pages, fully associative, %d entries\n", ITLBEntriesA);
	else
		printf("L1 Instruction TLB:  \t2 MByte/4 MByte pages, %d-way set associative, %d entries\n", ITLBAssocA, ITLBEntriesA);
	if (DTLBAssocB == 0xFF)
		printf("L1 Data TLB:         \t4 KByte pages, fully associative, %d entries\n", DTLBEntriesB);
	else
		printf("L1 Data TLB:         \t4 KByte pages, %d-way set associative, %d entries\n", DTLBAssocB, DTLBEntriesB);
	if (ITLBAssocB == 0xFF)
		printf("L1 Instruction TLB:  \t4 KByte pages, fully associative, %d entries\n", ITLBEntriesB);
	else
		printf("L1 Instruction TLB:  \t4 KByte pages, %d-way set associative, %d entries\n", ITLBAssocB, ITLBEntriesB);
	if (DCacheAssoc == 0xFF)
		printf("L1 Data Cache:       \t%d KByte, fully associative, %d byte line size\n", DCacheSize, DCacheLineSize);
	else
		printf("L1 Data Cache:       \t%d KByte, %d-way set associative, %d byte line size\n", DCacheSize, DCacheAssoc, DCacheLineSize);
	if (ICacheAssoc == 0xFF)
		printf("L1 Instruction Cache:\t%d KByte, fully associative, %d byte line size\n", ICacheSize, ICacheLineSize);
	else
		printf("L1 Instruction Cache:\t%d KByte, %d-way set associative, %d byte line size\n", ICacheSize, ICacheAssoc, ICacheLineSize);
}
void FindL2L3_AMD()
{
	// 80000006H : 
	unsigned int CacheTLBInfo2A_AMD;// [11:0]: L2 instruction TLB number of entries for 2-MB and 4-MB pages
									// [15:12]: L2 instruction TLB associativity for 2-MB and 4-MB pages
									// [27:16]: L2 data TLB number of entries for 2-MB and 4-MB pages
									// [31:28]: L2 data TLB associativity for 2-MB and 4-MB pages
	unsigned int CacheTLBInfo2B_AMD;// [11:0]: L2 instruction TLB number of entries for 4-KB pages
									// [15:12]: L2 instruction TLB associativity for 4-KB pages
									// [27:16]: L2 data TLB number of entries for 4-KB pages
									// [31:28]: L2 data TLB associativity for 4-KB pages
	unsigned int CacheTLBInfo2C_AMD;// [7:0]: L2 cache line size in bytes
									// [11:8]: L2 cache lines per tag
									// [15:12]: L2 cache associativity
									// [31:16]: L2 cache size in KB
	unsigned int CacheTLBInfo2D_AMD;// [7:0]: L3 cache line size in bytes
									// [11:8]: L3 cache lines per tag
									// [15:12]: L3 cache associativity
									// [31:18]: Specifies the L3 cache size range
	unsigned int DTLBAssocA, DTLBEntriesA, ITLBAssocA, ITLBEntriesA;
	unsigned int DTLBAssocB, DTLBEntriesB, ITLBAssocB, ITLBEntriesB;
	unsigned int L2CacheSize, L2CacheAssoc, L2CacheLines, L2CacheLineSize;
	unsigned int L3CacheSize, L3CacheAssoc, L3CacheLines, L3CacheLineSize;
	unsigned int K2, K3;
	char* AssocWay[16];
	AssocWay[0] = "disabled"; AssocWay[1] = "direct mapped"; AssocWay[2] = "2-way associative"; AssocWay[3] = "3-way associative";
	AssocWay[4] = "4-way associative"; AssocWay[5] = "6-way associative"; AssocWay[6] = "8-way associative"; AssocWay[7] = "null";
	AssocWay[8] = "16-way associative"; AssocWay[9] = "null"; AssocWay[10] = "32-way associative"; AssocWay[11] = "48-way associative";
	AssocWay[12] = "64-way associative"; AssocWay[13] = "96-way associative"; AssocWay[14] = "128-way associative"; AssocWay[15] = "fully associative";
	printf("\nCPU L2 Cache and TLB and L3 Cache Information:\n");
	printf("------------------------------------------------------------\n");
	__asm__(
		"movl $0x80000006, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(CacheTLBInfo2A_AMD), "=b"(CacheTLBInfo2B_AMD), "=c"(CacheTLBInfo2C_AMD), "=d"(CacheTLBInfo2D_AMD)
		:
		: "memory"
	);
	DTLBAssocA = (CacheTLBInfo2A_AMD & 0xF0000000) >> 28;
	DTLBEntriesA = (CacheTLBInfo2A_AMD & 0xFFF0000) >> 16;
	ITLBAssocA = (CacheTLBInfo2A_AMD & 0xF000) >> 12;
	ITLBEntriesA = (CacheTLBInfo2A_AMD & 0xFFF);
	DTLBAssocB = (CacheTLBInfo2B_AMD & 0xF0000000) >> 28;
	DTLBEntriesB = (CacheTLBInfo2B_AMD & 0xFFF0000) >> 16;
	ITLBAssocB = (CacheTLBInfo2B_AMD & 0xF000) >> 12;
	ITLBEntriesB = (CacheTLBInfo2B_AMD & 0xFFF);
	L2CacheSize = (CacheTLBInfo2C_AMD & 0xFFFF0000) >> 16;
	L2CacheAssoc = (CacheTLBInfo2C_AMD & 0xF000) >> 12;
	L2CacheLines = (CacheTLBInfo2C_AMD & 0xF00) >> 8;
	L2CacheLineSize = (CacheTLBInfo2C_AMD & 0xFF);
	K2 = !(L2CacheSize >> 10);
	L2CacheSize = K2 ? (L2CacheSize) : (L2CacheSize >> 10);
	L3CacheSize = (CacheTLBInfo2D_AMD & 0xFFFC0000) >> 9;
	K3 = !(L3CacheSize >> 10);
	L3CacheSize = K3 ? (L3CacheSize) : (L3CacheSize >> 10);
	L3CacheAssoc = (CacheTLBInfo2D_AMD & 0xF000) >> 12;
	L3CacheLines = (CacheTLBInfo2D_AMD & 0xF00) >> 8;
	L3CacheLineSize = (CacheTLBInfo2D_AMD & 0xFF);
	printf("L2 Data TLB:       \t2 MByte/4 MByte pages, %s, %d entries\n", AssocWay[DTLBAssocA], DTLBEntriesA);
	printf("L2 Instruction TLB:\t2 MByte/4 MByte pages, %s, %d entries\n", AssocWay[ITLBAssocA], ITLBEntriesA);
	printf("L2 Data TLB:       \t4 KByte pages, %s, %d entries\n", AssocWay[DTLBAssocB], DTLBEntriesB);
	printf("L2 Instruction TLB:\t4 KByte pages, %s, %d entries\n", AssocWay[ITLBAssocB], ITLBEntriesB);
	printf("L2 Cache:          \t%d %s, %s, %d byte line size\n", L2CacheSize, K2 ? "KBytes" : "MBytes", AssocWay[L2CacheAssoc], L2CacheLineSize);
	printf("L3 Cache:          \t%d %s, %s, %d byte line size\n", L3CacheSize, K3 ? "KBytes" : "MBytes", AssocWay[L3CacheAssoc], L3CacheLineSize);
}
void Find1G_AMD()
{
	// 80000019H 
	unsigned int TLBInfoA_AMD;		// [11:0]: L1 instruction TLB number of entries for 1 GB pages
									// [15:12]: L1 instruction TLB associativity for 1 GB pages
									// [27:16]: L1 data TLB number of entries for 1 GB pages
									// [31:28]: L1 data TLB associativity for 1 GB pages
	unsigned int TLBInfoB_AMD;		// [11:0]: L2 instruction TLB number of entries for 1 GB pages
									// [15:12]: L2 instruction TLB associativity for 1 GB pages
									// [27:16]: L2 data TLB number of entries for 1 GB pages
									// [31:28]: L2 data TLB associativity for 1 GB pages

	unsigned int DTLBAssoc1, DTLBEntries1, ITLBAssoc1, ITLBEntries1;
	unsigned int DTLBAssoc2, DTLBEntries2, ITLBAssoc2, ITLBEntries2;
	char* AssocWay[16];
	AssocWay[0] = "disabled"; AssocWay[1] = "direct mapped"; AssocWay[2] = "2-way associative"; AssocWay[3] = "3-way associative";
	AssocWay[4] = "4-way associative"; AssocWay[5] = "6-way associative"; AssocWay[6] = "8-way associative"; AssocWay[7] = "null";
	AssocWay[8] = "16-way associative"; AssocWay[9] = "null"; AssocWay[10] = "32-way associative"; AssocWay[11] = "48-way associative";
	AssocWay[12] = "64-way associative"; AssocWay[13] = "96-way associative"; AssocWay[14] = "128-way associative"; AssocWay[15] = "fully associative";
	printf("\nCPU TLB Characteristics for 1GB pages:\n");
	printf("------------------------------------------------------------\n");
	__asm__(
		"movl $0x80000019, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		:"=a"(TLBInfoA_AMD), "=b"(TLBInfoB_AMD)
		:
		: "memory"
	);
	DTLBAssoc1 = (TLBInfoA_AMD & 0xF0000000) >> 28;
	DTLBEntries1 = (TLBInfoA_AMD & 0xFFF0000) >> 16;
	ITLBAssoc1 = (TLBInfoA_AMD & 0xF000) >> 12;
	ITLBEntries1 = (TLBInfoA_AMD & 0xFFF);
	DTLBAssoc2 = (TLBInfoB_AMD & 0xF0000000) >> 28;
	DTLBEntries2 = (TLBInfoB_AMD & 0xFFF0000) >> 16;
	ITLBAssoc2 = (TLBInfoB_AMD & 0xF000) >> 12;
	ITLBEntries2 = (TLBInfoB_AMD & 0xFFF);
	printf("L1 Data TLB:       \t1 GByte pages, %s, %d entries\n", AssocWay[DTLBAssoc1], DTLBEntries1);
	printf("L1 Instruction TLB:\t1 GByte pages, %s, %d entries\n", AssocWay[ITLBAssoc1], ITLBEntries1);
	printf("L2 Data TLB:       \t1 GByte pages, %s, %d entries\n", AssocWay[DTLBAssoc2], DTLBEntries2);
	printf("L2 Instruction TLB:\t1 GByte pages, %s, %d entries\n", AssocWay[ITLBAssoc2], ITLBEntries2);
}
void FindCache_AMD()
{
	// 8000001DH
	unsigned int CacheInfoA_AMD;	// [4:0]: Cache Type, 0:NULL;1:Data Cache;2:Instruction Cache;3:Unified Cache
									// [7:5]: Cache Level
									// [8]: Self Initializing cache level (does not need SW initialization)
									// [9]: Fully Associative cache
									// [25:14]: Maximum number of addressable IDs for logical processors sharing this cache
									// [31:26]: Maximum number of addressable IDs for processor cores in the physical package
	unsigned int CacheInfoB_AMD;	// [11:0]: System Coherency Line Size
									// [21:12]: Physical Line partitions
									// [31:22]: Ways of associativity
	unsigned int CacheInfoC_AMD;	// Number of Sets
	unsigned int CacheInfoD_AMD;	// [0]: Write-Back 0=VD/1=INVD
									// [1]: Cache Inclusiveness
									// [2]: Complex Cache Indexing
	unsigned int i = 0;
	unsigned int Level, LineSize, CacheWays, CacheSize;
	unsigned int K;
	char* CacheType[4];
	char* CacheAssoc[16];
	CacheType[0] = "NULL"; CacheType[1] = "Data Cache:       "; CacheType[2] = "Instruction Cache:"; CacheType[3] = "Unified Cache:    ";
	CacheAssoc[0] = "DIsabled"; CacheAssoc[1] = "1-way set"; CacheAssoc[2] = "2-way set"; CacheAssoc[4] = "4-way set";
	CacheAssoc[6] = "8-way set"; CacheAssoc[8] = "16-way set"; CacheAssoc[10] = "32-way set"; CacheAssoc[11] = "48-way set";
	CacheAssoc[12] = "64-way set"; CacheAssoc[13] = "96-way set"; CacheAssoc[14] = "128-way set"; CacheAssoc[15] = "fully";
	printf("\nCPU Cache Information:\n");
	printf("------------------------------------------------------------\n");
	while (1) {
		__asm__(
			"movl $0x8000001D, %%eax;"
			"movl %4, %%ecx;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			"movl %%edx, %3;"
			:"=a"(CacheInfoA_AMD), "=b"(CacheInfoB_AMD), "=c"(CacheInfoC_AMD), "=d"(CacheInfoD_AMD)
			: "c"(i)
			: "memory"
		);
		if (!(CacheInfoA_AMD & 0x1F)) break;
		Level = ((CacheInfoA_AMD & 0xE0) >> 5);
		LineSize = (CacheInfoB_AMD & 0xFFF) + 1;
		CacheWays = ((CacheInfoB_AMD & 0xFFC00000) >> 22) + 1;
		CacheSize = (CacheInfoC_AMD + 1) * LineSize * CacheWays;
		K = !(CacheSize >> 20);
		CacheSize = K ? (CacheSize >> 10) : (CacheSize >> 20);
		if (CacheInfoA_AMD & 0x200)
			printf("L%d %s\t%d %sytes, fully associative, %d byte line size\n",
				Level, CacheType[CacheInfoA_AMD & 0xF], CacheSize, K ? "KB" : "MB", LineSize);
		else
			printf("L%d %s\t%d %sytes, %d-way set associative, %d byte line size\n",
				Level, CacheType[CacheInfoA_AMD & 0xF], CacheSize, K ? "KB" : "MB", CacheWays, LineSize);
		i++;
	}
}

int rb_get_cpu_support()
{
	// Basic Information
	// 00H
	unsigned int BaseIndexMax=0;		// Maximum Input Value for Basic CPUID Information
	unsigned int VendorID[3] = { 0 };		// Intel: GenuineIntel
									// AMD: AuthcAMDenti 
	// 01H
	unsigned int ProcSign=0;			// [3:0]: Steping ID
									// [7:4]: Model
									// [11:8]: Family ID
									// [13:12]: Processor Type, 00:Early OEM; 01:OverDrive;10:MultiProcessor
									// [19:16]: Extended Model ID
									// [27:20]: Extended Family ID
	unsigned int FeaInfo0=0;			// [7:0]: Brand Index
									// [15:8]: CLFLUSH line size
									// [23:16]: Maximum number of addressable IDs for logical processors in this physical package
									// [31:24]: Initial APIC ID
	unsigned int FeaInfo1_Intel=0;	// [0]:SSE3, [1]:PCLMULQDQ, [2]:DTES64, [3]:MONITOR
									// [4]:DS-CPL, [5]: VMX, [6]:SMX, [7]:EIST
									// [8]:TM2, [9]:SSE3, [10]:CNXT-ID, [11]:SDBG
									// [12]:FMA, [13]:CMPXCHG16B, [14]:xTPR Update Control, [15]:PDCM
									// [16]:Reserved, [17]:PCID, [18]:DCA, [19]:SSE4.1
									// [20]:SSE4.2, [21]:x2APIC, [22]:MOVBE, [23]:POPCNT
									// [24]:TSC-Deadline, [25]:AESNI, [26]:XSAVE, [27]:OSXSAVE
									// [28]:AVX, [29]:F16C, [30]:RDRAND, [31]:Not Used           		
	unsigned int FeaInfo2_Intel=0;	// [0]:FPU, [1]:VME, [2]:DE, [3]:PSE
									// [4]:TSC, [5]:MSR, [6]:PAE, [7]:MCE
									// [8]:CX8, [9]:APIC, [10]:Reserved, [11]:SEP
									// [12]:MTRR, [13]:PGE, [14]:MCA, [15]:CMOV
									// [16]:PAT, [17]:PSE-36, [18]:PSN, [19]:CLFSH
									// [20]:Reserved, [21]:DS, [22]:ACPI, [23]:MMX
									// [24]:FXSR, [25]:SSE, [26]:SSE2, [27]:SS
									// [28]:HTT, [29]:TM, [30]:Reserved, [31]:PBE
	// 02H
	unsigned int CacheTLBInfo_Intel[4] = { 0 };	// Cache and TLB 
	// 03H
	unsigned int ProcSeriNum_Intel[2] = { 0 };	// Processor Serial Number
	// 04H : In FindCache_Intel
	// 07H
	unsigned int SubLeafMax_Intel = 0;	// Reports the maximum input value for supported leaf 7 sub-leaves
	unsigned int Subleaf1_Intel = 0;	// [0]:FSGSBASE, [1]:IA32_TSC_ADJUST MSR, [2]:SGX, [3]:BMI1
									// [4]:HLE, [5]:AVX2, [6]:FDP_EXCPTN_ONLY, [7]:SMEP
									// [8]:BMI2, [9]:Enhanced REP, [10]:INVPCID, [11]:RTM
									// [12]:RDT-M, [13]:Deprecates FPU CD&DS, [14]:MPX, [15]:RDT-A,
									// [16]:AVX512F, [17]:AVX512DQ, [18]:RDSEED, [19]:ADX
									// [20]:SMAP, [21]:AVX512_IFMA, [22]:RDPID, [23]:CLFLUSHOPT
									// [24]:CLWB, [25]:Intel Processor Trace, [26]:AVX512PF, [27]:AVX512ER
									// [28]:AVX512CD, [29]:SHA, [30]:AVX512BW, [31]:AVX512VL
	unsigned int SubLeafMax_AMD = 0;	// Reports the maximum input value for supported leaf 7 sub-leaves
	unsigned int SubleafB_AMD = 0;		// [0]:FSGSBASE, [3]:BMI1
									// [5]:AVX2, [7]:SMEP
									// [8]:BMI2, [10]:INVPCID 
									// [18]:RDSEED, [19]:ADX
									// [20]:SMAP, [22]:RDPID, [23]:CLFLUSHOPT
									// [24]:CLWB
									// [29]:SHA
	unsigned int SubleafC_AMD = 0;		// [2]:UMIP, [3]:PKU
									// [4]:OSPKE, [7]:CET_SS
									// [9]:VAES, [10]:VPCMULQDQ
	// 18H : IN FindCache_Intel
	// Extended Information
	// 80000000H
	unsigned int ExtendIndexMax = 0;	// Maximum Input Value for Extended Function CPUID Information
	// 80000001H
	unsigned int ExtSignC_Intel = 0;	// [0]: LAHF/SAHF
									// [5]: LZCNT
									// [8]: PREFETCHW
	unsigned int ExtSignD_Intel = 0;	// [11]: SYSCALL/SYSRET
									// [20]: Execute Disable Bit available
									// [26]: 1-GByte pages are available
									// [27]: RDTSCP and IA32_TSC_AUX are available
									// [29]: Intel 64 Architecture available
	unsigned int ExtSignA_AMD = 0;		// [3:0]: Steping ID
									// [7:4]: Model
									// [11:8]: Family ID
									// [19:15]: Extended Model ID
									// [27:20]: Extended Family ID
	unsigned int ExtSignB_AMD = 0;		// [15:0]: Brand ID
									// [31:28]: Package type
	unsigned int ExtSignC_AMD = 0;		// [0]:LAHF/SAHF, [1]:CMPLEGACY, [2]:SVM, [3]:EXTAPICSPACE
									// [4]:ALTMOVCR8, [5]:ABM, [6]:SSE4A, [7]:MISALIGNSSE
									// [8]:3DNOWPREFETCH, [9]:OSVW, [10]:IBS, [11]:XOP
									// [12]:SKINIT, [13]:WDT, [15]:LWP
									// [16]:FMA4, [17]:TCE
									// [21]:TBM, [22]:TOPOLOGYEXT, [23]:PERFCTREXTCORE
									// [24]:PERFCTREXTNB, [26]:DATABKPTEXT, [27]:PERFTSC
									// [28]:PERFCTREXTLLC, [29]:MONITORX, [30]:ADDRMASKEXT
	unsigned int ExtSignD_AMD = 0;		// [0]:FPU, [1]:VME, [2]:DE, [3]:PSE
									// [4]:TSC, [5]:MSR, [6]:PAE, [7]:MCE
									// [8]:CMPXCHG8B, [9]:APIC, [11]:SYSCALL/SYSRET
									// [12]:MTRR, [13]:PGE, [14]:MCA, [15]:CMOV
									// [16]:PAT, [17]:PSE36,
									// [20]:NX, [22]:MMXEXT, [23]:MMX,
									// [24]:FXSR, [25]:FFXSR, [26]:PAGE1GB, [27]:RDTSCP
									// [29]:LM, [30]:3DNOWEXT, [31]:3DNOW
	// 80000002H
	unsigned int ProcBrand[12] = { 0 };		// Processor Brand String
	// 80000005H In FindL1_AMD
	// 80000006H : In FindCache_Intel, FindL2L3_AMD	
	// 80000019H : In Find1G_AMD
	// 8000001DH : In FindCache_AMD				
	// Other
	char Vendor[13] = { 0 };
	char Brand[49] = { 0 };
	unsigned int Intel_AMD = 0;			// 1:Intel, 0:AMD

	__asm__(
		"movl $0, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		: "=a"(BaseIndexMax), "=b"(VendorID[0]), "=c"(VendorID[2]), "=d"(VendorID[1])
		:
		: "memory"
	);
	if (VendorID[0] == 0x68747541) 
	{
		__asm__(
			"movl $0, %%eax;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			"movl %%edx, %3;"
			: "=a"(BaseIndexMax), "=b"(VendorID[0]), "=c"(VendorID[2]), "=d"(VendorID[1])
			:
			: "memory"
		);
		Intel_AMD = 0;
	}
	else
	{
		Intel_AMD = 1;
	}


	memcpy(Vendor, VendorID, 12);
	Vendor[12] = '\0';
	__asm__(
		"movl $1, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(ProcSign), "=b"(FeaInfo0), "=c"(FeaInfo1_Intel), "=d"(FeaInfo2_Intel)
		:
		: "memory"
	);
	__asm__(
		"movl $2, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(CacheTLBInfo_Intel[0]), "=b"(CacheTLBInfo_Intel[1]), "=c"(CacheTLBInfo_Intel[2]), "=d"(CacheTLBInfo_Intel[3])
		:
		: "memory"
	);
	__asm__(
		"movl $3, %%eax;"
		"cpuid;"
		"movl %%ecx, %0;"
		"movl %%edx, %1;"
		:"=c"(ProcSeriNum_Intel[0]), "=d"(ProcSeriNum_Intel[1])
		:
		: "memory"
	);
	if (Intel_AMD)
		__asm__(
			"movl $7, %%eax;"
			"movl $0, %%ecx;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			:"=a"(SubLeafMax_Intel), "=b"(Subleaf1_Intel)
			:
			: "memory"
		);
	else
		__asm__(
			"movl $7, %%eax;"
			"movl $0, %%ecx;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			:"=a"(SubLeafMax_AMD), "=b"(SubleafB_AMD), "=c"(SubleafC_AMD)
			:
			: "memory"
		);
	__asm__(
		"movl $0x80000000, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		:"=a"(ExtendIndexMax)
		:
		: "memory"
	);
	if (Intel_AMD)
		__asm__(
			"movl $0x80000001, %%eax;"
			"cpuid;"
			"movl %%ecx, %0;"
			"movl %%edx, %1;"
			:"=c"(ExtSignC_Intel), "=d"(ExtSignD_Intel)
			:
			: "memory"
		);
	else
		__asm__(
			"movl $0x80000001, %%eax;"
			"cpuid;"
			"movl %%eax, %0;"
			"movl %%ebx, %1;"
			"movl %%ecx, %2;"
			"movl %%edx, %3;"
			:"=a"(ExtSignA_AMD), "=b"(ExtSignB_AMD), "=c"(ExtSignC_AMD), "=d"(ExtSignD_AMD)
			:
			: "memory"
		);
	__asm__(
		"movl $0x80000002, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(ProcBrand[0]), "=b"(ProcBrand[1]), "=c"(ProcBrand[2]), "=d"(ProcBrand[3])
		:
		: "memory"
	);
	__asm__(
		"movl $0x80000003, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(ProcBrand[4]), "=b"(ProcBrand[5]), "=c"(ProcBrand[6]), "=d"(ProcBrand[7])
		:
		: "memory"
	);
	__asm__(
		"movl $0x80000004, %%eax;"
		"cpuid;"
		"movl %%eax, %0;"
		"movl %%ebx, %1;"
		"movl %%ecx, %2;"
		"movl %%edx, %3;"
		:"=a"(ProcBrand[8]), "=b"(ProcBrand[9]), "=c"(ProcBrand[10]), "=d"(ProcBrand[11])
		:
		: "memory"
	);
#ifdef display_cpu_info
	memcpy(Brand, ProcBrand, 48);
	Brand[48] = '\0';

	printf("CPU Vendor:  %s\n", Vendor);
	printf("CPU Name:  %s\n", Brand);
	printf("CPU Basic Information: Family:%X  Model:%X  Stepping ID:%X\n", (ProcSign & 0x0F00) >> 8,
		(ProcSign & 0xF0) >> 4, ProcSign & 0xF);
	printf("CPU Extended Information: ExFamily:%X  ExModel:%X  Type:%X\n", (ProcSign & 0xFF00000) >> 20,
		(ProcSign & 0xF0000) >> 16, (ProcSign & 0x3000) >> 12);
	printf("CPU Brand Index: 0x%X\n", FeaInfo0 & 0xFF);
	printf("CPU CLFLUSH Line Size: 0x%X\n", (FeaInfo0 & 0xFF00) >> 8);
	printf("CPU Maximum Logical Processors: %d\n", (FeaInfo0 & 0xFF0000) >> 16);
	printf("CPU Initial APIC ID: 0x%X\n", (FeaInfo0 & 0xFF000000) >> 24);
	if (Intel_AMD) {
		printf("LAHF/SAHF:              \t%s\n", (ExtSignC_Intel & 0x1) ? "Yes" : "No");
		printf("LZCNT:                  \t%s\n", (ExtSignC_Intel & 0x10) ? "Yes" : "No");
		printf("PREFETCHW:              \t%s\n", (ExtSignC_Intel & 0x100) ? "Yes" : "No");
		printf("SYSCALL/SYSRET:         \t%s\n", (ExtSignD_Intel & 0x800) ? "Yes" : "No");
		printf("Execute Disable Bit:    \t%s\n", (ExtSignD_Intel & 0x100000) ? "Yes" : "No");
		printf("1-GByte pages:          \t%s\n", (ExtSignD_Intel & 0x4000000) ? "Yes" : "No");
		printf("RDTSCP and IA32_TSC_AUX:\t%s\n", (ExtSignD_Intel & 0x8000000) ? "Yes" : "No");
		printf("Intel 64 Architecture:  \t%s\n", (ExtSignD_Intel & 0x20000000) ? "Yes" : "No");
	}
	else {
		printf("CPU Brand ID: 0x%X\n", ExtSignB_AMD & 0xFFFF);
		printf("Package Type: 0x%X\n", (ExtSignB_AMD & 0xF0000000) >> 28);
	}
#endif
	/*if (Intel_AMD) {
		ListCacheTLB_Intel(CacheTLBInfo_Intel);
		if (BaseIndexMax >= 4)
			FindCache_Intel();
		if (BaseIndexMax >= 0x18)
			FindTLB_Intel();
		if (ExtendIndexMax >= 6)
			OtherCacheTLB_Intel();
		if (BaseIndexMax >= 3)
			printf("\nCPU Serial Number: %X%X\n", ProcSeriNum_Intel[0], ProcSeriNum_Intel[1]);
	}
	else {
		FindL1_AMD();
		FindL2L3_AMD();
		if (ExtendIndexMax >= 0x19)
			Find1G_AMD();
		if ((ExtSignC_AMD & 0x00400000) >> 22)
			FindCache_AMD();
	}*/
#ifdef display_cpu_info
	printf("\nCPU Instruction Set Support:\n");
	printf("------------------------------------------------------------\n");
#endif
	memset(&rb_cpu_su, 0, sizeof(rb_cpu_su));

	if (Intel_AMD) 
	{
		rb_cpu_su.supports_mmx =	(FeaInfo2_Intel & 0x00800000) >> 23;
		rb_cpu_su.supports_sse =	(FeaInfo2_Intel & 0x02000000) >> 25;
		rb_cpu_su.supports_sse2 =	(FeaInfo2_Intel & 0x04000000) >> 26;
		rb_cpu_su.supports_sse3 =	(FeaInfo1_Intel & 0x00000001) >> 0;
		rb_cpu_su.supports_ssse3 =	(FeaInfo1_Intel & 0x00000200) >> 9;
		rb_cpu_su.supports_sse4_1 = (FeaInfo1_Intel & 0x00080000) >> 19;
		rb_cpu_su.supports_sse4_2 = (FeaInfo1_Intel & 0x00100000) >> 20;
		rb_cpu_su.supports_avx =	(FeaInfo1_Intel & 0x10000000) >> 28;
		rb_cpu_su.supports_avx2 =	(Subleaf1_Intel & 0x00000020) >> 5;
		rb_cpu_su.supports_aes =	(FeaInfo1_Intel & 0x02000000) >> 25;

#ifdef display_cpu_info
		printf("support mmx %d\n", rb_cpu_su.supports_mmx);
		printf("support sse %d\n", rb_cpu_su.supports_sse);
		printf("support sse2 %d\n", rb_cpu_su.supports_sse2);
		printf("support sse3 %d\n", rb_cpu_su.supports_sse3);
		printf("support ssse3 %d\n", rb_cpu_su.supports_ssse3);
		printf("support sse4.1 %d\n", rb_cpu_su.supports_sse4_1);
		printf("support sse4.2 %d\n", rb_cpu_su.supports_sse4_2);
		printf("support avx %d\n", rb_cpu_su.supports_avx);
		printf("support avx2 %d\n", rb_cpu_su.supports_avx2);
		printf("support aes %d\n", rb_cpu_su.supports_aes);
		printf("\n");
#endif

#if 0
		printf("SSE3: %d         ", (FeaInfo1_Intel & 0x00000001) >> 0);
		printf("PCLMULQDQ: %d    ", (FeaInfo1_Intel & 0x00000002) >> 1);
		printf("DTES64: %d       ", (FeaInfo1_Intel & 0x00000004) >> 2);
		printf("MONITOR: %d\n", (FeaInfo1_Intel & 0x00000008) >> 3);
		printf("DS-CPL: %d       ", (FeaInfo1_Intel & 0x00000010) >> 4);
		printf("VMX: %d          ", (FeaInfo1_Intel & 0x00000020) >> 5);
		printf("SMX: %d          ", (FeaInfo1_Intel & 0x00000040) >> 6);
		printf("EIST: %d\n", (FeaInfo1_Intel & 0x00000080) >> 7);
		printf("TM2: %d          ", (FeaInfo1_Intel & 0x00000100) >> 8);
		printf("SSSE3: %d         ", (FeaInfo1_Intel & 0x00000200) >> 9);
		printf("CNXT-ID: %d      ", (FeaInfo1_Intel & 0x00000400) >> 10);
		printf("SDBG: %d\n", (FeaInfo1_Intel & 0x00000800) >> 11);
		printf("FMA: %d          ", (FeaInfo1_Intel & 0x00001000) >> 12);
		printf("CMPXCHG16B: %d   ", (FeaInfo1_Intel & 0x00002000) >> 13);
		printf("xTPR: %d         ", (FeaInfo1_Intel & 0x00004000) >> 14);
		printf("PDCM: %d\n", (FeaInfo1_Intel & 0x00008000) >> 15);
		printf("PCID: %d         ", (FeaInfo1_Intel & 0x00020000) >> 17);
		printf("DCA: %d          ", (FeaInfo1_Intel & 0x00040000) >> 18);
		printf("SSE4.1: %d       ", (FeaInfo1_Intel & 0x00080000) >> 19);
		printf("SSE4.2: %d\n", (FeaInfo1_Intel & 0x00100000) >> 20);
		printf("x2APIC: %d       ", (FeaInfo1_Intel & 0x00200000) >> 21);
		printf("MOVBE: %d        ", (FeaInfo1_Intel & 0x00400000) >> 22);
		printf("POPCNT: %d       ", (FeaInfo1_Intel & 0x00800000) >> 23);
		printf("TSC-D: %d\n", (FeaInfo1_Intel & 0x01000000) >> 24);
		printf("AESNI: %d        ", (FeaInfo1_Intel & 0x02000000) >> 25);
		printf("XSAVE: %d        ", (FeaInfo1_Intel & 0x04000000) >> 26);
		printf("OSXSAVE: %d      ", (FeaInfo1_Intel & 0x08000000) >> 27);
		printf("AVX: %d\n", (FeaInfo1_Intel & 0x10000000) >> 28);
		printf("F16C: %d         ", (FeaInfo1_Intel & 0x20000000) >> 29);
		printf("RDRAND: %d       ", (FeaInfo1_Intel & 0x40000000) >> 30);
		printf("FPU: %d          ", (FeaInfo2_Intel & 0x00000001) >> 0);
		printf("VME: %d\n", (FeaInfo2_Intel & 0x00000002) >> 1);
		printf("DE: %d           ", (FeaInfo2_Intel & 0x00000004) >> 2);
		printf("PSE: %d          ", (FeaInfo2_Intel & 0x00000008) >> 3);
		printf("TSC: %d          ", (FeaInfo2_Intel & 0x00000010) >> 4);
		printf("MSR: %d\n", (FeaInfo2_Intel & 0x00000020) >> 5);
		printf("PAE: %d          ", (FeaInfo2_Intel & 0x00000040) >> 6);
		printf("MCE: %d          ", (FeaInfo2_Intel & 0x00000080) >> 7);
		printf("CX8: %d          ", (FeaInfo2_Intel & 0x00000100) >> 8);
		printf("APIC: %d\n", (FeaInfo2_Intel & 0x00000200) >> 9);
		printf("SEP: %d          ", (FeaInfo2_Intel & 0x00000800) >> 11);
		printf("MTRR: %d         ", (FeaInfo2_Intel & 0x00001000) >> 12);
		printf("PGE: %d          ", (FeaInfo2_Intel & 0x00002000) >> 13);
		printf("MCA: %d\n", (FeaInfo2_Intel & 0x00004000) >> 14);
		printf("CMOV: %d         ", (FeaInfo2_Intel & 0x00008000) >> 15);
		printf("PAT: %d          ", (FeaInfo2_Intel & 0x00010000) >> 16);
		printf("PSE-36: %d       ", (FeaInfo2_Intel & 0x00020000) >> 17);
		printf("PSN: %d\n", (FeaInfo2_Intel & 0x00040000) >> 18);
		printf("CLFSN: %d        ", (FeaInfo2_Intel & 0x00080000) >> 19);
		printf("DS: %d           ", (FeaInfo2_Intel & 0x00200000) >> 21);
		printf("ACPI: %d         ", (FeaInfo2_Intel & 0x00400000) >> 22);
		printf("MMX: %d\n", (FeaInfo2_Intel & 0x00800000) >> 23);
		printf("FXSR: %d         ", (FeaInfo2_Intel & 0x01000000) >> 24);
		printf("SSE: %d          ", (FeaInfo2_Intel & 0x02000000) >> 25);
		printf("SSE2: %d         ", (FeaInfo2_Intel & 0x04000000) >> 26);
		printf("SS: %d\n", (FeaInfo2_Intel & 0x08000000) >> 27);
		printf("HTT: %d          ", (FeaInfo2_Intel & 0x10000000) >> 28);
		printf("TM: %d           ", (FeaInfo2_Intel & 0x20000000) >> 29);
		printf("PBE: %d          ", (FeaInfo2_Intel & 0x80000000) >> 31);
		printf("FSGSBASE: %d\n", (Subleaf1_Intel & 0x00000001) >> 0);
		printf("IA32_MSR: %d     ", (Subleaf1_Intel & 0x00000002) >> 1);
		printf("SGX: %d          ", (Subleaf1_Intel & 0x00000004) >> 2);
		printf("BMI1: %d         ", (Subleaf1_Intel & 0x00000008) >> 3);
		printf("HLE: %d\n", (Subleaf1_Intel & 0x00000010) >> 4);
		printf("AVX2: %d         ", (Subleaf1_Intel & 0x00000020) >> 5);
		printf("FDP_ONLY: %d     ", (Subleaf1_Intel & 0x00000040) >> 6);
		printf("SMEP: %d         ", (Subleaf1_Intel & 0x00000080) >> 7);
		printf("BMI2: %d\n", (Subleaf1_Intel & 0x00000100) >> 8);
		printf("EnhanREP: %d     ", (Subleaf1_Intel & 0x00000200) >> 9);
		printf("INVPCID: %d      ", (Subleaf1_Intel & 0x00000400) >> 10);
		printf("RTM: %d          ", (Subleaf1_Intel & 0x00000800) >> 11);
		printf("RDT-M: %d\n", (Subleaf1_Intel & 0x00001000) >> 12);
		printf("FPU CD&DS: %d    ", (Subleaf1_Intel & 0x00002000) >> 13);
		printf("MPX: %d          ", (Subleaf1_Intel & 0x00004000) >> 14);
		printf("RDT-A: %d        ", (Subleaf1_Intel & 0x00008000) >> 15);
		printf("AVX512F: %d\n", (Subleaf1_Intel & 0x00010000) >> 16);
		printf("AVX512DQ: %d     ", (Subleaf1_Intel & 0x00020000) >> 17);
		printf("RDSEED: %d       ", (Subleaf1_Intel & 0x00040000) >> 18);
		printf("ADX: %d          ", (Subleaf1_Intel & 0x00080000) >> 19);
		printf("SMAP: %d\n", (Subleaf1_Intel & 0x00100000) >> 20);
		printf("AVX512_IFMA: %d  ", (Subleaf1_Intel & 0x00200000) >> 21);
		printf("RDPID: %d        ", (Subleaf1_Intel & 0x00200000) >> 22);
		printf("ACLFLUSHOPT: %d  ", (Subleaf1_Intel & 0x00800000) >> 23);
		printf("CLWB: %d\n", (Subleaf1_Intel & 0x01000000) >> 24);
		printf("PTrace: %d       ", (Subleaf1_Intel & 0x02000000) >> 25);
		printf("AVX512PF: %d     ", (Subleaf1_Intel & 0x04000000) >> 26);
		printf("AVX512ER: %d     ", (Subleaf1_Intel & 0x08000000) >> 27);
		printf("AVX512CD: %d\n", (Subleaf1_Intel & 0x10000000) >> 28);
		printf("SHA: %d          ", (Subleaf1_Intel & 0x20000000) >> 29);
		printf("AVX512BW: %d     ", (Subleaf1_Intel & 0x40000000) >> 30);
		printf("AVX512VL: %d     ", (Subleaf1_Intel & 0x80000000) >> 31);
#endif
	}
	else 
	{
		rb_cpu_su.supports_mmx		=	(ExtSignD_AMD & 0x00800000) >> 23;
		rb_cpu_su.supports_sse		=	0;
		rb_cpu_su.supports_sse2		=	0;
		rb_cpu_su.supports_sse3		=	0;
		rb_cpu_su.supports_ssse3	=	0;
		rb_cpu_su.supports_sse4_1	=	0;
		rb_cpu_su.supports_sse4_2	=	0;
		rb_cpu_su.supports_avx		=	0;
		rb_cpu_su.supports_avx2		=	(SubleafB_AMD & 0x00000020) >> 5;
		rb_cpu_su.supports_aes		=	0;

#ifdef display_cpu_info
		printf("support mmx %d\n", rb_cpu_su.supports_mmx);
		printf("support sse %d\n", rb_cpu_su.supports_sse);
		printf("support sse2 %d\n", rb_cpu_su.supports_sse2);
		printf("support sse3 %d\n", rb_cpu_su.supports_sse3);
		printf("support ssse3 %d\n", rb_cpu_su.supports_ssse3);
		printf("support sse4.1 %d\n", rb_cpu_su.supports_sse4_1);
		printf("support sse4.2 %d\n", rb_cpu_su.supports_sse4_2);
		printf("support avx %d\n", rb_cpu_su.supports_avx);
		printf("support avx2 %d\n", rb_cpu_su.supports_avx2);
		printf("support aes %d\n", rb_cpu_su.supports_aes);
		printf("\n");
#endif
#if 0
		printf("FSGSBASE: %d         ", (SubleafB_AMD & 0x00000001) >> 0);
		printf("BMI1: %d             ", (SubleafB_AMD & 0x00000004) >> 3);
		printf("AVX2: %d             ", (SubleafB_AMD & 0x00000020) >> 5);
		printf("SMEP: %d\n", (SubleafB_AMD & 0x00000080) >> 7);
		printf("BMI2: %d             ", (SubleafB_AMD & 0x00000100) >> 8);
		printf("INVPCID: %d          ", (SubleafB_AMD & 0x00000400) >> 10);
		printf("RDSEED: %d           ", (SubleafB_AMD & 0x00040000) >> 18);
		printf("ADX: %d\n", (SubleafB_AMD & 0x00080000) >> 19);
		printf("SMAP: %d             ", (SubleafB_AMD & 0x00100000) >> 20);
		printf("RDPID: %d            ", (SubleafB_AMD & 0x00400000) >> 22);
		printf("CLFLUSHOPT: %d       ", (SubleafB_AMD & 0x00800000) >> 23);
		printf("CLWB: %d\n", (SubleafB_AMD & 0x01000000) >> 24);
		printf("SHA: %d              ", (SubleafB_AMD & 0x20000000) >> 29);
		printf("UMIP: %d             ", (SubleafC_AMD & 0x00000002) >> 2);
		printf("PKU: %d              ", (SubleafC_AMD & 0x00000004) >> 3);
		printf("OSPKE: %d\n", (SubleafC_AMD & 0x00000008) >> 4);
		printf("CET_SS: %d           ", (SubleafC_AMD & 0x00000080) >> 7);
		printf("VAES: %d             ", (SubleafC_AMD & 0x00000200) >> 9);
		printf("VPCMULQDQ: %d        ", (SubleafC_AMD & 0x00000400) >> 10);
		printf("LAHF/SAHF: %d\n", (ExtSignC_AMD & 0x00000001) >> 0);
		printf("CMPLEGACY: %d        ", (ExtSignC_AMD & 0x00000002) >> 1);
		printf("SVM: %d              ", (ExtSignC_AMD & 0x00000003) >> 2);
		printf("EXTAPICSPACE: %d     ", (ExtSignC_AMD & 0x00000008) >> 3);
		printf("ALTMOVCR8: %d\n", (ExtSignC_AMD & 0x00000010) >> 4);
		printf("ABM: %d              ", (ExtSignC_AMD & 0x00000020) >> 5);
		printf("SSE4A: %d            ", (ExtSignC_AMD & 0x00000040) >> 6);
		printf("MISALIGNSSE: %d      ", (ExtSignC_AMD & 0x00000080) >> 7);
		printf("3DNOWPREFETCH: %d\n", (ExtSignC_AMD & 0x00000100) >> 8);
		printf("OSVW: %d             ", (ExtSignC_AMD & 0x00000200) >> 9);
		printf("IBS: %d              ", (ExtSignC_AMD & 0x00000400) >> 10);
		printf("XOP: %d              ", (ExtSignC_AMD & 0x00000800) >> 11);
		printf("SKINIT: %d\n", (ExtSignC_AMD & 0x00001000) >> 12);
		printf("WDT: %d              ", (ExtSignC_AMD & 0x00002000) >> 13);
		printf("LWP: %d              ", (ExtSignC_AMD & 0x00008000) >> 15);
		printf("FMA4: %d             ", (ExtSignC_AMD & 0x00010000) >> 16);
		printf("TCE: %d\n", (ExtSignC_AMD & 0x00020000) >> 17);
		printf("TBM: %d              ", (ExtSignC_AMD & 0x00200000) >> 21);
		printf("TOPOLOGYEXT: %d      ", (ExtSignC_AMD & 0x00400000) >> 22);
		printf("PERFCTREXTCORE: %d   ", (ExtSignC_AMD & 0x00800000) >> 23);
		printf("PERFCTREXTNB: %d\n", (ExtSignC_AMD & 0x01000000) >> 24);
		printf("DATABKPTEXT: %d      ", (ExtSignC_AMD & 0x04000000) >> 26);
		printf("PERFTSC: %d          ", (ExtSignC_AMD & 0x08000000) >> 27);
		printf("PERFCTREXTLLC: %d    ", (ExtSignC_AMD & 0x10000000) >> 28);
		printf("MONITORX: %d\n", (ExtSignC_AMD & 0x20000000) >> 29);
		printf("ADDRMASKEXT: %d      ", (ExtSignC_AMD & 0x40000000) >> 30);
		printf("FPU: %d              ", (ExtSignD_AMD & 0x00000001) >> 0);
		printf("VME: %d              ", (ExtSignD_AMD & 0x00000002) >> 1);
		printf("DE: %d\n", (ExtSignD_AMD & 0x00000004) >> 2);
		printf("PSE: %d              ", (ExtSignD_AMD & 0x00000008) >> 3);
		printf("TSC: %d              ", (ExtSignD_AMD & 0x00000010) >> 4);
		printf("MSR: %d              ", (ExtSignD_AMD & 0x00000020) >> 5);
		printf("PAE: %d\n", (ExtSignD_AMD & 0x00000040) >> 6);
		printf("MCE: %d              ", (ExtSignD_AMD & 0x00000080) >> 7);
		printf("CMPXCHG8B: %d        ", (ExtSignD_AMD & 0x00000100) >> 8);
		printf("APIC: %d             ", (ExtSignD_AMD & 0x00000200) >> 9);
		printf("SYSCALL/SYSRET: %d\n", (ExtSignD_AMD & 0x00000800) >> 11);
		printf("MTRR: %d             ", (ExtSignD_AMD & 0x00001000) >> 12);
		printf("PGE: %d              ", (ExtSignD_AMD & 0x00002000) >> 13);
		printf("MCA: %d              ", (ExtSignD_AMD & 0x00004000) >> 14);
		printf("CMOV: %d\n", (ExtSignD_AMD & 0x00008000) >> 15);
		printf("PAT: %d              ", (ExtSignD_AMD & 0x00010000) >> 16);
		printf("PSE36: %d            ", (ExtSignD_AMD & 0x00020000) >> 17);
		printf("NX: %d               ", (ExtSignD_AMD & 0x00100000) >> 20);
		printf("MMXEXT: %d\n", (ExtSignD_AMD & 0x00400000) >> 22);
		printf("MMX: %d              ", (ExtSignD_AMD & 0x00800000) >> 23);
		printf("FXSR: %d             ", (ExtSignD_AMD & 0x01000000) >> 24);
		printf("FFXSR: %d            ", (ExtSignD_AMD & 0x02000000) >> 25);
		printf("PAGE1GB: %d\n", (ExtSignD_AMD & 0x04000000) >> 26);
		printf("RDTSCP: %d           ", (ExtSignD_AMD & 0x08000000) >> 27);
		printf("LM: %d               ", (ExtSignD_AMD & 0x20000000) >> 29);
		printf("3DNOWEXT: %d         ", (ExtSignD_AMD & 0x40000000) >> 30);
		printf("3DNOW: %d\n", (ExtSignD_AMD & 0x80000000) >> 31);
#endif
	}

#if 0
	printf("support mmx %d\n", rb_cpu_su.supports_mmx);
	printf("support sse %d\n", rb_cpu_su.supports_sse);
	printf("support sse2 %d\n", rb_cpu_su.supports_sse2);
	printf("support sse3 %d\n", rb_cpu_su.supports_sse3);
	printf("support ssse3 %d\n", rb_cpu_su.supports_ssse3);
	printf("support sse4.1 %d\n", rb_cpu_su.supports_sse4_1);
	printf("support sse4.2 %d\n", rb_cpu_su.supports_sse4_2);
	printf("support avx %d\n", rb_cpu_su.supports_avx);
	printf("support avx2 %d\n", rb_cpu_su.supports_avx2);
	printf("support aes %d\n", rb_cpu_su.supports_aes);
	printf("\n");
#endif
	return 0;
}
#endif



#if defined(WIN32_VERSION) && defined(RB_SIMD_X86)
#include <windows.h>
#include <conio.h>
#include <tchar.h>

#if _MSC_VER >=1400	
#include <intrin.h>	
#else
#include <emmintrin.h>	// MMX, SSE, SSE2
#endif



#include <stdio.h>


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

// CPUIDFIELD
typedef int32_t CPUIDFIELD;

#define  CPUIDFIELD_MASK_POS	0x0000001F	
#define  CPUIDFIELD_MASK_LEN	0x000003E0	
#define  CPUIDFIELD_MASK_REG	0x00000C00	
#define  CPUIDFIELD_MASK_FIDSUB	0x000FF000	
#define  CPUIDFIELD_MASK_FID	0xFFF00000	

#define CPUIDFIELD_SHIFT_POS	0
#define CPUIDFIELD_SHIFT_LEN	5
#define CPUIDFIELD_SHIFT_REG	10
#define CPUIDFIELD_SHIFT_FIDSUB	12
#define CPUIDFIELD_SHIFT_FID	20

#define CPUIDFIELD_MAKE(fid,fidsub,reg,pos,len)	(((fid)&0xF0000000) \
	| ((fid)<<CPUIDFIELD_SHIFT_FID & 0x0FF00000) \
	| ((fidsub)<<CPUIDFIELD_SHIFT_FIDSUB & CPUIDFIELD_MASK_FIDSUB) \
	| ((reg)<<CPUIDFIELD_SHIFT_REG & CPUIDFIELD_MASK_REG) \
	| ((pos)<<CPUIDFIELD_SHIFT_POS & CPUIDFIELD_MASK_POS) \
	| (((len)-1)<<CPUIDFIELD_SHIFT_LEN & CPUIDFIELD_MASK_LEN) \
	)
#define CPUIDFIELD_FID(cpuidfield)	( ((cpuidfield)&0xF0000000) | (((cpuidfield) & 0x0FF00000)>>CPUIDFIELD_SHIFT_FID) )
#define CPUIDFIELD_FIDSUB(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_FIDSUB)>>CPUIDFIELD_SHIFT_FIDSUB )
#define CPUIDFIELD_REG(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_REG)>>CPUIDFIELD_SHIFT_REG )
#define CPUIDFIELD_POS(cpuidfield)	( ((cpuidfield) & CPUIDFIELD_MASK_POS)>>CPUIDFIELD_SHIFT_POS )
#define CPUIDFIELD_LEN(cpuidfield)	( (((cpuidfield) & CPUIDFIELD_MASK_LEN)>>CPUIDFIELD_SHIFT_LEN) + 1 )

// È¡µÃÎ»Óò
#ifndef __GETBITS32
#define __GETBITS32(src,pos,len)	( ((src)>>(pos)) & (((uint32_t)-1)>>(32-len)) )
#endif


#define CPUF_SSE4A	CPUIDFIELD_MAKE(0x80000001,0,2,6,1)
#define CPUF_AES	CPUIDFIELD_MAKE(1,0,2,25,1)
#define CPUF_PCLMULQDQ	CPUIDFIELD_MAKE(1,0,2,1,1)

#define CPUF_AVX	CPUIDFIELD_MAKE(1,0,2,28,1)
#define CPUF_AVX2	CPUIDFIELD_MAKE(7,0,1, 5,1)
#define CPUF_OSXSAVE	CPUIDFIELD_MAKE(1,0,2,27,1)
#define CPUF_XFeatureSupportedMaskLo	CPUIDFIELD_MAKE(0xD,0,0,0,32)
#define CPUF_F16C	CPUIDFIELD_MAKE(1,0,2,29,1)
#define CPUF_FMA	CPUIDFIELD_MAKE(1,0,2,12,1)
#define CPUF_FMA4	CPUIDFIELD_MAKE(0x80000001,0,2,16,1)
#define CPUF_XOP	CPUIDFIELD_MAKE(0x80000001,0,2,11,1)

#define SIMD_SSE_NONE	0	
#define SIMD_SSE_1	1	
#define SIMD_SSE_2	2	
#define SIMD_SSE_3	3	
#define SIMD_SSE_3S	4	
#define SIMD_SSE_41	5	
#define SIMD_SSE_42	6	

const char* rb_simd_sse_names[] = {
	"None",
	"SSE",
	"SSE2",
	"SSE3",
	"SSSE3",
	"SSE4.1",
	"SSE4.2",
};

#define SIMD_AVX_NONE	0	
#define SIMD_AVX_1	1	
#define SIMD_AVX_2	2	

const char* rb_simd_avx_names[] = {
	"None",
	"AVX",
	"AVX2"
};



char szBuf[64];
int32_t dwBuf[4];

#if defined(_WIN64)

#else
#if _MSC_VER < 1600	
void __cpuidex(int32_t CPUInfo[4], int32_t InfoType, int32_t ECXValue)
{
	if (NULL == CPUInfo)	return;
#ifdef 
	_asm {
	
		mov edi, CPUInfo;	
		mov eax, InfoType;
		mov ecx, ECXValue;
	
		cpuid;
	
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
#else
	__asm__ __volatile__(
		"MOV r0,%[input1] \n"
		"MOV r1,%[input2] \n"
		"ADD %[output1], r0, r1"
		: [output1] "+r"(c)
		: [input1] "+r"(a), [input2]"+r"(b)
		: r0, r1);
#endif
}
#endif

#if _MSC_VER < 1400	// 
void __cpuid(int32_t CPUInfo[4], int32_t InfoType)
{
	__cpuidex(CPUInfo, InfoType, 0);
}
#endif	

#endif	


uint32_t	rb_getcpuidfield_buf(const int32_t dwBuf[4], CPUIDFIELD cpuf)
{
	return __GETBITS32(dwBuf[CPUIDFIELD_REG(cpuf)], CPUIDFIELD_POS(cpuf), CPUIDFIELD_LEN(cpuf));
}


uint32_t	rb_getcpuidfield(CPUIDFIELD cpuf)
{
	int32_t dwBuf[4];
	__cpuidex(dwBuf, CPUIDFIELD_FID(cpuf), CPUIDFIELD_FIDSUB(cpuf));
	return rb_getcpuidfield_buf(dwBuf, cpuf);
}


int rb_cpu_getvendor(char* pvendor)
{
	int32_t dwBuf[4];
	if (NULL == pvendor)	return 0;

	__cpuid(dwBuf, 0);

	*(int32_t*)&pvendor[0] = dwBuf[1];	
	*(int32_t*)&pvendor[4] = dwBuf[3];	
	*(int32_t*)&pvendor[8] = dwBuf[2];	
	pvendor[12] = '\0';
	return 12;
}


int rb_cpu_getbrand(char* pbrand)
{
	int32_t dwBuf[4];
	if (NULL == pbrand)	return 0;
	
	__cpuid(dwBuf, 0x80000000);
	if (dwBuf[0] < 0x80000004)	return 0;
	
	__cpuid((int32_t*)&pbrand[0], 0x80000002);	
	__cpuid((int32_t*)&pbrand[16], 0x80000003);	
	__cpuid((int32_t*)&pbrand[32], 0x80000004);	
	pbrand[48] = '\0';
	return 48;
}



unsigned int	rb_simd_mmx(unsigned int* phwmmx)
{
	const int32_t	BIT_D_MMX = 0x00800000;
	unsigned int	rt = 0;	// result
	int32_t dwBuf[4];


	__cpuid(dwBuf, 1);	
	if (dwBuf[3] & BIT_D_MMX)	
		rt = 1;

	if (NULL != phwmmx)	*phwmmx = rt;


	if (rt)
	{
#if defined(_WIN64)
	
		rt = 0;
#else
		__try
		{
			_mm_empty();	
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			rt = 0;
		}
#endif	
	}

	return rt;
}


int	rb_simd_sse_level(int* phwsse)
{
	const int32_t	BIT_D_SSE = 0x02000000;	// bit 25
	const int32_t	BIT_D_SSE2 = 0x04000000;	// bit 26
	const int32_t	BIT_C_SSE3 = 0x00000001;	// bit 0
	const int32_t	BIT_C_SSSE3 = 0x00000200;	// bit 9
	const int32_t	BIT_C_SSE41 = 0x00080000;	// bit 19
	const int32_t	BIT_C_SSE42 = 0x00100000;	// bit 20
	int	rt = SIMD_SSE_NONE;	// result
	int32_t dwBuf[4];

	// check processor support
	__cpuid(dwBuf, 1);	// Function 1: Feature Information
	if (dwBuf[3] & BIT_D_SSE)
	{
		rt = SIMD_SSE_1;
		if (dwBuf[3] & BIT_D_SSE2)
		{
			rt = SIMD_SSE_2;
			if (dwBuf[2] & BIT_C_SSE3)
			{
				rt = SIMD_SSE_3;
				if (dwBuf[2] & BIT_C_SSSE3)
				{
					rt = SIMD_SSE_3S;
					if (dwBuf[2] & BIT_C_SSE41)
					{
						rt = SIMD_SSE_41;
						if (dwBuf[2] & BIT_C_SSE42)
						{
							rt = SIMD_SSE_42;
						}
					}
				}
			}
		}
	}
	if (NULL != phwsse)	*phwsse = rt;

	// check OS support
	__try
	{
		__m128 xmm1 = _mm_setzero_ps();	
		if (0 != *(int*)&xmm1)	rt = SIMD_SSE_NONE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		rt = SIMD_SSE_NONE;
	}

	return rt;
}


int	rb_simd_avx_level(int* phwavx)
{
	int	rt = SIMD_AVX_NONE;	// result

	// check processor support
	if (0 != rb_getcpuidfield(CPUF_AVX))
	{
		rt = SIMD_AVX_1;
		if (0 != rb_getcpuidfield(CPUF_AVX2))
		{
			rt = SIMD_AVX_2;
		}
	}
	if (NULL != phwavx)	*phwavx = rt;

	// check OS support
	if (0 != rb_getcpuidfield(CPUF_OSXSAVE))	
	{
		uint32_t n = rb_getcpuidfield(CPUF_XFeatureSupportedMaskLo);	
		if (6 == (n & 6))	
		{
			return rt;
		}
	}
	return SIMD_AVX_NONE;
}


#ifdef RB_SIMD_X86

int rb_get_cpu_support()
{
	int i;
	memset(&rb_cpu_su, 0, sizeof(rb_cpu_su));

	rb_cpu_getvendor(szBuf);
#ifdef display_cpu_info
	printf("CPU Vendor:\t%s\n", szBuf);
#endif

	rb_cpu_getbrand(szBuf);
#ifdef display_cpu_info
	printf("CPU Name:\t%s\n", szBuf);
#endif

	unsigned int bhwmmx;	
	unsigned int bmmx;	
	bmmx = rb_simd_mmx(&bhwmmx);
#ifdef display_cpu_info
	printf("MMX: %d\t// hw: %d\n", bmmx, bhwmmx);
#endif
	if (bmmx == bhwmmx)
		rb_cpu_su.supports_mmx = 1;

	int	nhwsse;	
	int	nsse;	
	nsse = rb_simd_sse_level(&nhwsse);
#ifdef display_cpu_info
	printf("SSE: %d\t// hw: %d\n", nsse, nhwsse);
#endif
	for (i = 1; i < sizeof(rb_simd_sse_names) / sizeof(rb_simd_sse_names[0]); ++i)
	{
		if (nhwsse >= i)
		{
			if (i == 1)
				rb_cpu_su.supports_sse = 1;
			else
				if (i == 2)
					rb_cpu_su.supports_sse2 = 1;
				else
					if (i == 3)
						rb_cpu_su.supports_sse3 = 1;
					else
						if (i == 4)
							rb_cpu_su.supports_ssse3 = 1;
						else
							if (i == 5)
								rb_cpu_su.supports_sse4_1 = 1;
							else
								if (i == 6)
									rb_cpu_su.supports_sse4_2 = 1;
#ifdef display_cpu_info
			printf("\t%s\n", rb_simd_sse_names[i]);
#endif
		}
		
	}
#ifdef display_cpu_info
	
	printf("SSE4A: %d\n", rb_getcpuidfield(CPUF_SSE4A));
	printf("AES: %d\n", rb_getcpuidfield(CPUF_AES));
	printf("PCLMULQDQ: %d\n", rb_getcpuidfield(CPUF_PCLMULQDQ));
#endif

	rb_cpu_su.supports_aes = rb_getcpuidfield(CPUF_AES);


	int	nhwavx;	
	int	navx;	
	navx = rb_simd_avx_level(&nhwavx);
#ifdef display_cpu_info
	printf("AVX: %d\t// hw: %d\n", navx, nhwavx);
#endif
	for (i = 1; i < sizeof(rb_simd_avx_names) / sizeof(rb_simd_avx_names[0]); ++i)
	{
		if (nhwavx >= i)
		{
			if (i == 1)
				rb_cpu_su.supports_avx = 1;
			else
				if (i == 2)
					rb_cpu_su.supports_avx2 = 1;
				else
					if (i == 3)
						rb_cpu_su.supports_avx512 = 1;
#ifdef display_cpu_info
			printf("\t%s\n", rb_simd_avx_names[i]);
#endif
		}
	}
#ifdef display_cpu_info

	printf("F16C: %d\n", rb_getcpuidfield(CPUF_F16C));
	printf("FMA: %d\n", rb_getcpuidfield(CPUF_FMA));
	printf("FMA4: %d\n", rb_getcpuidfield(CPUF_FMA4));
	printf("XOP: %d\n", rb_getcpuidfield(CPUF_XOP));
#endif

	return 0;
}

#endif

#endif