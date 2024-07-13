#include "limb_dlist.h"
#include "limb_radix_common.h"
#include "limb_radix_custom.h"
#include "limb_radix_pow2.h"
#include "limb_collatz.h"

limb_dlist_t* collatz_encode(limb_dlist_t* ll) {
  limb_dlist_t* result = new_limb_list();
  limb_dlist_t* ll_half = new_limb_list();
  size_t i = 0;
  
  // There does not exist a collatz encoding for 0
  // so we must check if its equal to zero
  canonicalize(ll);
  if (ll->length == 0) {
    destroy_limb_list(ll_half);
    return result;
  }

  while (!is_eq_one(ll)) {
    if (is_even(ll)) {
      // x / 2
      right_shift(ll);
    }
    else {
      // (3 x + 1) / 2 = x + (x + 1) / 2 = x + ((x + 1) >> 1)
      // since x is odd: = x + (x >> 1) + 1 also works
      copy_limb_list(ll_half, ll);
      
      // let's use optimized: x + ((x + 1) >> 1)
      fused_increment_divide_by_two(ll_half);
      add(ll, ll_half);
      set_ith_bit(result, i);
    }
    i++;
  }
  set_ith_bit(result, i);
  destroy_limb_list(ll_half);
  return result;
}

limb_dlist_t* collatz_decode(limb_dlist_t* ll) {
  limb_dlist_t* result = new_limb_list();
  size_t bit_length = get_bit_length(ll);
  
  pad_zero(result);
  plus_one(result);

  // There does not exist a collatz encoding for 0
  // so we must check if its equal to zero
  canonicalize(ll);
  if (ll->length == 0) {
    return result;
  }

  for (size_t i = bit_length - 2; i != __SIZE_MAX__; i--) {
    left_shift(result);
    
    if (get_ith_bit(ll, i) != 0) {
      minus_one(result);
      divide_by_three(result);
    }
  }
  return result;
}
