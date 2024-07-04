#include "limb.h"
#include "limb_list.h"

#include <err.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define DEFER(...) for (int _i = 1; _i; _i = 0, __VA_ARGS__)

#define max(a,b) ((a) > (b) ? (a) : (b))

#define LOG_EXECUTION_TIME(STR) for( \
  clock_t _start = clock(), _end = 0; \
  _end == 0; \
  _end = clock(), \
  printf((STR), (double) (_end - _start) / CLOCKS_PER_SEC))


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
