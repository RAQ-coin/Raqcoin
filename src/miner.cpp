// Copyright (c) 2018 The Abcmint developers
// Copyright (c) 2025 The Raqcoin developers


#include "init.h"
#include "miner.h"
#include "wallet.h"
#include <bitset>

using namespace std;
using namespace boost;

#ifdef USE_GPU
#include "gpumining.h"
#endif

// Vec4 typedef is now defined in miner.h for all architectures

#define THRESHOLD 9
vector_t init_vector(int n_rows) {
    return (vector_t)calloc(n_rows, sizeof(int));
}

pck_vector_t pack(int n, const vector_t v) {
    pck_vector_t r = 0;
    assert((unsigned int) n <= 8*sizeof(pck_vector_t));

    for(int i=n-1; i>=0; i--) {
        r = r << 1ll;
        r |= v[i] & 0x0001ll;
    }
    return r;
}

uint64_t to_gray(uint64_t i) {
    return (i ^ (i >> 1ll));
}

#ifdef __i386
uint64_t rdtsc() {
    uint64_t x;
    __asm__ volatile ("rdtsc" : "=A" (x));
    return x;
}
#elif defined(__x86_64__)
uint64_t rdtsc() {
    uint64_t a, d;
    __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
    return (d<<32) | a;
}
#elif defined(__aarch64__) || defined(__arm64__)
// ARM architecture - use system timer
uint64_t rdtsc() {
    uint64_t val;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r" (val));
    return val;
}
#else
// Fallback for other architectures - use clock
uint64_t rdtsc() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
#endif

pck_vector_t packed_eval(LUT_t LUT, int n, int d, pck_vector_t F[], uint64_t i) {
    if (d == 2) {
        return packed_eval_deg_2(LUT, n, F, i);
    } else {
        printf("degree-%d naive evaluation is not yet implemented...\n", d);
        assert(0);
        return 0;
    }
}

// this ought to be optimized, unrolled, etc.
void BFS(pck_vector_t *A, int start, int n,CBlockIndex* pindexPrev) {
    for(int i=0; i<n; i++) {
        if (pindexPrev != pindexBest) {
            return;
        }
        int Sz = 1 << i;
        int Pos = start;

        while(Pos < start + (1 << n)) {
            for(int j=0; j<Sz; j++)
            A[Pos + Sz + j] ^= A[Pos + j];
            Pos += 2*Sz;
            if (pindexPrev != pindexBest) {
                return;
            }
        }
    }
}

void hybrid_DFS_BFS(pck_vector_t *A, int from, int to, CBlockIndex* pindexPrev) {
    if (from >= to-1) return;
    if (pindexPrev != pindexBest) {
        return;
    }
    int center = (to+from)/2;
    int half_length = (to-from)/2;

    if (half_length == (1 << (THRESHOLD-1)))
       BFS(A, from, THRESHOLD,pindexPrev);
    else {
        hybrid_DFS_BFS(A, from, center, pindexPrev);
        hybrid_DFS_BFS(A, center, to,pindexPrev);
        for(int i=0; i<half_length; i++)
            A[center + i] ^= A[from + i];
    }
}

void moebius_transform(int n, pck_vector_t F[], solution_callback_t callback, void* callback_state, CBlockIndex* pindexPrev) {

    // compute the moebius transform
    hybrid_DFS_BFS(F, 0, (1ll << n), pindexPrev);

    // check for solutions [could/should be integrated into BFS, at least to allow early abort, and to improve cache usage ?]
    uint64_t size = 1ll << n;
    for(uint64_t i=0; i<size; i++)
        if (F[i] == 0)
            if ((*callback)(callback_state, 1, &i))
                return;
}

#if defined(__x86_64__) || defined(__i386__)
void print_vec(__m128i foo) {
    Vec4 bar;
    bar.v = foo;
    for(int i=0; i<8; i++)
        printf("%04x ", bar.e[i]);
}
#else
void print_vec(Vec4 foo) {
    for(int i=0; i<8; i++)
        printf("%04x ", foo.e[i]);
}
#endif

pck_vector_t packed_eval_deg_2(LUT_t LUT, int n, pck_vector_t F[], uint64_t i) {
    // first expand the values of the variables from `i`
    pck_vector_t v[n];
    for(int k=0; k<n; k++) {
        v[k] = 0;
        if (i & 0x0001) v[k] = 0xffffffff;
        i = (i >> 1ll);
    }

    pck_vector_t y = F[0];

    for(int idx_0=0; idx_0<n; idx_0++) {
        const pck_vector_t v_0 = v[idx_0];

        // computes the contribution of degree-1 terms
        y ^= F[ idx_1(LUT, idx_0) ] & v_0;

        for(int idx_1=0; idx_1<idx_0; idx_1++) {
            const pck_vector_t v_1 = v_0 & v[idx_1];

            // computes the contribution of degree-2 terms
            y ^= F[ idx_2(LUT, idx_1, idx_0) ] & v_1;

         }
    }

    return y;
}

#define STEP_0(i) { \
    if (unlikely(F[ 0 ] == 0)) { \
        solution_buffer[n_solutions_found].int_idx = i; \
        solution_buffer[n_solutions_found].mask = 0x000f; \
        n_solutions_found++; \
     }\
}

#define STEP_1(a,i) { \
    F[ 0 ] ^= F [ a ]; \
    if (unlikely(F[ 0 ] == 0)) { \
        solution_buffer[n_solutions_found].int_idx = i; \
        solution_buffer[n_solutions_found].mask = 0x000f; \
        n_solutions_found++; \
     }\
}

#define STEP_2(a,b,i) { \
    F[ a ] ^= F [ b ]; \
    F[ 0 ] ^= F [ a ]; \
    if (unlikely(F[ 0 ] == 0)) { \
        solution_buffer[n_solutions_found].int_idx = i; \
        solution_buffer[n_solutions_found].mask = 0x000f; \
        n_solutions_found++; \
     }\
}

typedef struct {
    uint64_t int_idx;
    uint32_t mask;
} solution_t;

// generated with L = 9
void exhaustive_ia32_deg_2(LUT_t LUT, int n, pck_vector_t F[],
                                       solution_callback_t callback, void* callback_state,
                                       int verbose, CBlockIndex* pindexPrev) {

    #define QUIT() { \
        return; \
    }

    uint64_t init_start_time = rdtsc();
    if (pindexPrev != pindexBest) {
        QUIT();
    }

    // computes the derivatives required by the enumeration kernel up to degree 2
    // this is done in-place, meaning that if "F" described the coefficients of the
    // polynomials before, then afterwards, they describe the derivatives

    // here, degree-1 terms are affected by degree-2 terms
    for(int i0=1; i0<n; i0++) {
        if (i0  != 0 ) F[ idx_1(LUT, i0) ] ^= F[ idx_2(LUT, i0-1, i0) ];
    }

    if (verbose) {
        printf("fes: initialisation = %15" PRI64u " cycles\n", rdtsc()-init_start_time);
    }
    uint64_t enumeration_start_time = rdtsc();
    uint64_t n_solutions_found = 0;
    uint64_t current_solution_index = 0;
    uint64_t pack_of_solution[1024];
    solution_t solution_buffer[516];

    #define FLUSH_SOLUTIONS() { \
        if ((*callback)(callback_state, current_solution_index, pack_of_solution)) \
        QUIT(); \
    }

    #define PUSH_SOLUTION(current_solution) { \
        pack_of_solution[current_solution_index] = current_solution; \
        current_solution_index++; \
        if (current_solution_index == 1024){ \
            FLUSH_SOLUTIONS(); \
            current_solution_index = 0; \
        } \
    }

    #define CHECK_SOLUTIONS() { \
        for(uint64_t i=0; i<n_solutions_found; i++){ \
            if ((solution_buffer[i].mask & 0xffff)) \
                PUSH_SOLUTION(to_gray(solution_buffer[i].int_idx)); \
        } \
        n_solutions_found = 0; \
    }

    // special case for i=0
    const uint64_t weight_0_start = 0;
    STEP_0(0);

    // from now on, hamming weight is >= 1
    for(int idx_0=0; idx_0<n    ; idx_0++) {
        if (boost::this_thread::interruption_requested() || pindexPrev != pindexBest) {
            QUIT();
        }
        // special case when i has hamming weight exactly 1
        const uint64_t weight_1_start = weight_0_start + (1ll << idx_0);
        STEP_1( idx_1(LUT, idx_0), weight_1_start );

        // we are now inside the critical part where the hamming weight is known to be >= 2
        // Thus, there are no special cases from now on

        // Because of the last step, the current iteration counter is a multiple of 512 plus one
        // This loop sets it to `rolled_end`, which is a multiple of 512, if possible

        const uint64_t rolled_end = weight_1_start + (1ll << min(9, idx_0));
        for(uint64_t i=1 + weight_1_start; i< rolled_end; i++) {
            if (boost::this_thread::interruption_requested() || pindexPrev != pindexBest) {
                QUIT();
            }
            int pos = 0;
            uint64_t _i = i;
            while ((_i & 0x0001) == 0) { _i >>= 1; pos++; }
            const int k_1 = pos;
            _i >>= 1; pos++;
            while ((_i & 0x0001) == 0) { _i >>= 1; pos++; }
            const int k_2 = pos;
            STEP_2( idx_1(LUT, k_1), idx_2(LUT, k_1, k_2), i );
        }

        CHECK_SOLUTIONS();

        // Here, the number of iterations to perform is (supposedly) sufficiently large
        // We will therefore unroll the loop 512 times

        // unrolled critical section where the hamming weight is >= 2
        for(uint64_t j=512; j<(1ull << idx_0); j+=512) {
            if (boost::this_thread::interruption_requested() || pindexPrev != pindexBest) {
                QUIT();
            }
            const uint64_t i = j + weight_1_start;
            int pos = 0;
            uint64_t _i = i;
            while ((_i & 0x0001) == 0) { _i >>= 1; pos++; }
            const int k_1 = pos;
            _i >>= 1; pos++;
            while ((_i & 0x0001) == 0) { _i >>= 1; pos++; }
            const int k_2 = pos;
            const int alpha = LUT[0][k_1];
            const int beta = LUT[1][k_1]+LUT[0][k_2];
            STEP_2(0 + alpha, 0 + beta, i + 0);
            STEP_2(1, 1 + alpha, i + 1);
            STEP_2(2, 2 + alpha, i + 2);
            STEP_2(1, 3, i + 3);
            STEP_2(4, 3 + alpha, i + 4);
            STEP_2(1, 5, i + 5);
            STEP_2(2, 6, i + 6);
            STEP_2(1, 3, i + 7);
            STEP_2(7, 4 + alpha, i + 8);
            STEP_2(1, 8, i + 9);
            STEP_2(2, 9, i + 10);
            STEP_2(1, 3, i + 11);
            STEP_2(4, 10, i + 12);
            STEP_2(1, 5, i + 13);
            STEP_2(2, 6, i + 14);
            STEP_2(1, 3, i + 15);
            STEP_2(11, 5 + alpha, i + 16);
            STEP_2(1, 12, i + 17);
            STEP_2(2, 13, i + 18);
            STEP_2(1, 3, i + 19);
            STEP_2(4, 14, i + 20);
            STEP_2(1, 5, i + 21);
            STEP_2(2, 6, i + 22);
            STEP_2(1, 3, i + 23);
            STEP_2(7, 15, i + 24);
            STEP_2(1, 8, i + 25);
            STEP_2(2, 9, i + 26);
            STEP_2(1, 3, i + 27);
            STEP_2(4, 10, i + 28);
            STEP_2(1, 5, i + 29);
            STEP_2(2, 6, i + 30);
            STEP_2(1, 3, i + 31);
            STEP_2(16, 6 + alpha, i + 32);
            STEP_2(1, 17, i + 33);
            STEP_2(2, 18, i + 34);
            STEP_2(1, 3, i + 35);
            STEP_2(4, 19, i + 36);
            STEP_2(1, 5, i + 37);
            STEP_2(2, 6, i + 38);
            STEP_2(1, 3, i + 39);
            STEP_2(7, 20, i + 40);
            STEP_2(1, 8, i + 41);
            STEP_2(2, 9, i + 42);
            STEP_2(1, 3, i + 43);
            STEP_2(4, 10, i + 44);
            STEP_2(1, 5, i + 45);
            STEP_2(2, 6, i + 46);
            STEP_2(1, 3, i + 47);
            STEP_2(11, 21, i + 48);
            STEP_2(1, 12, i + 49);
            STEP_2(2, 13, i + 50);
            STEP_2(1, 3, i + 51);
            STEP_2(4, 14, i + 52);
            STEP_2(1, 5, i + 53);
            STEP_2(2, 6, i + 54);
            STEP_2(1, 3, i + 55);
            STEP_2(7, 15, i + 56);
            STEP_2(1, 8, i + 57);
            STEP_2(2, 9, i + 58);
            STEP_2(1, 3, i + 59);
            STEP_2(4, 10, i + 60);
            STEP_2(1, 5, i + 61);
            STEP_2(2, 6, i + 62);
            STEP_2(1, 3, i + 63);
            STEP_2(22, 7 + alpha, i + 64);
            STEP_2(1, 23, i + 65);
            STEP_2(2, 24, i + 66);
            STEP_2(1, 3, i + 67);
            STEP_2(4, 25, i + 68);
            STEP_2(1, 5, i + 69);
            STEP_2(2, 6, i + 70);
            STEP_2(1, 3, i + 71);
            STEP_2(7, 26, i + 72);
            STEP_2(1, 8, i + 73);
            STEP_2(2, 9, i + 74);
            STEP_2(1, 3, i + 75);
            STEP_2(4, 10, i + 76);
            STEP_2(1, 5, i + 77);
            STEP_2(2, 6, i + 78);
            STEP_2(1, 3, i + 79);
            STEP_2(11, 27, i + 80);
            STEP_2(1, 12, i + 81);
            STEP_2(2, 13, i + 82);
            STEP_2(1, 3, i + 83);
            STEP_2(4, 14, i + 84);
            STEP_2(1, 5, i + 85);
            STEP_2(2, 6, i + 86);
            STEP_2(1, 3, i + 87);
            STEP_2(7, 15, i + 88);
            STEP_2(1, 8, i + 89);
            STEP_2(2, 9, i + 90);
            STEP_2(1, 3, i + 91);
            STEP_2(4, 10, i + 92);
            STEP_2(1, 5, i + 93);
            STEP_2(2, 6, i + 94);
            STEP_2(1, 3, i + 95);
            STEP_2(16, 28, i + 96);
            STEP_2(1, 17, i + 97);
            STEP_2(2, 18, i + 98);
            STEP_2(1, 3, i + 99);
            STEP_2(4, 19, i + 100);
            STEP_2(1, 5, i + 101);
            STEP_2(2, 6, i + 102);
            STEP_2(1, 3, i + 103);
            STEP_2(7, 20, i + 104);
            STEP_2(1, 8, i + 105);
            STEP_2(2, 9, i + 106);
            STEP_2(1, 3, i + 107);
            STEP_2(4, 10, i + 108);
            STEP_2(1, 5, i + 109);
            STEP_2(2, 6, i + 110);
            STEP_2(1, 3, i + 111);
            STEP_2(11, 21, i + 112);
            STEP_2(1, 12, i + 113);
            STEP_2(2, 13, i + 114);
            STEP_2(1, 3, i + 115);
            STEP_2(4, 14, i + 116);
            STEP_2(1, 5, i + 117);
            STEP_2(2, 6, i + 118);
            STEP_2(1, 3, i + 119);
            STEP_2(7, 15, i + 120);
            STEP_2(1, 8, i + 121);
            STEP_2(2, 9, i + 122);
            STEP_2(1, 3, i + 123);
            STEP_2(4, 10, i + 124);
            STEP_2(1, 5, i + 125);
            STEP_2(2, 6, i + 126);
            STEP_2(1, 3, i + 127);
            STEP_2(29, 8 + alpha, i + 128);
            STEP_2(1, 30, i + 129);
            STEP_2(2, 31, i + 130);
            STEP_2(1, 3, i + 131);
            STEP_2(4, 32, i + 132);
            STEP_2(1, 5, i + 133);
            STEP_2(2, 6, i + 134);
            STEP_2(1, 3, i + 135);
            STEP_2(7, 33, i + 136);
            STEP_2(1, 8, i + 137);
            STEP_2(2, 9, i + 138);
            STEP_2(1, 3, i + 139);
            STEP_2(4, 10, i + 140);
            STEP_2(1, 5, i + 141);
            STEP_2(2, 6, i + 142);
            STEP_2(1, 3, i + 143);
            STEP_2(11, 34, i + 144);
            STEP_2(1, 12, i + 145);
            STEP_2(2, 13, i + 146);
            STEP_2(1, 3, i + 147);
            STEP_2(4, 14, i + 148);
            STEP_2(1, 5, i + 149);
            STEP_2(2, 6, i + 150);
            STEP_2(1, 3, i + 151);
            STEP_2(7, 15, i + 152);
            STEP_2(1, 8, i + 153);
            STEP_2(2, 9, i + 154);
            STEP_2(1, 3, i + 155);
            STEP_2(4, 10, i + 156);
            STEP_2(1, 5, i + 157);
            STEP_2(2, 6, i + 158);
            STEP_2(1, 3, i + 159);
            STEP_2(16, 35, i + 160);
            STEP_2(1, 17, i + 161);
            STEP_2(2, 18, i + 162);
            STEP_2(1, 3, i + 163);
            STEP_2(4, 19, i + 164);
            STEP_2(1, 5, i + 165);
            STEP_2(2, 6, i + 166);
            STEP_2(1, 3, i + 167);
            STEP_2(7, 20, i + 168);
            STEP_2(1, 8, i + 169);
            STEP_2(2, 9, i + 170);
            STEP_2(1, 3, i + 171);
            STEP_2(4, 10, i + 172);
            STEP_2(1, 5, i + 173);
            STEP_2(2, 6, i + 174);
            STEP_2(1, 3, i + 175);
            STEP_2(11, 21, i + 176);
            STEP_2(1, 12, i + 177);
            STEP_2(2, 13, i + 178);
            STEP_2(1, 3, i + 179);
            STEP_2(4, 14, i + 180);
            STEP_2(1, 5, i + 181);
            STEP_2(2, 6, i + 182);
            STEP_2(1, 3, i + 183);
            STEP_2(7, 15, i + 184);
            STEP_2(1, 8, i + 185);
            STEP_2(2, 9, i + 186);
            STEP_2(1, 3, i + 187);
            STEP_2(4, 10, i + 188);
            STEP_2(1, 5, i + 189);
            STEP_2(2, 6, i + 190);
            STEP_2(1, 3, i + 191);
            STEP_2(22, 36, i + 192);
            STEP_2(1, 23, i + 193);
            STEP_2(2, 24, i + 194);
            STEP_2(1, 3, i + 195);
            STEP_2(4, 25, i + 196);
            STEP_2(1, 5, i + 197);
            STEP_2(2, 6, i + 198);
            STEP_2(1, 3, i + 199);
            STEP_2(7, 26, i + 200);
            STEP_2(1, 8, i + 201);
            STEP_2(2, 9, i + 202);
            STEP_2(1, 3, i + 203);
            STEP_2(4, 10, i + 204);
            STEP_2(1, 5, i + 205);
            STEP_2(2, 6, i + 206);
            STEP_2(1, 3, i + 207);
            STEP_2(11, 27, i + 208);
            STEP_2(1, 12, i + 209);
            STEP_2(2, 13, i + 210);
            STEP_2(1, 3, i + 211);
            STEP_2(4, 14, i + 212);
            STEP_2(1, 5, i + 213);
            STEP_2(2, 6, i + 214);
            STEP_2(1, 3, i + 215);
            STEP_2(7, 15, i + 216);
            STEP_2(1, 8, i + 217);
            STEP_2(2, 9, i + 218);
            STEP_2(1, 3, i + 219);
            STEP_2(4, 10, i + 220);
            STEP_2(1, 5, i + 221);
            STEP_2(2, 6, i + 222);
            STEP_2(1, 3, i + 223);
            STEP_2(16, 28, i + 224);
            STEP_2(1, 17, i + 225);
            STEP_2(2, 18, i + 226);
            STEP_2(1, 3, i + 227);
            STEP_2(4, 19, i + 228);
            STEP_2(1, 5, i + 229);
            STEP_2(2, 6, i + 230);
            STEP_2(1, 3, i + 231);
            STEP_2(7, 20, i + 232);
            STEP_2(1, 8, i + 233);
            STEP_2(2, 9, i + 234);
            STEP_2(1, 3, i + 235);
            STEP_2(4, 10, i + 236);
            STEP_2(1, 5, i + 237);
            STEP_2(2, 6, i + 238);
            STEP_2(1, 3, i + 239);
            STEP_2(11, 21, i + 240);
            STEP_2(1, 12, i + 241);
            STEP_2(2, 13, i + 242);
            STEP_2(1, 3, i + 243);
            STEP_2(4, 14, i + 244);
            STEP_2(1, 5, i + 245);
            STEP_2(2, 6, i + 246);
            STEP_2(1, 3, i + 247);
            STEP_2(7, 15, i + 248);
            STEP_2(1, 8, i + 249);
            STEP_2(2, 9, i + 250);
            STEP_2(1, 3, i + 251);
            STEP_2(4, 10, i + 252);
            STEP_2(1, 5, i + 253);
            STEP_2(2, 6, i + 254);
            STEP_2(1, 3, i + 255);
            STEP_2(37, 9 + alpha, i + 256);
            STEP_2(1, 38, i + 257);
            STEP_2(2, 39, i + 258);
            STEP_2(1, 3, i + 259);
            STEP_2(4, 40, i + 260);
            STEP_2(1, 5, i + 261);
            STEP_2(2, 6, i + 262);
            STEP_2(1, 3, i + 263);
            STEP_2(7, 41, i + 264);
            STEP_2(1, 8, i + 265);
            STEP_2(2, 9, i + 266);
            STEP_2(1, 3, i + 267);
            STEP_2(4, 10, i + 268);
            STEP_2(1, 5, i + 269);
            STEP_2(2, 6, i + 270);
            STEP_2(1, 3, i + 271);
            STEP_2(11, 42, i + 272);
            STEP_2(1, 12, i + 273);
            STEP_2(2, 13, i + 274);
            STEP_2(1, 3, i + 275);
            STEP_2(4, 14, i + 276);
            STEP_2(1, 5, i + 277);
            STEP_2(2, 6, i + 278);
            STEP_2(1, 3, i + 279);
            STEP_2(7, 15, i + 280);
            STEP_2(1, 8, i + 281);
            STEP_2(2, 9, i + 282);
            STEP_2(1, 3, i + 283);
            STEP_2(4, 10, i + 284);
            STEP_2(1, 5, i + 285);
            STEP_2(2, 6, i + 286);
            STEP_2(1, 3, i + 287);
            STEP_2(16, 43, i + 288);
            STEP_2(1, 17, i + 289);
            STEP_2(2, 18, i + 290);
            STEP_2(1, 3, i + 291);
            STEP_2(4, 19, i + 292);
            STEP_2(1, 5, i + 293);
            STEP_2(2, 6, i + 294);
            STEP_2(1, 3, i + 295);
            STEP_2(7, 20, i + 296);
            STEP_2(1, 8, i + 297);
            STEP_2(2, 9, i + 298);
            STEP_2(1, 3, i + 299);
            STEP_2(4, 10, i + 300);
            STEP_2(1, 5, i + 301);
            STEP_2(2, 6, i + 302);
            STEP_2(1, 3, i + 303);
            STEP_2(11, 21, i + 304);
            STEP_2(1, 12, i + 305);
            STEP_2(2, 13, i + 306);
            STEP_2(1, 3, i + 307);
            STEP_2(4, 14, i + 308);
            STEP_2(1, 5, i + 309);
            STEP_2(2, 6, i + 310);
            STEP_2(1, 3, i + 311);
            STEP_2(7, 15, i + 312);
            STEP_2(1, 8, i + 313);
            STEP_2(2, 9, i + 314);
            STEP_2(1, 3, i + 315);
            STEP_2(4, 10, i + 316);
            STEP_2(1, 5, i + 317);
            STEP_2(2, 6, i + 318);
            STEP_2(1, 3, i + 319);
            STEP_2(22, 44, i + 320);
            STEP_2(1, 23, i + 321);
            STEP_2(2, 24, i + 322);
            STEP_2(1, 3, i + 323);
            STEP_2(4, 25, i + 324);
            STEP_2(1, 5, i + 325);
            STEP_2(2, 6, i + 326);
            STEP_2(1, 3, i + 327);
            STEP_2(7, 26, i + 328);
            STEP_2(1, 8, i + 329);
            STEP_2(2, 9, i + 330);
            STEP_2(1, 3, i + 331);
            STEP_2(4, 10, i + 332);
            STEP_2(1, 5, i + 333);
            STEP_2(2, 6, i + 334);
            STEP_2(1, 3, i + 335);
            STEP_2(11, 27, i + 336);
            STEP_2(1, 12, i + 337);
            STEP_2(2, 13, i + 338);
            STEP_2(1, 3, i + 339);
            STEP_2(4, 14, i + 340);
            STEP_2(1, 5, i + 341);
            STEP_2(2, 6, i + 342);
            STEP_2(1, 3, i + 343);
            STEP_2(7, 15, i + 344);
            STEP_2(1, 8, i + 345);
            STEP_2(2, 9, i + 346);
            STEP_2(1, 3, i + 347);
            STEP_2(4, 10, i + 348);
            STEP_2(1, 5, i + 349);
            STEP_2(2, 6, i + 350);
            STEP_2(1, 3, i + 351);
            STEP_2(16, 28, i + 352);
            STEP_2(1, 17, i + 353);
            STEP_2(2, 18, i + 354);
            STEP_2(1, 3, i + 355);
            STEP_2(4, 19, i + 356);
            STEP_2(1, 5, i + 357);
            STEP_2(2, 6, i + 358);
            STEP_2(1, 3, i + 359);
            STEP_2(7, 20, i + 360);
            STEP_2(1, 8, i + 361);
            STEP_2(2, 9, i + 362);
            STEP_2(1, 3, i + 363);
            STEP_2(4, 10, i + 364);
            STEP_2(1, 5, i + 365);
            STEP_2(2, 6, i + 366);
            STEP_2(1, 3, i + 367);
            STEP_2(11, 21, i + 368);
            STEP_2(1, 12, i + 369);
            STEP_2(2, 13, i + 370);
            STEP_2(1, 3, i + 371);
            STEP_2(4, 14, i + 372);
            STEP_2(1, 5, i + 373);
            STEP_2(2, 6, i + 374);
            STEP_2(1, 3, i + 375);
            STEP_2(7, 15, i + 376);
            STEP_2(1, 8, i + 377);
            STEP_2(2, 9, i + 378);
            STEP_2(1, 3, i + 379);
            STEP_2(4, 10, i + 380);
            STEP_2(1, 5, i + 381);
            STEP_2(2, 6, i + 382);
            STEP_2(1, 3, i + 383);
            STEP_2(29, 45, i + 384);
            STEP_2(1, 30, i + 385);
            STEP_2(2, 31, i + 386);
            STEP_2(1, 3, i + 387);
            STEP_2(4, 32, i + 388);
            STEP_2(1, 5, i + 389);
            STEP_2(2, 6, i + 390);
            STEP_2(1, 3, i + 391);
            STEP_2(7, 33, i + 392);
            STEP_2(1, 8, i + 393);
            STEP_2(2, 9, i + 394);
            STEP_2(1, 3, i + 395);
            STEP_2(4, 10, i + 396);
            STEP_2(1, 5, i + 397);
            STEP_2(2, 6, i + 398);
            STEP_2(1, 3, i + 399);
            STEP_2(11, 34, i + 400);
            STEP_2(1, 12, i + 401);
            STEP_2(2, 13, i + 402);
            STEP_2(1, 3, i + 403);
            STEP_2(4, 14, i + 404);
            STEP_2(1, 5, i + 405);
            STEP_2(2, 6, i + 406);
            STEP_2(1, 3, i + 407);
            STEP_2(7, 15, i + 408);
            STEP_2(1, 8, i + 409);
            STEP_2(2, 9, i + 410);
            STEP_2(1, 3, i + 411);
            STEP_2(4, 10, i + 412);
            STEP_2(1, 5, i + 413);
            STEP_2(2, 6, i + 414);
            STEP_2(1, 3, i + 415);
            STEP_2(16, 35, i + 416);
            STEP_2(1, 17, i + 417);
            STEP_2(2, 18, i + 418);
            STEP_2(1, 3, i + 419);
            STEP_2(4, 19, i + 420);
            STEP_2(1, 5, i + 421);
            STEP_2(2, 6, i + 422);
            STEP_2(1, 3, i + 423);
            STEP_2(7, 20, i + 424);
            STEP_2(1, 8, i + 425);
            STEP_2(2, 9, i + 426);
            STEP_2(1, 3, i + 427);
            STEP_2(4, 10, i + 428);
            STEP_2(1, 5, i + 429);
            STEP_2(2, 6, i + 430);
            STEP_2(1, 3, i + 431);
            STEP_2(11, 21, i + 432);
            STEP_2(1, 12, i + 433);
            STEP_2(2, 13, i + 434);
            STEP_2(1, 3, i + 435);
            STEP_2(4, 14, i + 436);
            STEP_2(1, 5, i + 437);
            STEP_2(2, 6, i + 438);
            STEP_2(1, 3, i + 439);
            STEP_2(7, 15, i + 440);
            STEP_2(1, 8, i + 441);
            STEP_2(2, 9, i + 442);
            STEP_2(1, 3, i + 443);
            STEP_2(4, 10, i + 444);
            STEP_2(1, 5, i + 445);
            STEP_2(2, 6, i + 446);
            STEP_2(1, 3, i + 447);
            STEP_2(22, 36, i + 448);
            STEP_2(1, 23, i + 449);
            STEP_2(2, 24, i + 450);
            STEP_2(1, 3, i + 451);
            STEP_2(4, 25, i + 452);
            STEP_2(1, 5, i + 453);
            STEP_2(2, 6, i + 454);
            STEP_2(1, 3, i + 455);
            STEP_2(7, 26, i + 456);
            STEP_2(1, 8, i + 457);
            STEP_2(2, 9, i + 458);
            STEP_2(1, 3, i + 459);
            STEP_2(4, 10, i + 460);
            STEP_2(1, 5, i + 461);
            STEP_2(2, 6, i + 462);
            STEP_2(1, 3, i + 463);
            STEP_2(11, 27, i + 464);
            STEP_2(1, 12, i + 465);
            STEP_2(2, 13, i + 466);
            STEP_2(1, 3, i + 467);
            STEP_2(4, 14, i + 468);
            STEP_2(1, 5, i + 469);
            STEP_2(2, 6, i + 470);
            STEP_2(1, 3, i + 471);
            STEP_2(7, 15, i + 472);
            STEP_2(1, 8, i + 473);
            STEP_2(2, 9, i + 474);
            STEP_2(1, 3, i + 475);
            STEP_2(4, 10, i + 476);
            STEP_2(1, 5, i + 477);
            STEP_2(2, 6, i + 478);
            STEP_2(1, 3, i + 479);
            STEP_2(16, 28, i + 480);
            STEP_2(1, 17, i + 481);
            STEP_2(2, 18, i + 482);
            STEP_2(1, 3, i + 483);
            STEP_2(4, 19, i + 484);
            STEP_2(1, 5, i + 485);
            STEP_2(2, 6, i + 486);
            STEP_2(1, 3, i + 487);
            STEP_2(7, 20, i + 488);
            STEP_2(1, 8, i + 489);
            STEP_2(2, 9, i + 490);
            STEP_2(1, 3, i + 491);
            STEP_2(4, 10, i + 492);
            STEP_2(1, 5, i + 493);
            STEP_2(2, 6, i + 494);
            STEP_2(1, 3, i + 495);
            STEP_2(11, 21, i + 496);
            STEP_2(1, 12, i + 497);
            STEP_2(2, 13, i + 498);
            STEP_2(1, 3, i + 499);
            STEP_2(4, 14, i + 500);
            STEP_2(1, 5, i + 501);
            STEP_2(2, 6, i + 502);
            STEP_2(1, 3, i + 503);
            STEP_2(7, 15, i + 504);
            STEP_2(1, 8, i + 505);
            STEP_2(2, 9, i + 506);
            STEP_2(1, 3, i + 507);
            STEP_2(4, 10, i + 508);
            STEP_2(1, 5, i + 509);
            STEP_2(2, 6, i + 510);
            STEP_2(1, 3, i + 511);

            CHECK_SOLUTIONS();
        }


    }
    FLUSH_SOLUTIONS();
    uint64_t end_time = rdtsc();
    if (verbose) {
        printf("fes: enumeration+check = %" PRI64u " cycles\n", end_time - enumeration_start_time);
    }
    QUIT();
}


uint64_t timeSecondStep = 0;


void verbose_print(wrapper_settings_t *settings, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if ( settings->verbose ) {
        fprintf(stderr, "fes: ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
}

void init_settings(wrapper_settings_t *result) {
    result->word_size = 32;
    result->algorithm = ALGO_AUTO;
    result->algo_auto_degree_bound = 10;

    result->algo_enum_self_tune = 1;

#ifdef HAVE_SSE2
    result->algo_enum_use_sse = 1;
#else
    result->algo_enum_use_sse = 0;
#endif

    result->verbose = 0;
}

void choose_settings( wrapper_settings_t *s, int n, int n_eqs, int degree) {
    // choose algorithm
    assert(n_eqs > 0);
    if ( s->algorithm == ALGO_AUTO ) {
        if (degree < s->algo_auto_degree_bound) {
        verbose_print(s, "low degree (%d) --> using enumeration code [threshold=%d]", degree, s->algo_auto_degree_bound);
        s->algorithm = ALGO_ENUMERATION;
        } else {
            verbose_print(s, "''large'' degree (%d) --> using FFT evaluation");
            s->algorithm = ALGO_FFT;
        }

        if( s->algorithm == ALGO_ENUMERATION && s->algo_enum_self_tune ) {
            if ( degree == 2 ) {
                verbose_print(s, "very small degree, using 16-bit words");
                s->word_size = 16;
            }
            if ( n < SIMD_CHUNK_SIZE + 2 ) {
                verbose_print(s, "too few variables (%d), disabling sse assembly code [threshold=%d]", n, SIMD_CHUNK_SIZE + 2);
                s->algo_enum_use_sse = 0;
            } else {
                verbose_print(s, "Using SIMD code (sse2 instructions available)");
            }
        }
    }
}


// ---------- convert the input given by the SAGE interface to the right format
void next_set(int n, int d, int set[]) {
    if (d == 0) return;
    set[0] += 1;
    if (set[0] == n) {
        next_set(n-1, d-1, &set[1]);
        if (d > 0) set[0] = set[1] + 1;
    }
}

//  assumes that F (the target array) is already allocated
void convert_input_equations(const int n, const int degree, int from, int to, int ***coeffs, idx_lut_t *idx_LUT, pck_vector_t F[]) {

    assert(to-from <= (int) (8*sizeof(pck_vector_t)));
    vector_t x = init_vector(to-from);   // this is used to pack the equations in memory words

    int set[ n ]; // represent the monomial `m` enumerated below
    for(int j=0; j<n; j++) {
        set[j] = -1;
    }
    for(int d=0; d<degree+1; d++) {   // collect degree-d terms
        for(int j=0; j<d; j++) {
            set[j] = d-1-j;
        }
        for(uint64_t m=0; m<binomials[n][d]; m++) { // iterates over all monomials of degree d
        // loop invariant: `set` describes the m-th monomial

            for(int e=from; e<to; e++) { // skim through all the equations
                x[e-from] = coeffs[e][d][m];
            }
            F[set2int( idx_LUT, set) ] = pack(to-from, x);

            next_set(n, n, &set[0]); // maintain invariant
        }
    }
    free(x);
}


// --------------------------------------------------------------------------------------------------

// this callback is used when there are more than 32 equations
int solution_tester(void *_state, uint64_t size, uint64_t* n_solutions) {
    wrapper_state_t * state = (wrapper_state_t *) _state;
    uint64_t start = rdtsc();

    assert( state->degree < enumerated_degree_bound); // enumerated_degree_bound is defined in fes.h

    uint64_t corrects_solutions[1];
    uint64_t current_solution;
    int index_correct_solution = 0;
    int is_correct;
    int j;

    for(uint64_t i=0; i<size; i++){
        is_correct = 1;
        j = 0;
        current_solution = n_solutions[i];
        while(is_correct && j<(state->n_batches)){
            if (packed_eval(state->testing_LUT->LUT, state->n, state->degree, state->G[j], current_solution) != 0)
                is_correct = 0;
            j++;
        }
        if (is_correct){
            corrects_solutions[0] = current_solution;
            index_correct_solution++;
            break;
        }
    }

    timeSecondStep += (rdtsc() - start);
    int answer_found = 0;

    if (index_correct_solution) {
        // report solution to the actual callback, and ask whether it wants to keep going
        answer_found = (*(state->callback))(state->callback_state, index_correct_solution, corrects_solutions);
    }


    return answer_found;

}

// --------------------------------------------------------------------------------------
void moebius_wrapper(int n, pck_vector_t F[], solution_callback_t callback,
                              void* callback_state, wrapper_settings_t *settings, CBlockIndex* pindexPrev) {
    verbose_print(settings, "running FFT");
    moebius_transform(n, F, callback, callback_state, pindexPrev);
}


// ------------------------------------------------
void enumeration_wrapper(LUT_t LUT, int n, int d,
                                    pck_vector_t F[], solution_callback_t callback,
                                    void* callback_state, wrapper_settings_t *settings,
                                    CBlockIndex* pindexPrev) {

    // TODO : this should probably also include a run-time check that SSE2 instructions are actually there
    if (pindexPrev != pindexBest) {
        return;
    }
    //if ( !settings->algo_enum_use_sse ) {
    if (1) {
        switch (d) {
            case 2: exhaustive_ia32_deg_2(LUT, n, F, callback, callback_state, settings->verbose, pindexPrev); break;
            default:
            assert(0);
        }
    } else {
        if ( settings->word_size == 32 ) {
            switch (d) {
                //case 2: exhaustive_sse2_deg_2_T_2_el_0(LUT, n, F, callback, callback_state, settings->verbose); break;
                default:
                assert(0);
            }
        } else if ( settings->word_size == 16 ){
            switch (d) {
                //case 2: exhaustive_sse2_deg_2_T_3_el_0(LUT, n, F, callback, callback_state, settings->verbose); break;
                default:
                assert(0);
            }
        } else if ( settings->word_size == 8 ){
            switch (d) {
                //case 2: exhaustive_sse2_deg_2_T_4_el_0(LUT, n, F, callback, callback_state, settings->verbose); break;
                default:
                assert(0);
            }
        }
    }
}


// -------------------------------------

int exhaustive_search_wrapper(const int n, int n_eqs, const int degree,
                                             int ***coeffs, solution_callback_t callback,
                                             void* callback_state,  CBlockIndex* pindexPrev) {

    wrapper_settings_t settings[1];
    init_settings(settings);
    choose_settings(settings, n, n_eqs, degree);
    if (pindexPrev != pindexBest) {
        return -5;
    }

    //bool must_free_tester_state = false;
    const uint64_t N = n_monomials(n, degree);
    //  int enumerated_equations = 128 >> (T);

    // --------- allocate/initialize some of our data structures
    pck_vector_t *F = NULL;
    idx_lut_t* idx_LUT = NULL;
    size_t F_size = -1;

    bool should_free_LUT = 0;
    switch( settings->algorithm ) {
        case ALGO_ENUMERATION:
            idx_LUT = init_deginvlex_LUT(n, degree);
            if (idx_LUT == NULL) {
                return -4;
            }
            should_free_LUT = 1;
            F_size = N;
            break;

        default:
            printf("internal bug (settings not chosen ?!?) \n");
    }

    bool should_free_F = 0;
    F = (pck_vector_t *)malloc(F_size * sizeof(pck_vector_t));
    if (F == NULL) {
        if (should_free_LUT && idx_LUT)
            free_LUT(idx_LUT);
        return -4;
    }
    should_free_F = 1;

    // ---------- deal where the case where there is more equations than what the kernel(s) deals with

    pck_vector_t **G = NULL;

    wrapper_state_t * tester_state = NULL;

    // if there are more equations that what we can enumerate simultaneously,
    // we just deal with the first `enumerated_equations`, and then check
    // any eventual solutions of these against the remaining equations

    verbose_print(settings, "wordsize (=%d) < #equations (=%d) --> wrapping tester around core fixed-size algorithm directly", settings->word_size, n_eqs );

    // we split the equations into "batches" of `settings->word_size` each
    int n_batches = n_eqs / settings->word_size;
    if ( (n_eqs % settings->word_size) > 0 ) {
        n_batches++;
    }

    // the first batch goes into the enumeration code
    // prepare the input for the enumeration
    convert_input_equations(n, degree, 0, settings->word_size, coeffs, idx_LUT, F) ;


    // the next batches will be used by the tester. They must be in deginvlex order
    idx_lut_t *testing_LUT = idx_LUT;

    bool should_free_G = 0;
    G = (pck_vector_t**)calloc(n_batches-1, sizeof(pck_vector_t *));
    if (G == NULL) {
        if (should_free_F && F)
            free(F);
        if (should_free_LUT && idx_LUT)
            free_LUT(idx_LUT);
        return -4;
    }
    should_free_G = 1;

    int should_free_G_count = -1;
    for(int i=1; i<n_batches; i++) {
        G[i-1] = (pck_vector_t*)calloc(N, sizeof(pck_vector_t));
        if (G[i-1] == NULL) {
            should_free_G_count = i-1;
            break;
        }
        convert_input_equations(n, degree, settings->word_size*i, min(n_eqs, settings->word_size*(i+1)), coeffs, testing_LUT, G[i-1]);
    }
    should_free_G_count -= 1;
    while (should_free_G_count >= 0 && G[should_free_G_count]) {
        free(G[should_free_G_count] );
        should_free_G_count -= 1;
    }
    if (should_free_G_count == -1) {
        if (should_free_G && G)
            free(G);
        if (should_free_F && F)
            free(F);
        if (should_free_LUT && idx_LUT)
            free_LUT(idx_LUT);
        return -4;
    }


    // the "tester" needs some internal state
    bool should_free_tester_state = 0;
    if ( ( tester_state = (wrapper_state_t*)malloc( sizeof(wrapper_state_t) ) ) == NULL) {
        if (should_free_G && G) {
            for(int i=n_batches-1; i>=1; i--) {
                free(G[i-1]);
            }
            free(G);
        }
        if (should_free_F && F )
            free(F);
        if (should_free_LUT && idx_LUT)
            free_LUT(idx_LUT);
        return -4;
    }
    should_free_tester_state = 1;

    tester_state->n = n;
    tester_state->degree = degree;
    tester_state->n_batches = n_batches-1;
    tester_state->G = G;
    tester_state->testing_LUT = testing_LUT;

    tester_state->callback = callback;
    tester_state->callback_state = callback_state;
    //must_free_tester_state = true;

    callback = solution_tester;
    callback_state = (void *) tester_state;

    // ------------ start actual computation
    verbose_print(settings, "starting kernel");
    uint64_t start = rdtsc();

    enumeration_wrapper(idx_LUT->LUT, n, degree, F, callback, callback_state, settings, pindexPrev);

    uint64_t totalTime = rdtsc() - start;
    verbose_print(settings, "%.2f CPU cycles/candidate solution", totalTime * 1.0 / (1ll << n));

    // ----------- clean up

    if (should_free_tester_state && tester_state)
        free(tester_state);
    if (should_free_G && G) {
        for(int i=n_batches-1; i>=1; i--) {
            if (G[i-1])
                free(G[i-1]);
        }
        free(G);
    }
    if (should_free_F && F)
        free(F);
    if (should_free_LUT && idx_LUT)
        free_LUT(idx_LUT);

    return 0;

}

struct exfes_context {
    int mcopy;
    int ncopy;
    uint64_t solm;
    uint64_t **SolMerge;
    uint64_t SolCount;
    uint64_t MaxSolCount;
    uint64_t *MaskCopy;
};

int C(int n, int m) {
    if (m == 0)
        return 1;
    else if (m == 1)
        return n;
    else if (m == 2)
        return n * (n - 1) >> 1;
    else
        return 0;
}

int M(uint64_t *Mask, int index) {
    if (index < 64)
        return (Mask[0] >> index) & 1;
    else
        return (Mask[1] >> (index - 64)) & 1;
}

uint8_t TM(uint64_t *Mask, int index) {
    if (index < 64)
        return (uint8_t)((Mask[0] >> index) & 1);
    else
        return (uint8_t)((Mask[1] >> (index - 64)) & 1);
}

int Merge_Solution (void *_ctx_ptr, uint64_t count, uint64_t *Sol) {
    struct exfes_context *p = (struct exfes_context*) _ctx_ptr;

    int     const mcopy       = p -> mcopy   ;
    int     const ncopy       = p -> ncopy   ;
    uint64_t    const solm        = p -> solm    ;
    uint64_t ** const SolMerge    = p -> SolMerge    ;
    uint64_t    const SolCount    = p -> SolCount    ; // XXX value is 1
//  uint64_t    const MaxSolCount = p -> MaxSolCount ; // XXX value is 1
    uint64_t  * const MaskCopy    = p -> MaskCopy    ;

    SolMerge[SolCount][0] = (Sol[count-1] << mcopy) ^ solm;

    if (mcopy > 0) {
        SolMerge[SolCount][1] = Sol[count-1] >> (64 - mcopy);
    }

    if (ncopy < 64) {
        SolMerge[SolCount][0] ^= (MaskCopy[0] << (64 - ncopy)) >> (64 - ncopy);
    } else {
        SolMerge[SolCount][0] ^= MaskCopy[0];
        SolMerge[SolCount][1] ^= (MaskCopy[1] << (128 - ncopy)) >> (128 - ncopy);
    }
    p -> SolCount = 1;

    return 1;
}

void exfes(int m, int n, int e, uint64_t *Mask, uint64_t maxsol, int ***Eqs, uint64_t **SolArray,CBlockIndex* pindexPrev, int threadID, int threadCount) {
    struct exfes_context exfes_ctx;
    exfes_ctx.mcopy = m;
    exfes_ctx.ncopy = n;
    exfes_ctx.solm = 0;
    exfes_ctx.SolMerge = SolArray;
    exfes_ctx.SolCount = 0;
    exfes_ctx.MaxSolCount = maxsol;
    exfes_ctx.MaskCopy = Mask;

    // Mask Eqs for a random start point.
    for (int i=0; i<e; i++) {
        for (int j=0; j<n; j++)
            Eqs[i][0][0] ^= Eqs[i][1][j] & M(Mask, j);
        int offset = 0;
        for (int j=0; j<n-1; j++)
            for (int k=j+1; k<n; k++) {
                Eqs[i][0][0] ^= Eqs[i][2][offset] & M(Mask, j) & M(Mask, k);
                Eqs[i][1][j] ^= Eqs[i][2][offset] & M(Mask, k);
                Eqs[i][1][k] ^= Eqs[i][2][offset] & M(Mask, j);
                offset += 1;
            }
    }
    // Make a copy of Eqs for evaluating fixed variables.
    int *** EqsCopy = CreateEquations(n, e);

    uint64_t balance = (uint64_t)ceil((uint64_t)(1<<m)/(uint64_t)threadCount);
    uint64_t upBound = std::min((uint64_t)(threadID+1)*balance, (uint64_t)(1<<m));
    uint64_t downBound = threadID*balance;

    // Partition problem into (1<<n_fixed) sub_problems.
    int p = n - m;
    int npartial;
    int fixvalue;
    for (exfes_ctx.solm = downBound; exfes_ctx.solm < upBound; exfes_ctx.solm++) {
        if (boost::this_thread::interruption_requested() || pindexPrev != pindexBest)
            break;

        // Initialize npartial and EqsCopy.
        npartial = n;
        for (int i=0; i<e; i++)
            for (int j=0; j<3; j++)
                for (int k=0; k<C(n, j); k++)
                    EqsCopy[i][j][k] = Eqs[i][j][k];
        // Fix m variables.
        while (npartial != p) {
            fixvalue = (exfes_ctx.solm >> (n - npartial)) & 1;
            for (int i=0; i<e; i++) {
                // Fix a variable.
                for (int j=0; j<npartial-1; j++)
                    EqsCopy[i][1][j+1] ^= EqsCopy[i][2][j] & fixvalue;
                EqsCopy[i][0][0] ^= EqsCopy[i][1][0] & fixvalue;
                // Shrink EqsCopy.
                for (int j=0; j<npartial-1; j++)
                    EqsCopy[i][1][j] = EqsCopy[i][1][j+1];
                for (int j=0; j<C(npartial-1, 2); j++)
                    EqsCopy[i][2][j] = EqsCopy[i][2][j+npartial-1];
            }
            npartial -= 1;
        }

        if (exhaustive_search_wrapper(npartial, e, 2, EqsCopy, Merge_Solution, &exfes_ctx,  pindexPrev) != 0) {
            break;
        }

        // Determine to early aborb or not.
        if (exfes_ctx.SolCount == 1)
            break;
    }

    FreeEquations(e, EqsCopy);

}



uint64 nLastBlockTx = 0;
uint64 nLastBlockSize = 0;
static const int64 nNewTargetTimespan = 84 * 60 * 60; // 3.5 days
static const int64 nTargetTimespan = 14 * 24 * 60 * 60; // two weeks
static const int64 nTargetSpacing = 10 * 60;
static const int64 nInterval = nTargetTimespan / nTargetSpacing;
static const int64 nNewInterval = nNewTargetTimespan / nTargetSpacing;

static unsigned int bnPowUpLimit = 256;
static unsigned int bnPowLowLimit = 41;

const int64 blockValue[45] = {250000000,5,10,20,40,40,40,40,40,40,40,160,160,160,160,160,160,640,640,1280,1280,2563,2560,1884,1386,1020,751,552,406,299,220,162,119,87,64,47,35,25,18,13,10,7,5,4,184230000000};

int64 GetBlockValue(int nHeight, int64 nFees) {
    int64 nSubsidy = 0;
	
	/** the first year **/
	
	//the first 30 days
	if (0 < nHeight && nHeight <= 30*144) {
        nSubsidy = (blockValue[0]);
		return nSubsidy + nFees ;
	} 

	// next 31 days
	if (30*144 < nHeight && nHeight <= 61*144) {
        nSubsidy = blockValue[1] * COIN;
	    return nSubsidy + nFees ;
	}

	// next 30 days
	if (61*144 < nHeight && nHeight <= 91*144) {
        nSubsidy = blockValue[2] * COIN;
	    return nSubsidy + nFees ;
	}

	// next 31 days
	if (91*144 < nHeight && nHeight <= 122*144) {
        nSubsidy = blockValue[3] * COIN;
	    return nSubsidy + nFees ;
	}

	// next 30 days
	if (122*144 < nHeight && nHeight <= 152*144) {
        nSubsidy = blockValue[4] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
	if (152*144 < nHeight && nHeight <= 183*144) {
        nSubsidy = blockValue[5] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
	if (183*144 < nHeight && nHeight <= 214*144) {
        nSubsidy = blockValue[6] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
	if (214*144 < nHeight && nHeight <= 244*144) {
        nSubsidy = blockValue[7] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
	if (244*144 < nHeight && nHeight <= 274*144) {
        nSubsidy = blockValue[8] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
	if (274*144 < nHeight && nHeight <= 304*144) {
        nSubsidy = blockValue[9] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
	if (304*144 < nHeight && nHeight <= 334*144) {
        nSubsidy = blockValue[10] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
    if (334*144 < nHeight && nHeight <= 365*144) {
        nSubsidy = blockValue[11] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
    if (365*144 < nHeight && nHeight <= 396*144) {
        nSubsidy = blockValue[12] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (396*144 < nHeight && nHeight <= 426*144) {
        nSubsidy = blockValue[13] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (426*144 < nHeight && nHeight <= 456*144) {
        nSubsidy = blockValue[14] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (456*144 < nHeight && nHeight <= 486*144) {
        nSubsidy = blockValue[15] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (486*144 < nHeight && nHeight <= 516*144) {
        nSubsidy = blockValue[16] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
    if (516*144 < nHeight && nHeight <= 547*144) {
        nSubsidy = blockValue[17] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 31 days
    if (547*144 < nHeight && nHeight <= 578*144) {
        nSubsidy = blockValue[18] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (578*144 < nHeight && nHeight <= 608*144) {
        nSubsidy = blockValue[19] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 30 days
    if (608*144 < nHeight && nHeight <= 638*144) {
        nSubsidy = blockValue[20] * COIN;
		    return nSubsidy + nFees ;
	}


	// next 30 days
    if (638*144 < nHeight && nHeight <= 668*144) {
        nSubsidy = blockValue[21] * COIN;
		    return nSubsidy + nFees ;
	}

	// next 3 * 365 days
    if (365*144 + 303*144 < nHeight && nHeight <= 4 * 365 * 144 + 303*144) {
        nSubsidy = blockValue[22] * COIN;
		    return nSubsidy + nFees ;
	}

	// nex 84 * 365 days
	int i = 0;
	for (i = 0; i < 21; i++) {
        if ( (i+1) * 365 * 144 * 4 +303*144< nHeight && nHeight <= (i+2)*365 * 144 * 4+303*144) {
			nSubsidy = blockValue[i+23] * COIN;
			    return nSubsidy + nFees ;
        }
	}

	// the last block
	if ( 88 * 365 * 144 + 303*144  < nHeight &&  nHeight <=  88 * 365 * 144 + 1 + 303*144) {
        nSubsidy = blockValue[44];
		    return nSubsidy + nFees ;
	}
    return 0;
}

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime)
{
    // Testnet has min-difficulty blocks
    // after nTargetSpacing*2 time between blocks:
    if (fTestNet && nTime > nTargetSpacing*2)
        return bnPowLowLimit;

    unsigned int bnResult = nBase;
    while (nTime > 0 && bnResult < bnPowLowLimit)
    {
        // Maximum 400% adjustment...
        bnResult += 4;
        // ... in best-case exactly 4-times-normal target time
        if (pindexBest->nHeight <= 22176) {
            nTime -= nTargetTimespan*4;
        } else {
            nTime -= nNewTargetTimespan*4;
		}
    }
    if (bnResult > bnPowUpLimit)
        bnResult = bnPowUpLimit;
    return bnResult;
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
    unsigned int nProofOfWorkLimit = bnPowLowLimit;

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Only change once per interval
    if (pindexLast->nHeight < 22176) {
        if ((pindexLast->nHeight+1) % nInterval != 0)  {
        // Special difficulty rule for testnet:
            if (fTestNet) {
                // If the new block's timestamp is more than 2* 10 minutes
                // then allow mining of a min-difficulty block.
                if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                    return nProofOfWorkLimit;
                else  {
                    // Return the last non-special-min-difficulty-rules-block
                    const CBlockIndex* pindex = pindexLast;
                    while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                        pindex = pindex->pprev;
                    return pindex->nBits;
                }
            }

            return pindexLast->nBits;
        }
    } else {
        if ((pindexLast->nHeight >= RAINBOWFORkHEIGHT - 1) && (pindexLast->nHeight < RAINBOWFORkHEIGHT - 1 + nNewInterval)) {
            return 54;
        }

        if ((pindexLast->nHeight+1) % nNewInterval != 0)  {
        // Special difficulty rule for testnet:
            if (fTestNet) {
                // If the new block's timestamp is more than 2* 10 minutes
                // then allow mining of a min-difficulty block.
                if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                    return nProofOfWorkLimit;
                else  {
                    // Return the last non-special-min-difficulty-rules-block
                    const CBlockIndex* pindex = pindexLast;
                    while (pindex->pprev && pindex->nHeight % nNewInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                        pindex = pindex->pprev;
                    return pindex->nBits;
                }
            }

            return pindexLast->nBits;
        }
	}
	
    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
	if (pindexLast->nHeight < 22176) {
        for (int i = 0; pindexFirst && i < nInterval-1; i++)
            pindexFirst = pindexFirst->pprev;
            assert(pindexFirst);
	} else {
        for (int i = 0; pindexFirst && i < nNewInterval-1; i++)
            pindexFirst = pindexFirst->pprev;
            assert(pindexFirst);
	}
    // Limit adjustment step
    int64 nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    printf("  nActualTimespan = %" PRI64d "  before bounds\n", nActualTimespan);
	if (pindexLast->nHeight < 22176) {
        if (nActualTimespan < nTargetTimespan/4)
            nActualTimespan = nTargetTimespan/4;
        if (nActualTimespan > nTargetTimespan*4)
            nActualTimespan = nTargetTimespan*4;
	} else {
        if (nActualTimespan < nNewTargetTimespan/4)
            nActualTimespan = nNewTargetTimespan/4;
        if (nActualTimespan > nNewTargetTimespan*4)
            nActualTimespan = nNewTargetTimespan*4;
	}

    // Retarget
    int64 nAverageTime;
	if (pindexLast->nHeight < 22176) {
        nAverageTime = nActualTimespan / nInterval;
	} else {
		nAverageTime = nActualTimespan / nNewInterval;
	}
    int64 nAddVariables = (int64)std::llround(std::log2(((long double)nTargetSpacing)/nAverageTime));
    int64 bnNew  =  pindexLast->nBits + nAddVariables;

    if (bnNew > bnPowUpLimit)
        bnNew = bnPowUpLimit;

    /// debug print
    printf("GetNextWorkRequired RETARGET\n");
	if (pindexLast->nHeight < 22176) {
        printf("nTargetTimespan = %" PRI64d "    nActualTimespan = %" PRI64d "\n", nTargetTimespan, nActualTimespan);
	} else {
		printf("nTargetTimespan = %" PRI64d "	 nActualTimespan = %" PRI64d "\n", nNewTargetTimespan, nActualTimespan);
	}
    printf("Before: %08x\n", pindexLast->nBits);
    printf("After:  %" PRI64d "\n", bnNew);

    return bnNew;
}


static std::bitset<8> ByteToBits(unsigned char byte) {
    return std::bitset<8>(byte);
}

static void Uint256ToBits(const unsigned char *bytes, std::bitset<256> &bits) {
    for (int i = 0; i < 32; i++) {
        std::bitset<8> tempbits = ByteToBits(bytes[i]);
        for (int j = 0; j < 8; j++) {
            bits[i*8+j] = tempbits[j];
        }
    }
}

static void ArrayShiftRight(uint8_t array[], int len, int nShift) {
    int i = 0;
    uint8_t temp;
    do {
        i = (i+nShift) % len;
        temp = array[i];
        array[i] = array[0];
        array[0] = temp;
    } while(i);
}

static void New2GenCoeffMatrix(uint256 hash, unsigned int nBits, std::vector<uint8_t> &coeffM) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;

    unsigned char in[32], out[32];
    unsigned int count = 0, i, j ,k;
    uint8_t g[nTerms];
    std::bitset<256> bits;

    unsigned char tmp[64];
    pqcSha512(hash.begin(),32,tmp);
    pqcSha256(tmp,64,in);

	for (i = 0; i < mEquations ; i++) {
	    count = 0;
		do {
            pqcSha256(in,32,out);
            Uint256ToBits(out, bits);
            for (k = 0; k < 256; k++) {
                if(count < nTerms) {
                    g[count++] = (uint8_t)bits[k];
                 } else {
                     break;
                 }
             }
            for (j = 0; j < 32; j++) {
                in[j] = out[j];
            }
        } while(count < nTerms);
        for (j = 0; j < nTerms; j++) {
            coeffM[i*nTerms+j] = g[j];
        }
    }
}

static void  NewGenCoeffMatrix(uint256 hash, unsigned int nBits, std::vector<uint8_t> &coeffM) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    //generate the first polynomial coefficients.
    unsigned char in[32], out[32];
    unsigned int count = 0, i, j ,k;
    uint8_t g[nTerms];
    std::bitset<256> bits;
    pqcSha256(hash.begin(),32,in);

	for (i = 0; i < mEquations ; i++) {
	    count = 0;
		do {
            pqcSha256(in,32,out);
            Uint256ToBits(out, bits);
            for (k = 0; k < 256; k++) {
                if(count < nTerms) {
                    g[count++] = (uint8_t)bits[k];
                 } else {
                     break;
                 }
             }
            for (j = 0; j < 32; j++) {
                in[j] = out[j];
            }
        } while(count < nTerms);
        for (j = 0; j < nTerms; j++) {
            coeffM[i*nTerms+j] = g[j];
        }
    }
}

static void  GenCoeffMatrix(uint256 hash, unsigned int nBits, std::vector<uint8_t> &coeffM) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;

    //generate the first polynomial coefficients.
    unsigned char in[32], out[32];
    unsigned int count = 0, i, j ,k;
    uint8_t g[nTerms];
    std::bitset<256> bits;
    pqcSha256(hash.begin(),32,in);
	
    do {
         pqcSha256(in,32,out);
         Uint256ToBits(out, bits);
         for (k = 0; k < 256; k++) {
             if(count < nTerms) {
                 g[count++] = (uint8_t)bits[k];
             } else {
                 break;
             }
         }
         for (j = 0; j < 32; j++) {
             in[j] = out[j];
         }
    } while(count < nTerms);

    //generate the rest polynomials coefficients by shiftint f[0] one bit
    for (i = 0; i < mEquations ; i++) {
        ArrayShiftRight(g, nTerms, 1);
        for (j = 0; j < nTerms; j++)
            coeffM[i*nTerms+j] = g[j];
    }

}

static void Uint256ToSolutionBits(uint8_t *x, unsigned int nUnknowns, uint256 nonce) {
    std::bitset<256> bitnonce;
    Uint256ToBits(nonce.begin(), bitnonce);
    if (nUnknowns < 256) {
        for (unsigned int i = 0; i < nUnknowns; i++) {
                x[i] = (uint8_t)bitnonce[i];
        }
    } else {
        for (unsigned int i = 0; i < 256; i++) {
            x[i] = (uint8_t)bitnonce[i];
        }
    }
}

// Define the binomial function to calculate number of terms in different degrees.
static int Binomial(int n, int m) {
    if (m == 0)
        return 1;
    else if (m == 1)
        return n;
    else if (m == 2)
        return n * (n - 1) >> 1;
    else
        return 0;
}

// Transform coefficientsMatrix into the structure required by exfes.
static void TransformDataStructure (int n, int e, std::vector<uint8_t> &coefficientsMatrix, int ***Eqs) {
    uint64_t offset = 0;
    for (int i=0; i<e; i++) {
        for (int j=0; j<Binomial(n, 2); j++)
            Eqs[i][2][j] = (int)coefficientsMatrix[offset+j];
        offset += Binomial(n, 2);
        for (int j=0; j<n; j++)
            Eqs[i][1][j] = (int)coefficientsMatrix[offset+j];
        offset += n;
        Eqs[i][0][0] = (int)coefficientsMatrix[offset];
        offset += 1;
    }
}

// Define a function to print equations for debugging.
void PrintEquation (int n, int e, int ***Eqs) {
    for (int i=0; i<e; i++) {
        std::cout<<"Eqs["<<i<<"] =  ";
        for (int j=2; j>=0; j--)
            for (int k=0; k<Binomial(n, j); k++)
                std::cout<<" "<<Eqs[i][j][k];
        std::cout<<std::endl;
    }
}

// Define a function set all equation elements to zero.
int ***CreateEquations (int n, int e) {
    int ***Eqs = (int ***)calloc(e, sizeof(int **)); // Create an array for saving coefficients for exfes.
    if (!Eqs) printf("malloc Eqs failure!\n");
    for (int i=0; i<e; i++) {
        Eqs[i] = (int **)calloc(3, sizeof(int *));
        if (!Eqs[i]) printf("malloc Eqs failure!\n");
        for (int j=0; j<3; j++) {
            Eqs[i][j] = (int *)calloc(Binomial(n, j), sizeof(int));
            if (!Eqs[i][j]) printf("malloc Eqs failure!\n");
        }
    }
    return Eqs;
}

void FreeEquations (int e, int ***Eqs) {
    if (Eqs) {
        for (int i=0; i<e; i++) {
            for (int j=0; j<3; j++) {
                if (Eqs[i][j])
                    free(Eqs[i][j]);
            }
            if (Eqs[i])
                free(Eqs[i]);
        }
        free(Eqs);
    }
}

uint8_t ***CreateEquationsUint8(int n, int e) {
    uint8_t ***Eqs = (uint8_t ***)calloc(e, sizeof(uint8_t **)); 
    if (!Eqs) printf("malloc Eqs failure!\n");
    for (int i=0; i<e; i++) {
        Eqs[i] = (uint8_t **)calloc(3, sizeof(uint8_t *));
        if (!Eqs[i]) printf("malloc Eqs failure!\n");
        for (int j=0; j<3; j++) {
            Eqs[i][j] = (uint8_t *)calloc(Binomial(n, j), sizeof(uint8_t));
            if (!Eqs[i][j]) printf("malloc Eqs failure!\n");
        }
    }
    return Eqs;
}
void FreeEquationsUint8(int e, uint8_t ***Eqs) {
    if (Eqs) {
        for (int i=0; i<e; i++) {
            for (int j=0; j<3; j++) {
                if (Eqs[i][j])
                    free(Eqs[i][j]);
            }
            if (Eqs[i])
                free(Eqs[i]);
        }
        free(Eqs);
    }
}
// Define a function set all array elements to zero.
uint64_t **CreateArray (uint64_t maxsol) {
    uint64_t **SolArray = (uint64_t **)calloc(maxsol, sizeof(uint64_t *)); // Create an array for exfes to store solutions.
    for (uint64_t i=0; i<maxsol; i++) {
        SolArray[i] = (uint64_t *)calloc(4, sizeof(uint64_t));
    }
    return SolArray;
}

void FreeArray (uint64_t maxsol, uint64_t **SolArray) {
    for (uint64_t i=0; i<maxsol; i++) {
        free(SolArray[i]);
    }
    free(SolArray);
}


// Define a function to print solutions obtained from exfes.
static int ReportSolution (uint64_t maxsol, uint64_t **SolArray, uint256 &s) {
    for (uint64_t i=0; i<maxsol; i++) {
        for (int j=3; j>=0; j--) {
            s = s << 64;
            s ^= SolArray[i][j];
        }
    }
    return 0;
}

uint64_t  ReverseUint64ByBits(uint64_t  num){
    unsigned int count = sizeof(num) * 8 - 1;
    uint64_t reverse_num = num;
    num >>= 1;
    while(num)
    {
       reverse_num <<= 1;
       reverse_num |= num & 1;
       num >>= 1;
       count--;
    }
    reverse_num <<= count;
    return reverse_num;
}
static void TransformTo3D(int n, int e, std::vector<uint8_t> &coefficientsMatrix, uint8_t ***Eqs) {
    uint64_t offset = 0;
    for (int i=0; i<e; i++) {
        for (int j=0; j<Binomial(n, 2); j++)
            Eqs[i][2][j] = (uint8_t)coefficientsMatrix[offset+j];
        offset += Binomial(n, 2);
        for (int j=0; j<n; j++)
            Eqs[i][1][j] = (uint8_t)coefficientsMatrix[offset+j];
        offset += n;
        Eqs[i][0][0] = (uint8_t)coefficientsMatrix[offset];
        offset += 1;
    }
}
static void TransTo1D(int n, int e, std::vector<uint8_t> &coefficientsMatrix, uint8_t ***Eqs) {
    uint64_t offset = 0;
    for (int i=0; i<e; i++) {
        for (int j=0; j<Binomial(n, 2); j++)
           coefficientsMatrix[offset+j]=Eqs[i][2][j] ;
        offset += Binomial(n, 2);
        for (int j=0; j<n; j++)
           coefficientsMatrix[offset+j] =  Eqs[i][1][j];
        offset += n;
        coefficientsMatrix[offset] = Eqs[i][0][0] ;
        offset += 1;
    }
}
static void SolutionBitsToUint256(uint8_t *x, unsigned int nUnknowns, uint256 &nonce) {
    nonce = 0;
	uint256 base = 1;
	for (int i = 0; i < nUnknowns; i++) {
        if (x[i]) {
            nonce  = (base<<i) | nonce;
		}
	}
}
static void AddQuadraticTerms(std::vector<uint8_t> &src, unsigned int n, unsigned int m, std::vector<uint32_t> &dest) {
	int offset = 0 , count = 0;
	for (unsigned int k = 0; k < m; k++) {
        for (unsigned int i = 0; i < n; i++) {
            for (unsigned int j = i; j < n; j++) {
                if (i == j) {
                    dest[offset++] = 0;
			    } else {
                    dest[offset++] = src[count++];
			    }
		    }
	    }
		for (unsigned int i = 0; i < n ; i++) {
            dest[offset++] = src[count++];
	    }
	    dest[offset++] = src[count++];
	}
}
static void LexToGradeReverseLex(std::vector<uint32_t> &src, unsigned int n, unsigned int m, uint32_t *coefficients) {
    unsigned int nTerms = 1 +  (((n+1)*n)>>1) + n;
	int offset = 0;
    for (unsigned int k = 0; k < m; k++) {
		int count = 0;
        for (unsigned int i = 0; i < n; i++) {
            for (unsigned int j = 0; j <= i; j++) {
                coefficients[offset++] = (uint32_t)src[k*nTerms + j*n -(((j+1)*j)>>1) + i];
				count++;
			}
		}
		for (unsigned int i = 0; i < n; i++) {
            coefficients[offset++] = (uint32_t)src[k*nTerms +count + i];
		}
		coefficients[offset++] = (uint32_t)src[k*nTerms + count + n];
	}
}

#ifdef USE_GPU
static uint256 GPUMinerSearchSolution(uint256 hash, unsigned int nBits, uint256 randomNonce, CBlockIndex* pindexPrev, int threadID, int threadCount) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits + 8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);
	if (pindexPrev->nHeight < 25216) {
        GenCoeffMatrix(hash, nBits, coeffMatrix);
	} else if (pindexPrev->nHeight < RAINBOWFORkHEIGHT-1) {
        NewGenCoeffMatrix(hash, nBits, coeffMatrix);
	} else {
        New2GenCoeffMatrix(hash, nBits, coeffMatrix);
    }
	
    uint8_t maskArray[nUnknowns];
    Uint256ToSolutionBits(maskArray, nUnknowns, randomNonce);
    uint8_t ***Eqs = CreateEquationsUint8(nUnknowns, mEquations);
    uint64_t startPoint[4];
    for (int width = 0; width < 4; width++) {
        startPoint[width] = randomNonce.Get64(width);
    }
    TransformTo3D(nUnknowns, mEquations, coeffMatrix, Eqs);
    for (int i=0; i<mEquations; i++) {
        for (int j=0; j<nUnknowns; j++)
            Eqs[i][0][0] ^= Eqs[i][1][j] & TM(startPoint, j);
        int offset = 0;
        for (int j=0; j<nUnknowns-1; j++)
            for (int k=j+1; k<nUnknowns; k++) {
                Eqs[i][0][0] ^= Eqs[i][2][offset] & TM(startPoint, j) & TM(startPoint, k);
                Eqs[i][1][j] ^= Eqs[i][2][offset] & TM(startPoint, k);
                Eqs[i][1][k] ^= Eqs[i][2][offset] & TM(startPoint, j);
                offset += 1;
            }
    }
    uint8_t *** EqsCopy = CreateEquationsUint8(nUnknowns, mEquations);
    int nFix = 8;
    if (nBits <= 40) {
        nFix = 8;
    } else {
        nFix = nUnknowns - 40;
    }
    uint64_t balance = (uint64_t)ceil((uint64_t)(1<<nFix)/(uint64_t)threadCount);
    uint64_t upBound = std::min((uint64_t)(threadID+1)*balance, (uint64_t)(1<<nFix));
    uint64_t downBound = threadID*balance;
    int p = nUnknowns - nFix;
    int npartial;
    uint8_t fixvalue;
    uint256 nonce = 0;
    uint8_t w[nUnknowns];
    uint64_t solm = 0;
    unsigned int newNumVariables = nUnknowns - nFix;
    unsigned int newNumEquations = mEquations;
    unsigned int newNumTerms = 1 + (newNumVariables+1)*(newNumVariables)/2;
    uint64_t foundSolution = 0;
    uint32_t *coefficients = (uint32_t *)malloc((mEquations*(newNumTerms + newNumVariables))*sizeof(uint32_t));
    if (coefficients == NULL)
        printf("ERROR: SearchSolution malloc failure!");
    int searchTimes = 0;

    for (solm = downBound; solm < upBound; solm++) {
	    if (boost::this_thread::interruption_requested() || pindexPrev != pindexBest)
            break;
	    npartial = nUnknowns;
	    for (int i=0; i<mEquations; i++)
	        for (int j=0; j<3; j++)
		        for (int k=0; k<Binomial(nUnknowns, j); k++)
		            EqsCopy[i][j][k] = Eqs[i][j][k];
	    while (npartial != p) {
	        fixvalue = (uint8_t)((solm >> (nUnknowns - npartial)) & 1);
	        for (int i=0; i<mEquations; i++) {
		        for (int j=0; j<npartial-1; j++)
		            EqsCopy[i][1][j+1] ^= EqsCopy[i][2][j] & fixvalue;
        	    EqsCopy[i][0][0] ^= EqsCopy[i][1][0] & fixvalue;
		        for (int j=0; j<npartial-1; j++)
		            EqsCopy[i][1][j] = EqsCopy[i][1][j+1];
		        for (int j=0; j<Binomial(npartial-1, 2); j++)
			        EqsCopy[i][2][j] = EqsCopy[i][2][j+npartial-1];
	        }
	        npartial -= 1;
	    }
	    std::vector<uint8_t> dest;
	    dest.resize(mEquations*newNumTerms);
	    TransTo1D(newNumVariables, newNumEquations, dest, EqsCopy);
	    std::vector<uint32_t> withQudraticTerms;
	    withQudraticTerms.resize(mEquations*(newNumTerms + newNumVariables));
	    AddQuadraticTerms(dest, newNumVariables, mEquations, withQudraticTerms);
	    LexToGradeReverseLex(withQudraticTerms,newNumVariables,mEquations,coefficients);
        foundSolution = GPUSearchSolution(coefficients, newNumVariables, mEquations);
        uint256 fixSolution = uint256(foundSolution);
        uint8_t x[newNumVariables];
        Uint256ToSolutionBits(x, newNumVariables, fixSolution);
        uint8_t z[nFix];
        uint64_t fixv[1];
        fixv[0] = solm;
        for (unsigned int i = 0; i < nFix; i++) {
            z[i] = TM(fixv, i);
        }
        for (unsigned int i = 0; i < nUnknowns; i++) {
            if (i < nFix) {
                w[i] = z[i]^maskArray[i] ;
            } else {
                w[i] = x[i-nFix]^maskArray[i];
            }
        }
        nonce = 0;
        SolutionBitsToUint256(w, nUnknowns,nonce);
		uint256 prevblockhash = 0;
		if (pindexPrev->GetBlockHash() == hashGenesisBlock || pindexPrev->pprev->GetBlockHash() == hashGenesisBlock) {
		    prevblockhash = 0;
		} else {
            prevblockhash = pindexPrev->GetBlockHash();
		}	
        if ( CheckSolution(hash, nBits,prevblockhash, pindexPrev->nVersion,nonce))
           break;
    }

    FreeEquationsUint8(mEquations, EqsCopy);
    FreeEquationsUint8(mEquations, Eqs);
    free(coefficients);

    return nonce;
}
#endif

uint256 SerchSolution(uint256 hash, unsigned int nBits, uint256 randomNonce, CBlockIndex* pindexPrev, int threadID, int threadCount) {
    uint256 nonce = 0;
#ifdef USE_GPU
    nonce = GPUMinerSearchSolution(hash, nBits, randomNonce, pindexPrev, threadID, threadCount);
#else
   	unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits + 8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);
	if (pindexPrev->nHeight < 25216) {
        GenCoeffMatrix(hash, nBits, coeffMatrix);
	} else if (pindexPrev->nHeight < RAINBOWFORkHEIGHT-1) {
        NewGenCoeffMatrix(hash, nBits, coeffMatrix);
	} else {
        New2GenCoeffMatrix(hash, nBits, coeffMatrix);
    }
	
    int ***Eqs = CreateEquations(nUnknowns, mEquations);
    uint64_t maxsol = 1; // The solver only returns maxsol solutions. Other solutions will be discarded.
    uint64_t **SolArray = CreateArray(maxsol); // Set all array elements to zero.
    //serch
    uint64_t startPoint[4];
    for (int width = 0; width < 4; width++) {
        startPoint[width] = randomNonce.Get64(width);
    }
    int nSearchVariables = 0;
    if (nUnknowns > 62) {
        nSearchVariables = nUnknowns - 62;
    } else {
        nSearchVariables = 0;
    }
    TransformDataStructure(nUnknowns, mEquations, coeffMatrix, Eqs); // Transform coefficientsMatrix into the structure required by exfes.
    exfes(nSearchVariables, nUnknowns, mEquations, startPoint, maxsol, Eqs, SolArray, pindexPrev, threadID, threadCount); // Solve equations by exfes.
    ReportSolution(maxsol, SolArray, nonce); // Report obtained solutions in uint256 format.
    FreeEquations(mEquations, Eqs);
    FreeArray(maxsol, SolArray);
#endif
    return nonce;

}

bool CheckSolution(uint256 hash, unsigned int nBits, uint256 prevblockhash, int nblockversion, uint256 nNonce) {
    unsigned int mEquations = nBits;
    unsigned int nUnknowns = nBits+8;
    unsigned int nTerms = 1 + (nUnknowns+1)*(nUnknowns)/2;
    std::vector<uint8_t> coeffMatrix;
    coeffMatrix.resize(mEquations*nTerms);

    // Get prev block index
    CBlockIndex* pindexPrev = NULL;
    int height = 0;
	uint256 initHash = 0;
    if (prevblockhash != initHash) {
        std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(prevblockhash);
        if (mi == mapBlockIndex.end()) {
            if (nBestHeight > RAINBOWFORkHEIGHT + 20) {
                height = RAINBOWFORkHEIGHT;
            } else {
                if (nblockversion < BLOCK_VERSION2_BEFORE_FORK) {
                    height = 0;
                } else if (nblockversion < BLOCK_CURRENT_VERSION) {
                    height = 25217;
                } else {
                    height = RAINBOWFORkHEIGHT;
                }
            }
        } else {
            pindexPrev = (*mi).second;
            height = pindexPrev->nHeight+1;
        }
    } else {
        height = 0;
	}
    
    if (height < 25217) {
	    GenCoeffMatrix(hash, nBits, coeffMatrix);
    } else if (height < RAINBOWFORkHEIGHT) {
        NewGenCoeffMatrix(hash, nBits, coeffMatrix);
	} else {
        New2GenCoeffMatrix(hash, nBits, coeffMatrix);
    }
    unsigned int i, j, k, count;
    uint8_t x[nUnknowns], tempbit;
    Uint256ToSolutionBits(x, nUnknowns, nNonce);

    for (k = 0; k < mEquations; k++) {
        tempbit = 0;
        count = 0;
        for (i = 0; i < nUnknowns; i++) {
            for (j = i+1; j < nUnknowns; j++) {
                tempbit ^= (coeffMatrix[k*nTerms+count] * x[i] * x[j]);
                count++;
            }
        }
        for (i = 0; i < nUnknowns; i++) {
            tempbit ^= (coeffMatrix[k*nTerms+count]* x[i]);
            count++;
        }
        tempbit ^= (coeffMatrix[k*nTerms+count]);
        if (tempbit != 0) {
            return false;
        }
    }

    return true;

}


bool CheckProofOfWork(uint256 hash, unsigned int nBits, uint256 prevblockhash, int nblockversion, uint256 nNonce)
{
    unsigned int bnTarget = nBits;

    // Check range
    if (bnTarget <= 0 || bnTarget > bnPowUpLimit)
        return error("CheckProofOfWork() : nBits below minimum work");
    if (nNonce == uint256("0xffffffffffffffffffffffffffffffff") && nblockversion > 1)
       return false;
    // Check proof of work matches claimed amount
    if (!CheckSolution(hash, nBits, prevblockhash, nblockversion, nNonce))
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

int static FormatHashBlocks(void* pbuffer, unsigned int len)
{
    unsigned char* pdata = (unsigned char*)pbuffer;
    unsigned int blocks = 1 + ((len + 8) / 64);
    unsigned char* pend = pdata + 64 * blocks;
    memset(pdata + len, 0, 64 * blocks - len);
    pdata[len] = 0x80;
    unsigned int bits = len * 8;
    pend[-1] = (bits >> 0) & 0xff;
    pend[-2] = (bits >> 8) & 0xff;
    pend[-3] = (bits >> 16) & 0xff;
    pend[-4] = (bits >> 24) & 0xff;
    return blocks;
}

static const unsigned int pSHA256InitState[8] =
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

void SHA256Transform(void* pstate, void* pinput, const void* pinit)
{
    Sha256 ctx;
    unsigned char data[64];

    sha256Init(&ctx);

    for (int i = 0; i < 16; i++)
        ((uint32_t*)data)[i] = ByteReverse(((uint32_t*)pinput)[i]);

    for (int i = 0; i < 8; i++)
        ctx.state[i] = ((uint32_t*)pinit)[i];

    sha256Process(&ctx, data, sizeof(data));
    for (int i = 0; i < 8; i++)
        ((uint32_t*)pstate)[i] = ctx.state[i];
}


// Some explaining would be appreciated
class COrphan
{
public:
    CTransaction* ptx;
    set<uint256> setDependsOn;
    double dPriority;
    double dFeePerKb;

    COrphan(CTransaction* ptxIn)
    {
        ptx = ptxIn;
        dPriority = dFeePerKb = 0;
    }

    void print() const
    {
        printf("COrphan(hash=%s, dPriority=%.1f, dFeePerKb=%.1f)\n",
               ptx->GetHash().ToString().c_str(), dPriority, dFeePerKb);
        BOOST_FOREACH(uint256 hash, setDependsOn)
            printf("   setDependsOn %s\n", hash.ToString().c_str());
    }
};


// We want to sort transactions by priority and fee, so:
typedef boost::tuple<double, double, CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;
public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) { }
    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee)
        {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        }
        else
        {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock)
    {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight+1; // Height first in coinbase required for block.version=2
    pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce));
    assert(pblock->vtx[0].vin[0].scriptSig.size() <= 200*1024);

    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}


bool CheckWork(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey)
{
    if (pblock->hashPrevBlock != hashBestChain)
        return false;

    //// debug print
    //printf("RaqcoinMiner:\n");
    //pblock->print();
    //printf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue).c_str());

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != hashBestChain) {
            printf("\n\n ***********hashBestChain****************** %s \n\n", hashBestChain.ToString().c_str());
            printf("\n\n ***********pblock->hashPrevBlock****************** %s \n\n", pblock->hashPrevBlock.ToString().c_str());
            return error("RaqcoinMiner : generated block is stale");
        }

        // Remove key from key pool
        reservekey.KeepKey();

        // Track how many getdata requests this block gets
        {
            LOCK(wallet.cs_wallet);
            wallet.mapRequestCount[pblock->GetHash()] = 0;
        }

        // Process this block the same as if we had received it from another node
        CValidationState state;
        if (!ProcessBlock(state, NULL, pblock))
            return error("RaqcoinMiner : ProcessBlock, block not accepted");
    }

    return true;
}

CBlockTemplate* CreateNewBlock(CReserveKey& reservekey)
{
    // Create new block
    std::unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if(!pblocktemplate.get())
        return NULL;
    CBlock *pblock = &pblocktemplate->block; // pointer for convenience

    // Create coinbase tx
    CTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);

    CKeyID keyId;
    if (!reservekey.GetMinerAddress(keyId))
        return NULL;
    txNew.vout[0].scriptPubKey.SetDestination(keyId);

    // Add our coinbase tx as first transaction
    pblock->vtx.push_back(txNew);
    pblocktemplate->vTxFees.push_back(-1); // updated at end
    pblocktemplate->vTxSigOps.push_back(-1); // updated at end

    // Collect memory pool transactions into the block
    int64 nFees = 0;
    {
        if (boost::this_thread::interruption_requested()) {
            return NULL;
        }
        LOCK2(cs_main, mempool.cs);
        CBlockIndex* pindexPrev = pindexBest;
        CCoinsViewCache view(*pcoinsTip, true);

        unsigned int nBlockMaxSize = 0;
        if (pindexPrev->nHeight < RAINBOWFORkHEIGHT - 1) {
            nBlockMaxSize = GetArg("-blockmaxsize", MAX_BLOCK_SIZE_BEFORE_FORK);
        } else {
            nBlockMaxSize = GetArg("-blockmaxsize", MAX_BLOCK_SIZE);
        }

        unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
        nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

        // Priority order to process transactions
        list<COrphan> vOrphan; // list memory doesn't move
        map<uint256, vector<COrphan*> > mapDependers;
        bool fPrintPriority = GetBoolArg("-printpriority");

        // This vector will be sorted into a priority queue:
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (map<uint256, CTransaction>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi)
        {
            CTransaction& tx = (*mi).second;
            if (tx.IsCoinBase() || !tx.IsFinal())
                continue;

            COrphan* porphan = NULL;
            double dPriority = 0;
            int64 nTotalIn = 0;
            bool fMissingInputs = false;
            BOOST_FOREACH(const CTxIn& txin, tx.vin)
            {
                // Read prev transaction
                if (!view.HaveCoins(txin.prevout.hash))
                {
                    // This should never happen; all transactions in the memory
                    // pool should connect to either transactions in the chain
                    // or other transactions in the memory pool.
                    if (!mempool.mapTx.count(txin.prevout.hash))
                    {
                        printf("ERROR: mempool transaction missing input\n");
                        if (fDebug) assert("mempool transaction missing input" == 0);
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    // Has to wait for dependencies
                    if (!porphan)
                    {
                        // Use list for automatic deletion
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx[txin.prevout.hash].vout[txin.prevout.n].nValue;
                    continue;
                }
                const CCoins &coins = view.GetCoins(txin.prevout.hash);

                int64 nValueIn = coins.vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = pindexPrev->nHeight - coins.nHeight + 1;

                dPriority += (double)nValueIn * nConf;
            }
            if (fMissingInputs) continue;

            // Priority is sum(valuein * age) / txsize
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority /= nTxSize;

            // This is a more accurate fee-per-kilobyte than is used by the client code, because the
            // client code rounds up the size to the nearest 1K. That's good, because it gives an
            // incentive to create smaller transactions.
            double dFeePerKb =  double(nTotalIn-tx.GetValueOut()) / (double(nTxSize)/1000.0);

            if (porphan)
            {
                porphan->dPriority = dPriority;
                porphan->dFeePerKb = dFeePerKb;
            }
            else
                vecPriority.push_back(TxPriority(dPriority, dFeePerKb, &(*mi).second));
        }

        // Collect transactions into block
        uint64 nBlockSize = 1000;
        uint64 nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        while (!vecPriority.empty())
        {
            // Take highest priority transaction off the priority queue:
            double dPriority = vecPriority.front().get<0>();
            double dFeePerKb = vecPriority.front().get<1>();
            CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            // Size limits
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            // Legacy limits on sigOps:
            unsigned int nTxSigOps = tx.GetLegacySigOpCount();
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;

            // Skip low fee transactions if we're past the minimum block size:
            if (fSortedByFee && (dFeePerKb < CTransaction::nMinTxFee) && (nBlockSize + nTxSize >= nBlockPrioritySize))
            {
                continue;
            }

            // Prioritize by fee once past the priority size or we run out of high-priority
            // transactions:
            if (!fSortedByFee && ((nBlockSize + nTxSize >= nBlockPrioritySize) || (dPriority < COIN * 144 / 250)))
            {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            if (!tx.HaveInputs(view))
                continue;

            int64 nTxFees = tx.GetValueIn(view)-tx.GetValueOut();

            nTxSigOps += tx.GetP2SHSigOpCount(view);
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;

            CValidationState state;
            tx.vPubKeys.clear();
            if (!tx.CheckInputs(state, view, true, SCRIPT_VERIFY_P2SH))
                continue;

            CTxUndo txundo;
            uint256 hash = tx.GetHash();
            tx.UpdateCoins(state, view, txundo, pindexPrev->nHeight+1, hash);

            // Added
            pblock->vtx.push_back(tx);
            pblocktemplate->vTxFees.push_back(nTxFees);
            pblocktemplate->vTxSigOps.push_back(nTxSigOps);
            nBlockSize += nTxSize;
            ++nBlockTx;
            nBlockSigOps += nTxSigOps;
            nFees += nTxFees;

            if (fPrintPriority)
            {
                printf("priority %.1f feeperkb %.1f txid %s\n",
                       dPriority, dFeePerKb, tx.GetHash().ToString().c_str());
            }

            // Add transactions that depend on this one to the priority queue
            if (mapDependers.count(hash))
            {
                BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
                {
                    if (!porphan->setDependsOn.empty())
                    {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty())
                        {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->dFeePerKb, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
                    }
                }
            }
        }

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        printf("CreateNewBlock(): total size %" PRI64u "\n", nBlockSize);

        pblock->vtx[0].vout[0].nValue = GetBlockValue(pindexPrev->nHeight+1, nFees);
        pblocktemplate->vTxFees[0] = -nFees;

        // Fill in header
        pblock->hashPrevBlock  = pindexPrev->GetBlockHash();
        pblock->UpdateTime(pindexPrev);
        pblock->nBits          = GetNextWorkRequired(pindexPrev, pblock);
        pblock->nNonce         = 0;
        pblock->vtx[0].vin[0].scriptSig = CScript() << OP_0 << OP_0;
        pblocktemplate->vTxSigOps[0] = pblock->vtx[0].GetLegacySigOpCount();
        
        if (pindexPrev->nHeight < RAINBOWFORkHEIGHT - 1) {
            pblock->nVersion = BLOCK_VERSION2_BEFORE_FORK;
        }

        CBlockIndex indexDummy(*pblock);
        indexDummy.pprev = pindexPrev;
        indexDummy.nHeight = pindexPrev->nHeight + 1;
        CCoinsViewCache viewNew(*pcoinsTip, true);
        CValidationState state;
        if (!pblock->ConnectBlock(state, &indexDummy, viewNew, true))
            throw std::runtime_error("CreateNewBlock() : ConnectBlock failed");
    }

    return pblocktemplate.release();
}




void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1)
{
    //
    // Pre-build hash buffers
    //
    struct
    {
        struct unnamed2
        {
            int nVersion;
            uint256 hashPrevBlock;
            uint256 hashMerkleRoot;
            unsigned int nTime;
            unsigned int nBits;
            uint256 nNonce;
        }
        block;
        unsigned char pchPadding0[64];
        uint256 hash1;
        unsigned char pchPadding1[64];
    }
    tmp;
    memset(&tmp, 0, sizeof(tmp));

    tmp.block.nVersion       = pblock->nVersion;
    tmp.block.hashPrevBlock  = pblock->hashPrevBlock;
    tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;
    tmp.block.nTime          = pblock->nTime;
    tmp.block.nBits          = pblock->nBits;
    tmp.block.nNonce         = pblock->nNonce;

    FormatHashBlocks(&tmp.block, sizeof(tmp.block));
    FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));

    // Byte swap all the input buffer
    for (unsigned int i = 0; i < sizeof(tmp)/4; i++)
        ((unsigned int*)&tmp)[i] = ByteReverse(((unsigned int*)&tmp)[i]);

    // Precalc the first half of the first hash, which stays constant
    SHA256Transform(pmidstate, &tmp.block, pSHA256InitState);

    memcpy(pdata, &tmp.block, 128);
    memcpy(phash1, &tmp.hash1, 64);
}

uint256 getRandomNonce(unsigned int nBits, uint256 oldNonce) {
    static uint256 share_randomNonce;
    static boost::shared_mutex rwmutex;

    bool bChange = false;
    {
        boost::shared_lock<boost::shared_mutex> rlock(rwmutex);
        if (0 == share_randomNonce || oldNonce == share_randomNonce) {
            bChange = true;
        }
    }
    
    if (bChange) {
        boost::unique_lock<boost::shared_mutex> wlock(rwmutex);
        if (0 == share_randomNonce || oldNonce == share_randomNonce) {
            if (nBits + 8 <= 48) {
                share_randomNonce = (uint64)random_uint32_t();
            } else if ((nBits + 8 > 48) && (nBits + 8 <= 80)) {
                share_randomNonce = random_uint64_t();
            } else {
                share_randomNonce = GetRandHash();
            }
        }
    }

    return share_randomNonce;
}

void static RaqcoinMiner(CWallet *pwallet, int deviceID, int threadID, int threadCount)
{
    //printf("RaqcoinMiner started deviceID=%d, threadID=%d, threadCount=%d\n", deviceID, threadID, threadCount);
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("raqcoin-miner");

#ifdef USE_GPU
	SetDevice(deviceID);
#endif

    // Each thread has its own key and counter
    CReserveKey reservekey(pwallet);
    unsigned int nExtraNonce = 0;
    uint256 randomNonce = 0;

    try { while(true) {
        while (vNodes.empty()) {
            boost::this_thread::interruption_point();
            MilliSleep(1000);
        }
        //
        // Create new block
        //
        unsigned int nTransactionsUpdatedLast = nTransactionsUpdated;
        CBlockIndex* pindexPrev = pindexBest;

        std::unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock(reservekey));
        if (!pblocktemplate.get())
            return;
        CBlock *pblock = &pblocktemplate->block;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        printf("Running RaqcoinMiner with %" PRIszu " transactions in block (%u bytes)\n", pblock->vtx.size(),
               ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));


        //
        // Search
        //
        /*uint256 randomNonce = 0;
        if (pblock->nBits + 8 <= 48) {
            randomNonce = (uint64)random_uint32_t();
        } else if ((pblock->nBits + 8 > 48) && (pblock->nBits + 8 <= 80)) {
            randomNonce = random_uint64_t();
        } else {
            randomNonce = GetRandHash();
        }*/
        randomNonce = getRandomNonce(pblock->nBits, randomNonce);

        int64 nStart = GetTime();
        uint256 tempHash = pblock->hashPrevBlock ^ pblock->hashMerkleRoot;
        uint256 seedHash = Hash(BEGIN(tempHash), END(tempHash));
		uint256 prevblockhash = 0;
		if (pindexBest->GetBlockHash() == hashGenesisBlock || pindexPrev->pprev->GetBlockHash() == hashGenesisBlock) {
		    prevblockhash = 0;
		} else {
            prevblockhash = pindexPrev->GetBlockHash();
		}
        while(true)
        {
            // Solve the multivariable quadratic polynomial equations.
            uint256 nNonceFound = SerchSolution(seedHash, pblock->nBits, randomNonce, pindexPrev, threadID, threadCount);
            // Check if something found
            if (nNonceFound !=  uint256("0xffffffffffffffffffffffffffffffff"))
            {
                if (CheckSolution(seedHash, pblock->nBits, prevblockhash, pblock->nVersion, nNonceFound))
                {
                    // Found a solution
                    pblock->nNonce = nNonceFound;
                    SetThreadPriority(THREAD_PRIORITY_NORMAL);
                    CheckWork(pblock, *pwalletMain, reservekey);
                    SetThreadPriority(THREAD_PRIORITY_LOWEST);
                    break;
                }
            }

            // Check for stop or if block needs to be rebuilt
            boost::this_thread::interruption_point();
            if (vNodes.empty())
                break;
            if (nTransactionsUpdated != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                break;
            if (pindexPrev != pindexBest)
                break;

            // Update nTime every few seconds
            pblock->UpdateTime(pindexPrev);
        }
    } }
    catch (boost::thread_interrupted)
    {
        printf("RaqcoinMiner terminated\n");
        //throw;
    }
}

void GenerateRaqcoins(bool fGenerate, CWallet* pwallet)
{
    static boost::thread_group* minerThreads = NULL;

    if (minerThreads != NULL) {
        if (fGenerate) {
            return;
        }
        
        minerThreads->interrupt_all();
        minerThreads->join_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    int nThreads = GetArg("-genproclimit", -1);
    if (!fGenerate || 0 == nThreads) {
        return;
    }

    int tmpMultiple = 1;
#ifdef USE_GPU
    int deviceCount = GetDeviceCount();
    if (nThreads < 0) {
        nThreads = boost::thread::hardware_concurrency();
    }
    if (nThreads < deviceCount) {
        nThreads = deviceCount;
    }

    if (deviceCount <= 0 || nThreads <= 0) {
        return;
    }

    tmpMultiple = nThreads / deviceCount;
    nThreads = tmpMultiple * deviceCount;

    printf("    nThreads=%d, deviceCount=%d\n", nThreads, deviceCount);
#else
    if (nThreads < 0)
        nThreads = boost::thread::hardware_concurrency();
    
    if (0 == nThreads) {
        nThreads = 1;
    }
#endif

    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++) {
        minerThreads->create_thread(boost::bind(&RaqcoinMiner, pwallet, i/tmpMultiple, i, nThreads));
    }
}
