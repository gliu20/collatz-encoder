#include "limb.h"
#include "limb_list.h"
#include "limb_math_common.h"
#include "limb_math_radix_pow2.h"
#include "limb_math_radix_custom.h"

#include <err.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define DEFER(...) for (int _i = 1; _i; _i = 0, __VA_ARGS__)

#define LOG_EXECUTION_TIME(STR) for( \
  clock_t _start = clock(), _end = 0; \
  _end == 0; \
  _end = clock(), \
  printf((STR), (double) (_end - _start) / CLOCKS_PER_SEC))


limb_vec_t* collatz_encode(limb_vec_t* ll) {
  limb_vec_t* result = new_limb_list();
  limb_vec_t* ll_half = new_limb_list();
  size_t i = 0;
  
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

limb_vec_t* collatz_decode(limb_vec_t* ll) {
  limb_vec_t* result = new_limb_list();
  size_t bit_length = get_bit_length(ll);
  
  pad_zero(result);
  plus_one(result);

  if (ll->length == 0) {
    return result;
  }
  
  for (size_t i = bit_length - 2; i != (~((size_t) 0)); i--) {
    left_shift(result);
    
    if (get_ith_bit(ll, i) != 0) {
      minus_one(result);
      divide_by_three(result);
    }
  }
  return result;
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
  
  destroy_limb_list(ll);
}

int main() {
  test_limb_list();
  return 0;
}
