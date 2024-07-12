
#include "debug.h"
#include "limb_radix_common.h"

bool is_even(limb_vec_t* ll) {
  if (ll->length == 0) return true;
  return (LL_HEAD(ll) & 1) == 0;
}

void pad_zero(limb_vec_t* ll) {
  grow_limb_list_to_length(ll, ll->length + 1);
  insert_at_tail(ll, 0);
}

void pad_to_length(limb_vec_t* ll, size_t length) {
  grow_limb_list_to_length(ll, ll->length + 1);
  while (ll->length < length) insert_at_tail(ll, 0);
}

void guard_against_empty(limb_vec_t* ll) {
  if (ll->length == 0) pad_zero(ll);
}

void guard_against_overflow(limb_vec_t* ll) {
  // These need to be separate if statements to prevent OOB
  // read by the LL_TAIL macro
  if (ll->length == 0) {
    pad_zero(ll);
  }
  if (LL_TAIL(ll) != 0) {
    pad_zero(ll);
  }
}

bool is_eq_one(limb_vec_t* ll) {  
  canonicalize(ll);
  if (ll->length != 1) return false;
  if (LL_HEAD(ll) != 1) return false;
  return true;
}

bool is_eq(limb_vec_t* ll_a, limb_vec_t* ll_b) {  
  canonicalize(ll_a);
  canonicalize(ll_b);
  
  if (ll_a->length != ll_b->length) return false;
  for (size_t i = 0; i < ll_a->length; i++) {
    if (LL_INDEX(ll_a, i) != LL_INDEX(ll_b, i)) return false;
  }
  return true;
}

size_t get_bit_length(limb_vec_t* ll) {
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

