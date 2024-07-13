#include "limb.h"
#include "limb_file.h"
#include "limb_dlist.h"
#include "limb_collatz.h"
#include "limb_radix_common.h"
#include "limb_radix_convert.h"
#include "limb_radix_custom.h"

#include <err.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define DEFER(...) for (int _i = 1; _i; _i = 0, __VA_ARGS__)

#define LOG_EXECUTION_TIME(STR) for( \
  clock_t _start = clock(), _end = 0; \
  _end == 0; \
  _end = clock(), \
  printf((STR), (double) (_end - _start) / CLOCKS_PER_SEC))


int test() {
  limb_dlist_t* ll = new_limb_list();
  limb_dlist_t* input = new_limb_list();
  insert_at_tail(ll, 1);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 256*256*12; i++) {

      copy_limb_list(input, ll);
      limb_dlist_t* collatz = collatz_encode(input);
      limb_dlist_t* uncollatz = collatz_decode(collatz);
      canonicalize(uncollatz);

      if (!is_eq(ll, uncollatz)) {
        printf("main: input: ");
        print_limb_list(ll);
        printf("main: collatz: ");
        print_limb_list(collatz);
        printf("main: uncollatz: ");
        print_limb_list(uncollatz);
        printf("\n");
        errx(EXIT_FAILURE, "err: collatz mismatch");
      }
      
      destroy_limb_list(collatz);
      destroy_limb_list(uncollatz);
      
      plus_one(ll);
    }
    
    destroy_limb_list(ll);
    destroy_limb_list(input);
  }

  return 0;
}



int test_convert() {
  limb_dlist_t* ll = new_limb_list();
  limb_dlist_t* input = new_limb_list();
  insert_at_tail(ll, 1);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 256*256*12; i++) {

      copy_limb_list(input, ll);
      limb_dlist_t* pow2 = new_limb_list();
      limb_dlist_t* custom = new_limb_list();

      to_radix_pow2(pow2, input);
      to_radix_custom(custom, pow2);
      canonicalize(custom);

      if (!is_eq(ll, custom)) {
        printf("main: input: ");
        print_limb_list(ll);
        printf("main: to_pow2: ");
        print_limb_list(pow2);
        printf("main: to_custom: ");
        print_limb_list(custom);
        printf("\n");
        errx(EXIT_FAILURE, "err: radix mismatch");
      }
      
      destroy_limb_list(pow2);
      destroy_limb_list(custom);
      
      plus_one(ll);
    }
    
    destroy_limb_list(ll);
    destroy_limb_list(input);
  }

  return 0;
}

int test_range() {
  limb_dlist_t* ll = new_limb_list();
  limb_dlist_t* input = new_limb_list();
  insert_at_tail(ll, 3);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {
      copy_limb_list(input, ll);
      limb_dlist_t* collatz = collatz_encode(input);
      limb_dlist_t* uncollatz = collatz_decode(collatz);
      canonicalize(uncollatz);
      
      if (!is_eq(ll, uncollatz)) {
        printf("\nmain: input: ");
        print_limb_list(ll);
        printf("\nmain: collatz: ");
        print_limb_list(collatz);
        printf("\nmain: uncollatz: ");
        print_limb_list(uncollatz);
        printf("\n");
        errx(EXIT_FAILURE, "err: collatz mismatch");
      }
      
      destroy_limb_list(collatz);
      destroy_limb_list(uncollatz);
      
      left_shift(ll);
      minus_one(ll);
    }
    
    destroy_limb_list(ll);
    destroy_limb_list(input);
  }

  return 0;
}

int test_range2() {
  limb_dlist_t* ll = new_limb_list();
  limb_dlist_t* input = new_limb_list();
  insert_at_tail(ll, 1);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {

      copy_limb_list(input, ll);
      limb_dlist_t* collatz = collatz_encode(input);
      limb_dlist_t* uncollatz = collatz_decode(collatz);
      canonicalize(uncollatz);
      
      if (!is_eq(ll, uncollatz)) {
        printf("\nmain: input: ");
        print_limb_list(ll);
        printf("\nmain: collatz: ");
        print_limb_list(collatz);
        printf("\nmain: uncollatz: ");
        print_limb_list(uncollatz);
        printf("\n");
        errx(EXIT_FAILURE, "err: collatz mismatch");
      }
      
      destroy_limb_list(collatz);
      destroy_limb_list(uncollatz);
      
      left_shift(ll);
      plus_one(ll);
    }
    
    destroy_limb_list(ll);
    destroy_limb_list(input);
  }

  return 0;
}


void test_limb_list() {
  limb_dlist_t* ll = new_limb_list();
  printf("empty: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 10; i++) {
    insert_at_tail(ll, (limb_t) i);
  }
  
  printf("insert 0-9 at tail: ");
  print_limb_list(ll);
  

  grow_limb_list(ll);
  printf("grow list: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 6; i++) {
    insert_at_tail(ll, (limb_t) i);
  }
  printf("insert 0-6 at tail: ");
  print_limb_list(ll);
  
  for (size_t i = 0; i < 4; i++) {
    remove_at_tail(ll);
  }
  
  printf("remove 3-6 at tail: ");
  print_limb_list(ll);
  
  destroy_limb_list(ll);
}

void print_usage(char* prog_name) {
  fprintf(stderr, "Usage: %s <encode|decode> <input_file> <output_file>\n", prog_name);
  fprintf(stderr, "Usage: %s <test>\n", prog_name);
}


void encode_main(char* argv[]) {

  FILE *in_file = fopen(argv[2], "rb");
  if (in_file == NULL) {
    errx(EXIT_FAILURE, "err: failed to open file in read binary mode");
  }
  printf("file: open input: %s\n", argv[2]);


  // Erase the file
  FILE *out_file = fopen(argv[3], "wb");
  if (out_file == NULL) {
    fclose(in_file);
    errx(EXIT_FAILURE, "err: failed to open file in write binary mode");
  }
  fclose(out_file);

  // Re-open in append only
  out_file = fopen(argv[3], "ab");
  if (out_file == NULL) {
    fclose(in_file);
    errx(EXIT_FAILURE, "err: failed to open file in append binary mode");
  }
  printf("file: open output: %s\n", argv[3]);


  DEFER(fclose(in_file), fclose(out_file)) {
    limb_dlist_t* ll = new_limb_list();

    size_t bytes_read = read_file(ll, in_file);
    if (bytes_read == __SIZE_MAX__) {
      printf("err: failed to read from file");
      break;
    }
    printf("\nread: %zu bytes\n", bytes_read);

    //print_limb_list(ll);

    
    if (*argv[1] == 'e') {
      limb_dlist_t* buffer = new_limb_list();

      to_radix_custom(buffer, ll);
      destroy_limb_list(ll);
      ll = collatz_encode(buffer);
      destroy_limb_list(buffer);
    }
    else if (*argv[1] == 'd') {
      limb_dlist_t* buffer = collatz_decode(ll);
      to_radix_pow2(ll, buffer);
      destroy_limb_list(buffer);
    }
    else {
      print_usage(argv[0]);
      destroy_limb_list(ll);
      break;
    }
    
    canonicalize(ll);
    
    size_t bytes_write = write_file(ll, out_file);
    if (bytes_write == __SIZE_MAX__) {
      printf("err: failed to write to file");
      break;
    }

    printf("\nwrite: %zu bytes\n", bytes_write);

    destroy_limb_list(ll);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 4 && argc != 2) {
    print_usage(argv[0]);
    return 0;
  }

  if (argc == 2) {
    if (*argv[1] == 't') {
      test_convert();
      test();
      test_range();
      test_range2();
    }
    else {
      print_usage(argv[0]);
    }
    return 0;
  }
  
  if (argc == 4) {
    LOG_EXECUTION_TIME("Encoded in %f seconds\n") encode_main(argv);
  }
  
  return 0;
}
