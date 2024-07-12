
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "limb_dlist.h"

#define swap(A, B) do { \
  __typeof__(A) _temp = (A); \
  (A) = (B); \
  (B) = _temp; \
} while(0)

#define max(a,b) ((a) > (b) ? (a) : (b))

#define IS_POWER_OF_TWO(N) (((N) & ((N) - 1u)) == 0u)

limb_t* new_limb_handle(size_t container_size) {
  limb_t* handle = (limb_t*) malloc(sizeof(limb_t) * container_size);
  assert(handle != NULL && "oom: failed to allocate new limb memory");
  return handle;
}

limb_dlist_t* new_limb_list() {
  limb_dlist_t* ll = (limb_dlist_t*) malloc(sizeof(limb_dlist_t));
  assert(ll != NULL && "oom: failed to allocate new limb list");
  
  ll->length = 0;
  ll->container_size = LL_INITIAL_SIZE;
  ll->handle = new_limb_handle(LL_INITIAL_SIZE);
  
  return ll;
}

void canonicalize(limb_dlist_t* ll) {
  if (ll->length == 0) return;
  // note this logic intentionally skips the head
  for (size_t i = ll->length - 1; i != 0; i--) {
    if (ll->handle[i] != 0) break;
    ll->length--;
  }
}

void insert_at_tail(limb_dlist_t* ll, limb_t limb) {
  ll->handle[ll->length++] = limb;
  assert((ll->length <= ll->container_size) 
    && "oob: insert at tail overflowed container");
}

void remove_at_tail(limb_dlist_t* ll) {
  assert(ll->length != 0 
    && "oob: cannot remove at tail from empty list");
  ll->handle[--ll->length] = 0;
}

void resize_limb_list(limb_dlist_t* ll, size_t container_size) {
  limb_t* new_handle = realloc(ll->handle, container_size);
  assert(new_handle != NULL 
    && "oom: failed to re-allocate new limb memory");
  assert(IS_POWER_OF_TWO(container_size) 
    && "err: expected container_size to be a power of 2");
  ll->handle = new_handle;
  ll->container_size = container_size;
}

void grow_limb_list(limb_dlist_t* ll) {
  resize_limb_list(ll, ll->container_size * 2);
}

void shrink_limb_list(limb_dlist_t* ll) {
  resize_limb_list(ll, ll->container_size / 2);
}

bool is_well_sized(limb_dlist_t* ll, size_t length) {
  bool does_fit = length <= ll->container_size;
  bool is_snug_fit = length > ll->container_size / 4;
  bool is_small = length < LL_INITIAL_SIZE;

  if (does_fit && is_snug_fit) return true;
  if (does_fit && is_small) return true;
  return false;
}

void resize_limb_list_to_length(limb_dlist_t* ll, size_t length) {
  if (is_well_sized(ll, length)) return;

  size_t log2_len = 0;
  size_t _length = length;
  while(_length != 0) {
    _length >>= 1;
    log2_len++;    
  }
  size_t pow2_rounded_len = max(1 << log2_len, LL_INITIAL_SIZE);

  resize_limb_list(ll, pow2_rounded_len);
  assert(is_well_sized(ll, length) 
    && "err: expected container to be well-sized after resize");
}

void swap_limb_list(limb_dlist_t* a, limb_dlist_t* b) {
  swap(a->length, b->length);
  swap(a->container_size, b->container_size);
  swap(a->handle, b->handle);
}

void copy_limb_list(limb_dlist_t* dest, limb_dlist_t* src) {
  bool will_fit_data = dest->container_size >= src->length;

  if (!will_fit_data) {
    // TODO: perf: small performance inefficiency since
    // resize keeps the underlying data by copying
    // the old data into a new buffer; however, we don't
    // need the old data
    resize_limb_list_to_length(dest, src->length);
  }

  memcpy(dest->handle, src->handle, src->length * sizeof(limb_t));
  dest->length = src->length;
}

void destroy_limb_list(limb_dlist_t* ll) {
  free(ll->handle);
  ll->handle = NULL;
  ll->length = 0;
  ll->container_size = 0;
  free(ll);
}

void print_limb_list(limb_dlist_t* ll) {
  printf("len: %zu  ", ll->length);
  printf("size: %zu  ", ll->container_size);
  for (size_t i = 0; i < ll->length; i++) {
    printf("%016llx  ", ll->handle[i]);
  }
  printf("\n");
}
