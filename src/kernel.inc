#define COEF(I,J) ((((J)*((J)-1))>>1) + (I))

#define idx (blockIdx.x*blockDim.x + threadIdx.x)

__global__ void guess(uint32_t *deg1, uint32_t *result, uint32_t num_threads)
{
  uint32_t x = 128; // for round zero
  uint32_t y = 0;
  uint32_t z = 0;
  uint32_t sol = 0;
  uint32_t sol_cnt = 0;
  uint32_t block = 0;
  uint32_t tmp = 0;

  uint32_t diff0 = deg1[num_threads * 0 + idx];
  uint32_t diff1 = deg1[num_threads * 1 + idx] ^ deg2_block[0] ^ deg2_block[1];
  uint32_t diff2 = deg1[num_threads * 2 + idx] ^ deg2_block[2] ^ deg2_block[2];
  uint32_t diff3 = deg1[num_threads * 3 + idx] ^ deg2_block[5] ^ deg2_block[3];
  uint32_t diff4 = deg1[num_threads * 4 + idx] ^ deg2_block[9] ^ deg2_block[4];
  uint32_t diff5 = deg1[num_threads * 5 + idx] ^ deg2_block[14] ^ deg2_block[5];
  uint32_t diff6 = deg1[num_threads * 6 + idx] ^ deg2_block[20] ^ deg2_block[6];
  uint32_t diff7 = deg1[num_threads * 7 + idx] ^ deg2_block[27] ^ deg2_block[7];

  uint32_t diff[12];
  diff[8-8] = deg1[num_threads * 8 + idx] ^ deg2_block[35] ^ deg2_block[8];
  diff[9-8] = deg1[num_threads * 9 + idx] ^ deg2_block[44] ^ deg2_block[9];
  diff[10-8] = deg1[num_threads * 10 + idx] ^ deg2_block[54] ^ deg2_block[10];
  diff[11-8] = deg1[num_threads * 11 + idx] ^ deg2_block[65] ^ deg2_block[11];
  diff[12-8] = deg1[num_threads * 12 + idx] ^ deg2_block[77] ^ deg2_block[12];
  diff[13-8] = deg1[num_threads * 13 + idx] ^ deg2_block[90] ^ deg2_block[13];
  diff[14-8] = deg1[num_threads * 14 + idx] ^ deg2_block[104] ^ deg2_block[14];
  diff[15-8] = deg1[num_threads * 15 + idx] ^ deg2_block[119] ^ deg2_block[15];
  diff[16-8] = deg1[num_threads * 16 + idx] ^ deg2_block[135] ^ deg2_block[16];
  diff[17-8] = deg1[num_threads * 17 + idx] ^ deg2_block[152] ^ deg2_block[17];
  diff[18-8] = deg1[num_threads * 18 + idx] ^ deg2_block[170] ^ deg2_block[18];
  diff[19-8] = deg1[num_threads * 19 + idx] ^ deg2_block[189] ^ deg2_block[19];

  // undo changes of first round
  uint32_t res = deg1[num_threads * 20 + idx] ^ diff0 ^ deg2_block[0];

  __syncthreads();

  for (uint32_t rounds = 0; rounds < (1 << 20); rounds += (1 << 8))
  {
    tmp = (rounds & (rounds-1));
    y = rounds ^ tmp;
    x ^= (y ^ 128);
    z = tmp ^ (tmp & (tmp-1));

    uint32_t y_pos = y == 0 ? 0 : __ffs(y) - 1;
    uint32_t z_pos = z == 0 ? 0 : __ffs(z) - 1;

    block = y_pos * (y_pos-1) / 2;

    if (y_pos == 0)
    {
      diff0 ^= deg2_block[COEF(y_pos, z_pos)];
      res ^= diff0;
    }
    else
    {
      diff[y_pos - 8] ^= deg2_block[COEF(y_pos, z_pos)];
      res ^= diff[y_pos - 8];
    }

    if (res == 0) sol_cnt++;
    if (res == 0) sol = x;

    // unrolled loop
    diff0 ^= deg2_block[block++];
    res ^= diff0;
    if (res == 0) sol = 1 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[block++];
    res ^= diff1;
    if (res == 0) sol = 3 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 2 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[block++];
    res ^= diff2;
    if (res == 0) sol = 6 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 7 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 5 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 4 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[block++];
    res ^= diff3;
    if (res == 0) sol = 12 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 13 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 15 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 14 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 10 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 11 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 9 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 8 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[block++];
    res ^= diff4;
    if (res == 0) sol = 24 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 25 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 27 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 26 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 30 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 31 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 29 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 28 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 20 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 21 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 23 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 22 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 18 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 19 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 17 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 16 ^ x;
    if (res == 0) sol_cnt++;

    diff5 ^= deg2_block[block++];
    res ^= diff5;
    if (res == 0) sol = 48 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[10];
    res ^= diff0;
    if (res == 0) sol = 49 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[11];
    res ^= diff1;
    if (res == 0) sol = 51 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 50 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[12];
    res ^= diff2;
    if (res == 0) sol = 54 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 55 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 53 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 52 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[13];
    res ^= diff3;
    if (res == 0) sol = 60 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 61 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 63 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 62 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 58 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 59 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 57 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 56 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[14];
    res ^= diff4;
    if (res == 0) sol = 40 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 41 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 43 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 42 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 46 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 47 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 45 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 44 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 36 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 37 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 39 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 38 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 34 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 35 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 33 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 32 ^ x;
    if (res == 0) sol_cnt++;

    diff6 ^= deg2_block[block++];
    res ^= diff6;
    if (res == 0) sol = 96 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[15];
    res ^= diff0;
    if (res == 0) sol = 97 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[16];
    res ^= diff1;
    if (res == 0) sol = 99 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 98 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[17];
    res ^= diff2;
    if (res == 0) sol = 102 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 103 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 101 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 100 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[18];
    res ^= diff3;
    if (res == 0) sol = 108 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 109 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 111 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 110 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 106 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 107 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 105 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 104 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[19];
    res ^= diff4;
    if (res == 0) sol = 120 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 121 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 123 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 122 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 126 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 127 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 125 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 124 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 116 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 117 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 119 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 118 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 114 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 115 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 113 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 112 ^ x;
    if (res == 0) sol_cnt++;

    diff5 ^= deg2_block[20];
    res ^= diff5;
    if (res == 0) sol = 80 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[10];
    res ^= diff0;
    if (res == 0) sol = 81 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[11];
    res ^= diff1;
    if (res == 0) sol = 83 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 82 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[12];
    res ^= diff2;
    if (res == 0) sol = 86 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 87 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 85 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 84 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[13];
    res ^= diff3;
    if (res == 0) sol = 92 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 93 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 95 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 94 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 90 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 91 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 89 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 88 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[14];
    res ^= diff4;
    if (res == 0) sol = 72 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 73 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 75 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 74 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 78 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 79 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 77 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 76 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 68 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 69 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 71 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 70 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 66 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 67 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 65 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 64 ^ x;
    if (res == 0) sol_cnt++;

    diff7 ^= deg2_block[block++];
    res ^= diff7;
    if (res == 0) sol = 192 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[21];
    res ^= diff0;
    if (res == 0) sol = 193 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[22];
    res ^= diff1;
    if (res == 0) sol = 195 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 194 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[23];
    res ^= diff2;
    if (res == 0) sol = 198 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 199 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 197 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 196 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[24];
    res ^= diff3;
    if (res == 0) sol = 204 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 205 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 207 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 206 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 202 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 203 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 201 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 200 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[25];
    res ^= diff4;
    if (res == 0) sol = 216 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 217 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 219 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 218 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 222 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 223 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 221 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 220 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 212 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 213 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 215 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 214 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 210 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 211 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 209 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 208 ^ x;
    if (res == 0) sol_cnt++;

    diff5 ^= deg2_block[26];
    res ^= diff5;
    if (res == 0) sol = 240 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[10];
    res ^= diff0;
    if (res == 0) sol = 241 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[11];
    res ^= diff1;
    if (res == 0) sol = 243 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 242 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[12];
    res ^= diff2;
    if (res == 0) sol = 246 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 247 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 245 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 244 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[13];
    res ^= diff3;
    if (res == 0) sol = 252 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 253 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 255 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 254 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 250 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 251 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 249 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 248 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[14];
    res ^= diff4;
    if (res == 0) sol = 232 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 233 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 235 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 234 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 238 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 239 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 237 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 236 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 228 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 229 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 231 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 230 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 226 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 227 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 225 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 224 ^ x;
    if (res == 0) sol_cnt++;

    diff6 ^= deg2_block[27];
    res ^= diff6;
    if (res == 0) sol = 160 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[15];
    res ^= diff0;
    if (res == 0) sol = 161 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[16];
    res ^= diff1;
    if (res == 0) sol = 163 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 162 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[17];
    res ^= diff2;
    if (res == 0) sol = 166 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 167 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 165 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 164 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[18];
    res ^= diff3;
    if (res == 0) sol = 172 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 173 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 175 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 174 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 170 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 171 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 169 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 168 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[19];
    res ^= diff4;
    if (res == 0) sol = 184 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 185 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 187 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 186 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 190 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 191 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 189 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 188 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 180 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 181 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 183 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 182 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 178 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 179 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 177 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 176 ^ x;
    if (res == 0) sol_cnt++;

    diff5 ^= deg2_block[20];
    res ^= diff5;
    if (res == 0) sol = 144 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[10];
    res ^= diff0;
    if (res == 0) sol = 145 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[11];
    res ^= diff1;
    if (res == 0) sol = 147 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 146 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[12];
    res ^= diff2;
    if (res == 0) sol = 150 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 151 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 149 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 148 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[13];
    res ^= diff3;
    if (res == 0) sol = 156 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 157 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 159 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 158 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 154 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 155 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 153 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 152 ^ x;
    if (res == 0) sol_cnt++;

    diff4 ^= deg2_block[14];
    res ^= diff4;
    if (res == 0) sol = 136 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[6];
    res ^= diff0;
    if (res == 0) sol = 137 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[7];
    res ^= diff1;
    if (res == 0) sol = 139 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 138 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[8];
    res ^= diff2;
    if (res == 0) sol = 142 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 143 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 141 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 140 ^ x;
    if (res == 0) sol_cnt++;

    diff3 ^= deg2_block[9];
    res ^= diff3;
    if (res == 0) sol = 132 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[3];
    res ^= diff0;
    if (res == 0) sol = 133 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[4];
    res ^= diff1;
    if (res == 0) sol = 135 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 134 ^ x;
    if (res == 0) sol_cnt++;

    diff2 ^= deg2_block[5];
    res ^= diff2;
    if (res == 0) sol = 130 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[1];
    res ^= diff0;
    if (res == 0) sol = 131 ^ x;
    if (res == 0) sol_cnt++;

    diff1 ^= deg2_block[2];
    res ^= diff1;
    if (res == 0) sol = 129 ^ x;
    if (res == 0) sol_cnt++;

    diff0 ^= deg2_block[0];
    res ^= diff0;
    if (res == 0) sol = 128 ^ x;
    if (res == 0) sol_cnt++;
  }

  if (sol_cnt > 1) sol |= 0x80000000;
  result[idx] = sol;
}

