#include <assert.h>
#include <err.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define DEFER(...) for (int _i = 1; _i; _i = 0, __VA_ARGS__)

#define max(a,b) ((a) > (b) ? (a) : (b))

#define LOG_EXECUTION_TIME(STR) for( \
  clock_t _start = clock(), _end = 0; \
  _end == 0; \
  _end = clock(), \
  printf((STR), (double) (_end - _start) / CLOCKS_PER_SEC))

typedef unsigned long long limb_t;

// Assume: Bit length of a byte == 8;
// It is a hard requirement that we are working with bytes
// with a standard bit length  of 8
#define LIMB_CONTAINER_BIT_LENGTH (sizeof(limb_t) * 8u)
#define LIMB_BIT_LENGTH (LIMB_CONTAINER_BIT_LENGTH - 1u)
#define LIMB_BASE (((limb_t) 1u << LIMB_BIT_LENGTH) - 2u)
#define LIMB_MAX_VAL (LIMB_BASE - 1u)
#define LIMB_DIVIDE_BY_TWO (LIMB_BASE / 2u)
#define LIMB_DIVIDE_BY_THREE (LIMB_BASE / 3u)

#define LIMB_LIST_INITIAL_HEAD_OFFSET 4
#define LIMB_LIST_INITIAL_SIZE 4

#define LL_POWER_2_TO_SIZE(POWER_2) (1u << (POWER_2))
#define LL_POWER_2_TO_MASK(POWER_2) ((1u << (POWER_2)) - 1u)
#define LL_CIRCULAR_INDEX_MASK(LL, I, MASK) (((I) + (LL)->head_offset) & (MASK))
#define LL_CIRCULAR_INDEX(LL, I) (LL_CIRCULAR_INDEX_MASK(LL, I, LL_POWER_2_TO_MASK((LL)->size_power_2)))

typedef struct limb_vec {
  size_t head_offset;
  size_t size_power_2;
  size_t length;
  limb_t *handle;
} limb_vec_t;

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
  ll->handle[LL_CIRCULAR_INDEX(ll, ll->length - 1u)] = 0;
  ll->length--;
}

void remove_at_head(limb_vec_t* ll) {
  ll->handle[LL_CIRCULAR_INDEX(ll, 0)] = 0;
  ll->head_offset++;
  ll->length--;
}

void destory_limb_list(limb_vec_t* ll) {
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
    printf("%016llx  ", ll->handle[LL_CIRCULAR_INDEX(ll, i)]);
  }
  printf("\n");
}

bool is_even(limb_vec_t* ll) {
  if (ll->length == 0) return true;
  return (ll->handle[LL_CIRCULAR_INDEX(ll, 0)] & 1) == 0;
}

void pad_zero(limb_vec_t* ll) {
  insert_at_tail(ll, 0);
}

void pad_to_length(limb_vec_t* ll, size_t length) {
  while (ll->length < length) pad_zero(ll);
}

void guard_against_empty(limb_ll_t* ll) {
  if (ll->length == 0) pad_zero(ll);
}

void guard_against_overflow(limb_ll_t* ll) {
  if (ll->handle[LL_CIRCULAR_INDEX(ll, ll->length - 1u)] != 0) {
    pad_zero(ll);
  }
}


void test_limb_list() {
  limb_vec_t* ll = new_limb_list();
  printf("empty: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 10; i++) {
    insert_at_tail(ll, (limb_t) i);
  }
  
  printf("insert 0-9 at tail: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 4; i++) {
    insert_at_head(ll, (limb_t) i);
  }
  
  
  printf("insert 0-4 at head: ");
  print_limb_list(ll);
  
  grow_limb_list(ll);
  printf("grow list: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 6; i++) {
    insert_at_head(ll, (limb_t) i);
  }
  printf("insert 0-6 at head: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 4; i++) {
    remove_at_tail(ll);
  }
  
  printf("remove 6-9 at tail: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 3; i++) {
    remove_at_head(ll);
  }
  
  printf("remove 3-1 at head: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 12; i++) {
    remove_at_head(ll);
  }
  
  printf("remove all but one at head: ");
  print_limb_list(ll);
  
  destory_limb_list(ll);
}

int main() {
  test_limb_list();
  return 0;
}
