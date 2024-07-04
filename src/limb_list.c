
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
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

void shrink_limb_list(limb_vec_t* ll) {
  // Allocate half the old container size
  size_t new_size = ll->size_power_2 - 1;
  
  // Not enough space so abandon attempting to shrink
  if (new_size < ll->length) return; 
  
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

void insert_at_tail(limb_vec_t* ll, limb_t limb) {
  ll->handle[LL_CIRCULAR_INDEX(ll, ll->length)] = limb;
  ll->length++;
}

void insert_at_head(limb_vec_t* ll, limb_t limb) {
  size_t head_index = LL_CIRCULAR_INDEX(ll, LL_POWER_2_TO_SIZE(ll->size_power_2) - 1u);
  ll->handle[head_index] = limb;
  ll->head_offset = head_index;
  ll->length++;
}

void remove_at_tail(limb_vec_t* ll) {
  LL_TAIL(ll) = 0;
  ll->length--;
}

void remove_at_head(limb_vec_t* ll) {
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
