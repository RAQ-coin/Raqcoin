/*****************************************************************************
 *       Copyright (C) 2012 Ralf Zimmermann <Ralf.Zimmermann@rub.de>
 *       Copyright (C) 2012 Charles Bouillaguet <charles.bouillaguet@gmail.com>

 *
 * Distributed under the terms of the GNU General Public License (GPL)
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *                 http://www.gnu.org/licenses/
 *****************************************************************************/
#ifndef IDX_LUT_H
#define IDX_LUT_H

#include <stdint.h>

#ifdef __cplusplus 

extern "C"{

#endif

/* the idx_LUT.c file most importantly implements two hashing scheme
   for boolean monomials. It implements a specific perfect hashing
   scheme for boolean monomials of degree at most d, which is used in
   the enumeration kernels, and a simple hashing scheme for booleans
   monomials without degree restriction, which is used in the Moebius
   transform kernel(s).

   Let N(n,d) be the number of boolean monomials of degree at most d
   in n variables. It is well-known that:

   N(n,d) = binomial(n,0) + ... + binomial(n,d)

   The first hashing scheme implemented in this file turns a boolean
   monomial in n variables of degree at most d into an integer in the
   range [0; N(n,d)[. As a by-product, it induces an order on
   monomial, which is the inverse lexicographic order ('invlex' in
   Sage).  It is the same as the lexicographic order with reversed
   variables. This order is examplified by the following sequence of
   all degree-4 monomials over 5 variables :

   0 []
   1 [0]
   2 [1]
   3 [0, 1]
   4 [2]
   5 [0, 2]
   6 [1, 2]
   7 [0, 1, 2]
   8 [3]
   9 [0, 3]
  10 [1, 3]
  11 [0, 1, 3]
  12 [2, 3]
  13 [0, 2, 3]
  14 [1, 2, 3]
  15 [0, 1, 2, 3]
  16 [4]
  17 [0, 4]
  18 [1, 4]
  19 [0, 1, 4]
  20 [2, 4]
  21 [0, 2, 4]
  22 [1, 2, 4]
  23 [0, 1, 2, 4]
  24 [3, 4]
  25 [0, 3, 4]
  26 [1, 3, 4]
  27 [0, 1, 3, 4]
  28 [2, 3, 4]
  29 [0, 2, 3, 4]
  30 [1, 2, 3, 4]

  In this example, a degree-d monomial is represented by a sequence of
  d increasing integers. The indexing scheme turns an increasing
  sequence of at most d integers into a single integer, which is
  suitable to index a single array.

  The indexing scheme internally uses a set of d lookup tables (each
  one with n entries) that must be initialized before the hash
  function can be used. Evaluating the indexing scheme on degree-k
  monomials requires k lookups and (k-1) integer additions.
 */


// Type of the entries in the lookup tables. It must be
// sufficiently large to represent N(n,d)
typedef uint32_t LUT_int_t;

// Actual type of the lookup tables
typedef const LUT_int_t * const * const LUT_t;

// A "package" that wraps the actual lookup tables
// along with useful informations
typedef const struct {
  LUT_t LUT;
  const int n;
  const int d;
} idx_lut_t;


// initialise a lookup table that works with n variables, and
// up to degree d. It also works for smaller degree, but shouldn't be
// used with less variables (otherwise there will be "holes" in the
// indexing sequence
extern const idx_lut_t *init_deginvlex_LUT(int n, int d);


// this function initialize another indexing scheme, that turns any
// boolean monomial of GF(2)[x_1,...,x_n] (without degree
// restriction), into an integer between 0 and 2^n-1
extern const idx_lut_t *init_lex_LUT(int n);

// print the lookup table. Mainly useful for debugging
extern void print_idx_LUT(const idx_lut_t *table);

// print a set of indices.  Mainly useful for debugging
extern void print_idx(int d, int *set);


// reclaim the memory used by the lookup table
extern void free_LUT(const idx_lut_t *tab);

// generic (and somewhat slow) function that computes an integer
// index given a set of indices. It is understood that `set` is an
// array of at least `d` entries (where `d` was the argument given to
// the init_idx_LUT function). If d=5, then the monomial [1,4,6,7,8]
// should be represented by the array {8,7,6,4,1}, and the monomial
// [0,4,6] should be represented by {6,4,0,-1,-1}.
extern LUT_int_t set2int(const idx_lut_t *table, int *set);

// These macros implement the same functionnality as set2int(), but
// they are much faster and lend themselves to compile-time
// optimisations. The `LUT` argument should be the LUT field of an
// idx_lut_r structure. The smallest index must be given first.
// for example (if the LUTs are generated with d=4):
//     idx_2(LUT, 1,3)   --> 10
//     idx_1(LUT, 4)     --> 16
//     idx_3(LUT, 0,2,3) --> 13
#define idx_0(LUT) (0)
#define idx_1(LUT,i0) ((LUT)[0][i0])
#define idx_2(LUT,i1,i0) ( idx_1(LUT, i0) + (LUT)[1][i1])
#define idx_3(LUT,i2,i1,i0) (idx_2(LUT,i1,i0) + (LUT)[2][i2])
#define idx_4(LUT,i3,i2,i1,i0) (idx_3(LUT,i2,i1,i0) + (LUT)[3][i3])
#define idx_5(LUT,i4,i3,i2,i1,i0) (idx_4(LUT,i3,i2,i1,i0) + (LUT)[4][i4])
#define idx_6(LUT,i5,i4,i3,i2,i1,i0) (idx_5(LUT,i4,i3,i2,i1,i0) + (LUT)[5][i5])
#define idx_7(LUT,i6,i5,i4,i3,i2,i1,i0) (idx_6(LUT,i5,i4,i3,i2,i1,i0) + (LUT)[6][i6])
#define idx_8(LUT,i7,i6,i5,i4,i3,i2,i1,i0) (idx_7(LUT,i6,i5,i4,i3,i2,i1,i0) + (LUT)[7][i7])
#define idx_9(LUT,i8,i7,i6,i5,i4,i3,i2,i1,i0) (idx_8(LUT,i7,i6,i5,i4,i3,i2,i1,i0) + (LUT)[8][i8])
#define idx_10(LUT,i9,i8,i7,i6,i5,i4,i3,i2,i1,i0) (idx_9(LUT,i8,i7,i6,i5,i4,i3,i2,i1,i0) + (LUT)[9][i9])


// generic backwards conversion between an integer and a tuple
// of d indices describing a monomial. This is the reciprocal of the
// set2int function. The `set` pointer must point to an array
// of length at least d.
extern void int2set(const idx_lut_t *table, LUT_int_t index, int *set);


// Convert an integer index representing a monomial in the indexing scheme specified by
//  `table_from` to an integer index representing the same monomial in the indexing scheme
// specified by `table_to`. It is assumed that `table_to` must be able to handle the number
// of variables and the degree of the monomial (unexpected behavior otherwise).
extern LUT_int_t idx_convert(idx_lut_t *table_from, idx_lut_t *table_to, LUT_int_t i);


// binomials coefficients
extern uint64_t binomials[64][64];
extern uint64_t n_monomials(int n, int d);
#ifdef __cplusplus

}

#endif

#endif

