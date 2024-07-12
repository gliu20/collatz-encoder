#include "limb_radix_common.h"
#include "limb_radix_pow2.h"

void set_ith_bit(limb_dlist_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    pad_to_length(ll, desired_limb + 1);
    
    LL_INDEX(ll, desired_limb) |= ((limb_t) 1) << desired_bit;
}

limb_t get_ith_bit(limb_dlist_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    return LL_INDEX(ll, desired_limb) & (((limb_t) 1) << desired_bit);
}

size_t get_bit_length(limb_dlist_t* ll) {
  canonicalize(ll);
  if (ll->length == 0) return 0;
  
  size_t available_bits = ll->length * LIMB_CONTAINER_BIT_LENGTH;
  // Counting bits in a loop may seem inefficient but this accounts
  // for far less than 1% of the runtime. Additionally smart compilers
  // look for common patterns like this and optimize it to a couple
  // instructions anyway (optimized to BSR in gcc, but clang doesnt optimize this)
  limb_t most_significant_byte = LL_TAIL(ll);
  size_t used_bits = 0;
  while (most_significant_byte != 0) {
    used_bits++;
    most_significant_byte >>= 1;
  }
  return available_bits + used_bits - LIMB_CONTAINER_BIT_LENGTH;
}

