#include "limb_radix_convert.h"
#include "limb_radix_common.h"
#include "limb_radix_custom.h"
#include "limb_radix_pow2.h"

void to_radix_pow2(limb_dlist_t* dest, limb_dlist_t* src) {
  size_t src_bit_len = get_bit_length(src);
  
  // TODO: perf: create clear command so we dont call
  // realloc on data we dont intend to keep around 
  // Clear destination and ensure space fits
  dest->length = 0;
  resize_limb_list_to_length(dest, src_bit_len / LIMB_CONTAINER_BIT_LENGTH);

  for (size_t i = 0; i < src_bit_len; i++) {
    if (!is_even(src)) {
      set_ith_bit(dest, i);
    }
    if (is_eq_one(src)) break;
    right_shift(src);
  }
}
void to_radix_custom(limb_dlist_t* dest, limb_dlist_t* src) {
  size_t src_bit_len = get_bit_length(src);
  
  // Clear destination and forcibly resize to small size
  dest->length = 0;
  pad_zero(dest);
  
  for (size_t i = src_bit_len - 1u; i != __SIZE_MAX__; i--) {
    if (get_ith_bit(src, i) != 0) {
      plus_one(dest);
    }
    left_shift(dest);
  }
}
