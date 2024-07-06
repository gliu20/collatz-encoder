
#include <assert.h>
#include <stdio.h>

#include "debug.h"
#include "limb_list.h"

limb_t* new_limb_handle(size_t size_power_2) {
  limb_t* handle = (limb_t*) malloc(sizeof(limb_t) * LL_POWER_2_TO_SIZE(size_power_2));
  assert(handle != NULL && "oom: failed to allocate new limb memory");
  return handle;
}

limb_vec_t* new_limb_list() {
  limb_vec_t* ll = (limb_vec_t*) malloc(sizeof(limb_vec_t));
  assert(ll != NULL && "oom: failed to allocate new limb list");
  
  
  ll->head_offset = LIMB_LIST_INITIAL_HEAD_OFFSET;
  ll->size_power_2 = LIMB_LIST_INITIAL_SIZE;
  ll->length = 0;
  ll->handle = new_limb_handle(LIMB_LIST_INITIAL_SIZE);
  
  return ll;
}

void canonicalize(limb_vec_t* ll) {
  if (ll->length == 0) return;
  for (size_t i = ll->length - 1; i != 0; i--) {
    if (LL_INDEX(ll, i) != 0) break;
    ll->length--;
  }
}

void copy_limb_list(limb_vec_t* dest, limb_vec_t* src) {
  canonicalize(src);

  dest->head_offset = LIMB_LIST_INITIAL_HEAD_OFFSET;
  dest->length = 0;

  resize_limb_list_to_length(dest, src->length + 1);

  dest->length = src->length;

  for (size_t i = 0; i < src->length; i++) {
    LL_INDEX(dest, i) = LL_INDEX(src, i);
  }
}

void grow_limb_list(limb_vec_t* ll) {
  // Allocate double the old container size
  size_t new_size = ll->size_power_2 + 1;
  limb_t* new_handle = new_limb_handle(new_size);
  
  // Copy elements to new buffer
  for (size_t i = 0; i < ll->length; i++) {
    new_handle[LL_CIRCULAR_INDEX_MASK(ll, i, LL_POWER_2_TO_MASK(new_size))] = 
      ll->handle[LL_CIRCULAR_INDEX(ll, i)];
  }
  
  free(ll->handle);
  ll->handle = new_handle;
  ll->size_power_2 = new_size;
}

bool shrink_limb_list(limb_vec_t* ll) {
  // Allocate half the old container size
  size_t new_size = ll->size_power_2 - 1;
  
  // Not enough space so abandon attempting to shrink
  if (new_size < ll->length) return false; 
  
  limb_t* new_handle = new_limb_handle(new_size);
  
  // Copy elements to new buffer
  for (size_t i = 0; i < ll->length; i++) {
    new_handle[LL_CIRCULAR_INDEX_MASK(ll, i, LL_POWER_2_TO_MASK(new_size))] = 
      ll->handle[LL_CIRCULAR_INDEX(ll, i)];
  }
  
  free(ll->handle);
  ll->handle = new_handle;
  ll->size_power_2 = new_size;

  return true;
}

void resize_limb_list_to_length(limb_vec_t* ll, size_t length) {
  bool fits_desired_len = LL_POWER_2_TO_SIZE(ll->size_power_2) > length;
  bool is_oversized = (length << 2) > LL_POWER_2_TO_SIZE(ll->size_power_2);

  // Content already fits nicely so lets avoid resizing
  if (fits_desired_len && !is_oversized) return; 

  // Allocate ceiling of power of two that would fit requested length
  size_t new_size = sizeof(size_t) * 8u - (size_t) __builtin_clzll(length) + 1u;
  limb_t* new_handle = new_limb_handle(new_size);

  assert(LL_POWER_2_TO_SIZE(new_size) >= length && "resized container should fit required length");
  
  // Copy elements to new buffer
  for (size_t i = 0; i < ll->length; i++) {
    new_handle[LL_CIRCULAR_INDEX_MASK(ll, i, LL_POWER_2_TO_MASK(new_size))] = 
      ll->handle[LL_CIRCULAR_INDEX(ll, i)];
  }
  
  free(ll->handle);
  ll->handle = new_handle;
  ll->size_power_2 = new_size;
}

void grow_limb_list_to_length(limb_vec_t* ll, size_t length) {
  while (LL_POWER_2_TO_SIZE(ll->size_power_2) <= length) {
    grow_limb_list(ll);
  }
}

void shrink_limb_list_to_length(limb_vec_t* ll, size_t length) {
  while (LL_POWER_2_TO_SIZE(ll->size_power_2) > length) {
    bool did_shrink = shrink_limb_list(ll);
    if (!did_shrink) break;
  }
}

void insert_at_tail(limb_vec_t* ll, limb_t limb) {
  ll->handle[LL_CIRCULAR_INDEX(ll, ll->length)] = limb;
  ll->length++;

  assert(!(ll->length > LL_POWER_2_TO_SIZE(ll->size_power_2)) && 
    "insert at tail overflowed container");
}

void insert_at_head(limb_vec_t* ll, limb_t limb) {
  size_t head_index = LL_CIRCULAR_INDEX(ll, LL_POWER_2_TO_SIZE(ll->size_power_2) - 1u);
  ll->handle[head_index] = limb;
  ll->head_offset = head_index;
  ll->length++;

  assert(!(ll->length > LL_POWER_2_TO_SIZE(ll->size_power_2)) && 
    "insert at head overflowed container");
}

void remove_at_tail(limb_vec_t* ll) {
  assert(ll->length != 0 && "cannot remove at tail from empty list");
  LL_TAIL(ll) = 0;
  ll->length--;
}

void remove_at_head(limb_vec_t* ll) {
  assert(ll->length != 0 && "cannot remove at head from empty list");
  LL_HEAD(ll) = 0;
  ll->head_offset++;
  ll->length--;
}

void destroy_limb_list(limb_vec_t* ll) {
  free(ll->handle);
  ll->handle = NULL;
  ll->length = 0;
  ll->size_power_2 = 0;
  free(ll);
}

void print_limb_list(limb_vec_t* ll) {
  printf("pow2: %zu  ", ll->size_power_2);
  printf("len: %zu  ", ll->length);
  for (size_t i = 0; i < ll->length; i++) {
    printf("%016llx  ", LL_INDEX(ll, i));
  }
  printf("\n");
}
