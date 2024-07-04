
#include "limb_math_common.h"
#include "limb_math_radix_custom.h"

#define FOR_EACH_CARRY_PROPAGATE(LL, EXPR_I) do { \
  limb_t _carry = 0; \
  _Pragma("clang loop distribute(enable)") \
  for (size_t i = 0; i < (LL)->length; i++) { \
    limb_t _result = (EXPR_I) + _carry; \
    LL_INDEX(LL, i) = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
  } \
} \
while (0)


#define max(a,b) ((a) > (b) ? (a) : (b))


void add(limb_vec_t* a, limb_vec_t* b) {
  // Use max + 1 to ensure room in case of overflow 
  size_t len = max(a->length, b->length) + 1;
  pad_to_length(a, len);
  pad_to_length(b, len);
  
  FOR_EACH_CARRY_PROPAGATE(a, LL_INDEX(a, i) + LL_INDEX(b, i));
}

void plus_one(limb_vec_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) + (i == 0));
}

void minus_one(limb_vec_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) + LIMB_MAX_VAL);
}

void left_shift(limb_vec_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, LL_INDEX(ll, i) << 1u);
}

void right_shift(limb_vec_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  guard_against_empty(ll);
  guard_against_overflow(ll);

  for (size_t i = 0; i < ll->length; i++) {
    LL_INDEX(ll, i) = (LL_INDEX(ll, i) / 2u) + (LL_INDEX(ll, i + 1) % 2u) * LIMB_DIVIDE_BY_TWO;
  }
}

void divide_by_three(limb_vec_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  guard_against_empty(ll);
  guard_against_overflow(ll);

  // $$ 
  // \sum_{i=0}^{n} (a_i/3)b^i
  // = \sum_{i=0}^{n} \left( (a_i//3) + (a_{i+1}%3)(b/3) \right) b^i 
  // $$
  for (size_t i = 0; i < ll->length; i++) {
    LL_INDEX(ll, i) = (LL_INDEX(ll, i) / 3u) + (LL_INDEX(ll, i + 1) % 3u) * LIMB_DIVIDE_BY_THREE;
  }
}

void fused_increment_divide_by_two(limb_vec_t* ll) {
  // Ensures most significant limb is 0 so we dont have to do a check for the (i+1)-th index
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, (LL_INDEX(ll, i) / 2u) + (LL_INDEX(ll, i + 1) % 2u) * LIMB_DIVIDE_BY_TWO + (i == 0));
}