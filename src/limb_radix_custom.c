
#include "limb_radix_common.h"
#include "limb_radix_custom.h"

#define FOR_EACH_CARRY_PROPAGATE(LL, EXPR_I) do { \
  limb_t _carry = 0; \
  for (size_t i = 0; i < (LL)->length; i++) { \
    limb_t _result = (EXPR_I) + _carry; \
    LL_INDEX(LL, i) = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
  } \
} \
while (0)

#define PRAGMA_WRAP(X) _Pragma(#X)

#define FOR_EACH_CARRY_PROPAGATE_UNROLL(LL, EXPR_I, UNROLL) do { \
  limb_t _carry = 0; \
  PRAGMA_WRAP(unroll UNROLL) \
  for (size_t i = 0; i < (LL)->length; i++) { \
    limb_t _result = (EXPR_I) + _carry; \
    LL_INDEX(LL, i) = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
  } \
} \
while (0)

#define max(a,b) ((a) > (b) ? (a) : (b))


void add(limb_dlist_t* a, limb_dlist_t* b) {
  canonicalize(a);
  canonicalize(b);

  // Use max + 1 to ensure room in case of overflow 
  size_t len = max(a->length, b->length) + 1;
  pad_to_length(a, len);
  pad_to_length(b, len);
  
  FOR_EACH_CARRY_PROPAGATE(a, LL_INDEX(a, i) + LL_INDEX(b, i));
}

void plus_one(limb_dlist_t* ll) {
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) + (i == 0));
}

void minus_one(limb_dlist_t* ll) {
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) + LIMB_MAX_VAL);
}

void left_shift(limb_dlist_t* ll) {
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) << 1u);
}

void right_shift(limb_dlist_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  canonicalize(ll);
  guard_against_overflow(ll);

  // Most significant limb is 0, so use `len - 1` to prevent OOB read
  #pragma clang loop vectorize(enable)
  for (size_t i = 0; i < ll->length - 1; i++) {
    LL_INDEX(ll, i) = (LL_INDEX(ll, i) / 2u) + (LL_INDEX(ll, i + 1) % 2u) * LIMB_DIVIDE_BY_TWO;
  }
}

void divide_by_three(limb_dlist_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  canonicalize(ll);
  guard_against_overflow(ll);

  // $$ 
  // \sum_{i=0}^{n} (a_i/3)b^i
  // = \sum_{i=0}^{n} \left( (a_i//3) + (a_{i+1}%3)(b/3) \right) b^i 
  // $$
  // Most significant limb is 0, so use `len - 1` to prevent OOB read
  #pragma clang loop vectorize(enable)
  for (size_t i = 0; i < ll->length - 1; i++) {
    LL_INDEX(ll, i) = (LL_INDEX(ll, i) / 3u) + (LL_INDEX(ll, i + 1) % 3u) * LIMB_DIVIDE_BY_THREE;
  }
}

void multiply_by_three(limb_dlist_t* ll) {
  // Ensure most significant limb is 0
  canonicalize(ll);
  guard_against_overflow(ll);
  

  limb_t carry = 0;
  for (size_t i = 0; i < ll->length; i++) {
    // We have to split out into two separate parts in order
    // to avoid overflow due to multiplication
    limb_t lshift = (LL_INDEX(ll, i) << 1u) + carry;
    limb_t lshift_mod = lshift % LIMB_BASE;
    limb_t lshift_div = lshift / LIMB_BASE;


    limb_t triple = lshift_mod + LL_INDEX(ll, i);
    limb_t triple_mod = triple % LIMB_BASE;
    limb_t triple_div = triple / LIMB_BASE;

    LL_INDEX(ll, i) = triple_mod;

    carry = lshift_div + triple_div;
  }
}

void multiply_by_three_and_increment(limb_dlist_t* ll) {
  guard_against_overflow(ll);

  limb_t carry = 1;
  for (size_t i = 0; i < ll->length; i++) {
    // We have to split out into two separate parts in order
    // to avoid overflow due to multiplication
    limb_t lshift = (LL_INDEX(ll, i) << 1u) + carry;
    limb_t lshift_mod = lshift % LIMB_BASE;
    limb_t lshift_div = lshift / LIMB_BASE;


    limb_t triple = lshift_mod + LL_INDEX(ll, i);
    limb_t triple_mod = triple % LIMB_BASE;
    limb_t triple_div = triple / LIMB_BASE;

    LL_INDEX(ll, i) = triple_mod;

    carry = lshift_div + triple_div;
  }
}

void divide_by_three_optim(limb_dlist_t* ll, limb_dlist_t* buffer) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  canonicalize(ll);
  guard_against_overflow(ll);

  resize_limb_list_to_length(buffer, ll->length + 1);
  buffer->length = ll->length - 1;

  // $$ 
  // \sum_{i=0}^{n} (a_i/3)b^i
  // = \sum_{i=0}^{n} \left( (a_i//3) + (a_{i+1}%3)(b/3) \right) b^i 
  // $$
  size_t i;

  // Most significant limb is 0, so use `len - 1` to prevent OOB read
  #pragma unroll(1)
  #pragma clang loop vectorize(enable)
  for (i = 0; i < ll->length - 1; i += 4) {
    buffer->handle[i] = (ll->handle[i] / 3u) + (ll->handle[i + 1] % 3u) * LIMB_DIVIDE_BY_THREE;
    buffer->handle[i + 1] = (ll->handle[i + 1] / 3u) + (ll->handle[i + 2] % 3u) * LIMB_DIVIDE_BY_THREE;
    buffer->handle[i + 2] = (ll->handle[i + 2] / 3u) + (ll->handle[i + 3] % 3u) * LIMB_DIVIDE_BY_THREE;
    buffer->handle[i + 3] = (ll->handle[i + 3] / 3u) + (ll->handle[i + 4] % 3u) * LIMB_DIVIDE_BY_THREE;
  }

  // Fixup to finish the unrolled part above
  for (; i < ll->length - 1; i++) {
    buffer->handle[i] = (ll->handle[i] / 3u) + (ll->handle[i + 1] % 3u) * LIMB_DIVIDE_BY_THREE;
  }

  swap_limb_list(ll, buffer);
}

void fused_increment_divide_by_two(limb_dlist_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  canonicalize(ll);
  guard_against_overflow(ll);
  
  // This ensures that we stay within the true length of the list
  ll->length--;
  FOR_EACH_CARRY_PROPAGATE(ll, (LL_INDEX(ll, i) / 2u) + (LL_INDEX(ll, i + 1) % 2u) * LIMB_DIVIDE_BY_TWO + (i == 0));
}
