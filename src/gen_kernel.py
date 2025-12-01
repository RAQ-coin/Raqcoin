#!/usr/bin/env python3

import argparse, sys

# return the index of the coefficient c_ij in grevlex order
# (c_01 has index 0), assume i < j
def COEF(i, j):
  return ((j*(j-1)) >> 1) + i

# convert 01000 into 3, etc
def bit_to_idx(b, n):
  for i in range(n):
    if (b >> i) == 1:
      return i

  return 0


parser = argparse.ArgumentParser(description='Generate GPU kernel for fast exhaustive search.',
                formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-k', dest='k', type=int, required=True,
          help='number of variables per thread')
parser.add_argument('-u', '--unroll', dest='unroll', type=int, required=True,
          help='level to unroll code (>0)')
args = parser.parse_args()

k = args.k
unroll = args.unroll

if not unroll > 0:
  sys.stderr.write('"unroll" must be at least 1!\n')
  sys.exit(-1)

if unroll >= k:
  sys.stderr.write('"unroll" must be samller than k!\n')
  sys.exit(-1)


print("#define COEF(I,J) ((((J)*((J)-1))>>1) + (I))")
print("")

#print("""__device__ uint32_t ctz(uint32_t v)
#{
#  static const int MultiplyDeBruijnBitPosition[32] = 
#  {
#      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
#        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
#  };
#
#  return MultiplyDeBruijnBitPosition[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
#}\n""")


print("#define idx (blockIdx.x*blockDim.x + threadIdx.x)")
print("")
# start of the kernel function
print("__global__ void guess(uint32_t *deg1, uint32_t *result, uint32_t num_threads)")
print("{")
#print("  uint32_t idx = blockIdx.x*blockDim.x + threadIdx.x;")
print("  uint32_t x = {0}; // for round zero".format(1 << (unroll-1)))
print("  uint32_t y = 0;")
print("  uint32_t z = 0;")
print("  uint32_t sol = 0;")
print("  uint32_t sol_cnt = 0;")
print("  uint32_t block = 0;")
print("  uint32_t tmp = 0;")
print("")

# initialize diff and res (we can predict when a bit is first flipped)
print("  uint32_t diff0 = deg1[num_threads * 0 + idx];")

for i in range(1,unroll):
  print("  uint32_t diff{0} = ".format(i), end='')
  print("deg1[num_threads * {0} + idx] ^ deg2_block[{1}] ^ deg2_block[{0}];".format(i, COEF(i-1, i)))

print("")
print("  uint32_t diff[{0}];".format(k-unroll))

for i in range(unroll,k):
  print("  diff[{0}-{1}] = ".format(i, unroll), end='')
  print("deg1[num_threads * {0} + idx] ^ deg2_block[{1}] ^ deg2_block[{0}];".format(i, COEF(i-1, i)))

print("")
print("  // undo changes of first round")
print("  uint32_t res = deg1[num_threads * {0} + idx] ^ diff0 ^ deg2_block[0];".format(k))
print("")

print("  __syncthreads();")
print("")


# main loop
print("  for (uint32_t rounds = 0; rounds < (1 << {1}); rounds += (1 << {0}))".format(unroll, k))
print("  {")

print("    tmp = (rounds & (rounds-1));")
print("    y = rounds ^ tmp;")
print("    x ^= (y ^ {0});".format(1 << (unroll-1))) # important!
print("    z = tmp ^ (tmp & (tmp-1));")

print("")
print("    uint32_t y_pos = y == 0 ? 0 : __ffs(y) - 1;")
print("    uint32_t z_pos = z == 0 ? 0 : __ffs(z) - 1;")
#print("    uint32_t y_pos = ctz(y);")
#print("    uint32_t z_pos = ctz(z);")
print("")
print("    block = y_pos * (y_pos-1) / 2;")
print("")
print("    if (y_pos == 0)")
print("    {")
print("      diff0 ^= deg2_block[COEF(y_pos, z_pos)];")
print("      res ^= diff0;")
print("    }")
print("    else")
print("    {")
print("      diff[y_pos - {0}] ^= deg2_block[COEF(y_pos, z_pos)];".format(unroll))
print("      res ^= diff[y_pos - {0}];".format(unroll))
print("    }")
print("")
print("    if (res == 0) sol_cnt++;")
print("    if (res == 0) sol = x;")
print("")

print("    // unrolled loop")

x = 0

for i in range(1, 1 << unroll):
  y = (i ^ (i & (i - 1)))
  x ^= y
  tmp = y ^ i
  z = tmp ^ (tmp & (tmp - 1))

  y = bit_to_idx(y, unroll)
  z = bit_to_idx(z, unroll)

  if (i > 1):
    print("")

  if z == 0: # first flip in block
    print("    diff{0} ^= deg2_block[block++];".format(y))
  else:
    print("    diff{0} ^= deg2_block[{1}];".format(y, COEF(y,z)))

  print("    res ^= diff{0};".format(y))

  print("    if (res == 0) sol = {0} ^ x;".format(x))
  print("    if (res == 0) sol_cnt++;")

print("  }")

print("")
print("  if (sol_cnt > 1) sol |= 0x80000000;")
print("  result[idx] = sol;")

print("}")
print("")

