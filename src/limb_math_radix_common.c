
#include "limb_math_radix_common.h"


bool is_even(limb_vec_t* ll) {
  if (ll->length == 0) return true;
  return (LL_HEAD(ll) & 1) == 0;
}

void pad_zero(limb_vec_t* ll) {
  insert_at_tail(ll, 0);
}

void pad_to_length(limb_vec_t* ll, size_t length) {
  while (ll->length < length) pad_zero(ll);
}

void guard_against_empty(limb_vec_t* ll) {
  if (ll->length == 0) pad_zero(ll);
}

void guard_against_overflow(limb_vec_t* ll) {
  if (LL_TAIL(ll) != 0) {
    pad_zero(ll);
  }
}

