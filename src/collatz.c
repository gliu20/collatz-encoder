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


int test() {
  limb_vec_t* ll = new_limb_list();
  limb_vec_t* input = new_limb_list();
  insert_at_tail(ll, 1);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 256*256*12; i++) {

      copy_limb_list(input, ll);
      limb_vec_t* collatz = collatz_encode(input);
      limb_vec_t* uncollatz = collatz_decode(collatz);
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

int test_range() {
  limb_vec_t* ll = new_limb_list();
  limb_vec_t* input = new_limb_list();
  insert_at_tail(ll, 3);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {
      copy_limb_list(input, ll);
      limb_vec_t* collatz = collatz_encode(input);
      limb_vec_t* uncollatz = collatz_decode(collatz);
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
  limb_vec_t* ll = new_limb_list();
  limb_vec_t* input = new_limb_list();
  insert_at_tail(ll, 1);
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {

      copy_limb_list(input, ll);
      limb_vec_t* collatz = collatz_encode(input);
      limb_vec_t* uncollatz = collatz_decode(collatz);
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
    limb_vec_t* ll = new_limb_list();

    limb_t limb;
    size_t actual_read = 0;
    size_t data_units_read;
    fpos_t pos;
    
    while (true) {
      fgetpos(in_file,&pos);
      data_units_read = fread(&limb, sizeof(limb), 1, in_file);
      actual_read += data_units_read;
      if (data_units_read != 1) {
        if (feof(in_file)) {
          printf("warn: reached file eof\n");

          // Rewind back to just b4 eof error and 
          // attempt to read out as bytes
          fsetpos(in_file, &pos);
          printf("info: attempting to read last bytes\n");

          // Read last bytes
          limb_t mini_limb[sizeof(limb)] = {0};
          size_t mini_limb_index = 0;
          size_t mini_data_units_read = 0;

          while (true) {
            mini_data_units_read = fread(&mini_limb[mini_limb_index++], 1, 1, in_file);
            if (mini_data_units_read != 1) break;
          }

          printf("info: read %zu bytes\n", mini_limb_index - 1);

          // Reconstruct limb from mini limb
          limb = 0;
          for (size_t i = 0; i < mini_limb_index - 1; i++) {
            printf("info: got byte %02x\n", (char) mini_limb[i]);
            limb <<= 8;
            limb |= mini_limb[mini_limb_index - 2 - i];
          }

          printf("info: reconstructed limb: %016llx", limb);


          grow_limb_list_to_length(ll, ll->length + 1);
          insert_at_tail(ll, limb);

          break;
        }
        if (ferror(in_file)) {
          fclose(in_file);
          fclose(out_file);
          destroy_limb_list(ll);
          errx(EXIT_FAILURE, "err: failed to read from file");
        }
      }
      grow_limb_list_to_length(ll, ll->length + 1);
      insert_at_tail(ll, limb);
    }

    printf("\nread: %zu data units\n", actual_read);
    //print_limb_list(ll);

    limb_vec_t* collatz;
    
    if (*argv[1] == 'e') {
      collatz = collatz_encode(ll);
    }
    else if (*argv[1] == 'd') {
      collatz = collatz_decode(ll);
    }
    else {
      print_usage(argv[0]);
      destroy_limb_list(ll);
      break;
    }

    
    size_t actual_write = 0;
    size_t data_units_written;
    
    for (size_t i = 0; i < collatz->length; i++) {
      data_units_written= fwrite(&LL_INDEX(collatz,i), sizeof(limb), 1, out_file);
      actual_write += data_units_written;
      if (data_units_written != 1) {
        fclose(in_file);
        fclose(out_file);
        errx(EXIT_FAILURE, "err: failed to write to file");
      }
    }

    printf("\nwrite: %zu data units\n", actual_write);

    destroy_limb_list(ll);
    destroy_limb_list(collatz);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 4 && argc != 3) {
    print_usage(argv[0]);
    return 0;
  }

  if (argc == 3) {
    if (*argv[1] == 't') {
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
