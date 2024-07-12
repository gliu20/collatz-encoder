#include "limb_radix_common.h"

bool is_even(limb_dlist_t* ll) {
  if (ll->length == 0) return true;
  return (LL_HEAD(ll) & 1) == 0;
}

void pad_zero(limb_dlist_t* ll) {
  resize_limb_list_to_length(ll, ll->length + 1);
  insert_at_tail(ll, 0);
}

void pad_to_length(limb_dlist_t* ll, size_t length) {
  resize_limb_list_to_length(ll, length + 1);
  while (ll->length < length) insert_at_tail(ll, 0);
}

void guard_against_overflow(limb_dlist_t* ll) {
  if (ll->length == 0) {
    pad_zero(ll);
    return;
  }
  if (LL_TAIL(ll) != 0) {
    pad_zero(ll);
    return;
  }
}

bool is_eq_one(limb_dlist_t* ll) {  
  canonicalize(ll);
  if (ll->length != 1) return false;
  if (LL_HEAD(ll) != 1) return false;
  return true;
}

bool is_eq(limb_dlist_t* ll_a, limb_dlist_t* ll_b) {  
  canonicalize(ll_a);
  canonicalize(ll_b);
  
  if (ll_a->length != ll_b->length) return false;
  for (size_t i = 0; i < ll_a->length; i++) {
    if (LL_INDEX(ll_a, i) != LL_INDEX(ll_b, i)) return false;
  }
  return true;
}
