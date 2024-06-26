#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define NDEBUG

#define DEFER(...) for (int _i = 1; _i; _i = 0, ##__VA_ARGS__)

#define limb_addc __builtin_addcl
#define limb_subc __builtin_subcl

typedef unsigned long limb_t;
typedef uint32_t len_t;

#define LIMB_BIT_LENGTH (1ull * CHAR_BIT * sizeof(limb_t))
#define LIMB_INT_MAX ((limb_t) (~0ull))
#define LIMB_MSB_INDEX (LIMB_BIT_LENGTH - 1ull)
#define LIMB_MSB_MASK (1ull << LIMB_MSB_INDEX)

// 256*256*256 is approx 1.x GB
// 256*256*128 is approx 964mb
// 256*256*12 is approx 500mb
#define POOL_MAX 256*256


typedef struct limb_li {
  limb_t limb;
  struct limb_li *prev;
  struct limb_li *next;
} limb_li_t;

typedef struct limb_ll {
  len_t length;
  limb_li_t *head;
} limb_ll_t;

const static limb_t LIMB_DIVIDE_THREE_LUT[] = {
    0,
    (LIMB_INT_MAX / 3),
    (LIMB_INT_MAX / 3) * 2
};

static limb_ll_t* pool;

void return_limb_to_pool(limb_li_t* li);
limb_li_t* new_limb();
limb_li_t* new_limb_maybe_from_pool();
limb_li_t* new_limb_val(limb_t limb);
limb_ll_t* new_limb_list();
void insert_at_tail(limb_ll_t* ll, limb_li_t* li);
void insert_at_head(limb_ll_t* ll, limb_li_t* li);
limb_li_t* remove_at_head(limb_ll_t* ll);
limb_li_t* remove_at_tail(limb_ll_t* ll);
bool is_even(limb_ll_t* ll);
void pad_zero(limb_ll_t* ll);
void guard_against_overflow(limb_ll_t* ll);
void left_shift(limb_ll_t* ll);
void right_shift(limb_ll_t* ll);
void plus_one(limb_ll_t* ll);
void minus_one(limb_ll_t* ll);
limb_ll_t* add(limb_ll_t* ll_a, limb_ll_t* ll_b);
void set_ith_bit(limb_ll_t* ll, len_t bit_index);
limb_t get_ith_bit(limb_ll_t* ll, len_t bit_index);
void canonicalize(limb_ll_t* ll);
bool is_eq_one(limb_ll_t* ll);
bool is_eq(limb_ll_t* ll_a, limb_ll_t* ll_b);
len_t get_bit_length(limb_ll_t* ll);
limb_ll_t* copy_limb_list(limb_ll_t* ll);
void destroy_limb_list(limb_ll_t* ll);
void return_limb_list_to_pool(limb_ll_t* ll);
limb_ll_t* divide_by_three_limb(limb_li_t* li, len_t limb_index);
limb_ll_t* divide_by_three(limb_ll_t* ll);
void print_limb_list(limb_ll_t* ll);
limb_ll_t* collatz_encode(limb_ll_t* ll);
limb_ll_t* collatz_decode(limb_ll_t* ll);
void print_debug();
void init_pool();
int test();


void return_limb_to_pool(limb_li_t* li) {
  if (pool->length >= POOL_MAX) {
    free(li);
    return;
  }
  li->limb = 0;
  li->next = NULL;
  li->prev = NULL;
  insert_at_tail(pool, li);
}

limb_li_t* new_limb() {
  limb_li_t* ptr = (limb_li_t*) malloc(sizeof(limb_li_t));
  assert(ptr != NULL && "oom: failed to allocate new limb");
  return ptr;
}

limb_li_t* new_limb_maybe_from_pool() {
  if (pool->length > 0) {
    return remove_at_head(pool);
  }
  return new_limb();
}

limb_li_t* new_limb_val(limb_t limb) {
  limb_li_t* ptr = new_limb_maybe_from_pool();
  ptr->limb = limb;
  return ptr;
}

limb_ll_t* new_limb_list() {
  limb_ll_t* ll = (limb_ll_t*) malloc(sizeof(limb_ll_t));
  assert(ll != NULL && "oom: failed to allocate new limb list");
  ll->length = 0;
  ll->head = NULL;
  return ll;
}

void insert_at_tail(limb_ll_t* ll, limb_li_t* li) {
  assert(ll != NULL && "err: attempted to insert into NULL list");
  assert(li != NULL && "err: attempted to insert NULL into list");
  
  if (__builtin_expect(ll->head == NULL, 0)) {
    ll->head = li;
    li->next = li;
    li->prev = li;
    ll->length += 1;
    return;
  }
  
  li->prev = ll->head->prev;
  li->next = ll->head;
  
  assert(ll->head->prev != NULL && "err: list is malformed");
  
  ll->head->prev->next = li;
  ll->head->prev = li;
  ll->length += 1;
}

void insert_at_head(limb_ll_t* ll, limb_li_t* li) {
  insert_at_tail(ll, li);
  ll->head = ll->head->prev;
}

limb_li_t* remove_at_head(limb_ll_t* ll) {
  assert(ll->head != NULL && "err: attempted to remove from a NULL list");
  
  limb_li_t* removed = ll->head;
  
  if (ll->head == ll->head->next) {
    ll->length -= 1;
    ll->head = NULL;
    
    assert(ll->length == 0 && "err: expected list length to equal zero after removing last element");
    
    return removed;
  }
  
  ll->head->next->prev = removed->prev;
  ll->head->prev->next = removed->next;
  ll->head = ll->head->next;
  
  removed->prev = NULL;
  removed->next = NULL;
  ll->length -= 1;
  
  return removed;
}

limb_li_t* remove_at_tail(limb_ll_t* ll) {
  assert(ll->head != NULL && "err: attempted to remove from a NULL list");
  
  limb_li_t* removed = ll->head->prev;
  
  if (__builtin_expect(removed == ll->head, 0)) {
    return remove_at_head(ll);
  }
  
  removed->next->prev = removed->prev;
  removed->prev->next = removed->next;
  
  removed->prev = NULL;
  removed->next = NULL;
  ll->length -= 1;
  
  return removed;
}

bool is_even(limb_ll_t* ll) {
  if (ll->head == NULL) return true;
  return (ll->head->limb & 1) == 0;
}

void pad_zero(limb_ll_t* ll) {
  insert_at_tail(ll, new_limb_val(0));
}

void guard_against_empty(limb_ll_t* ll) {
  if (ll->head == NULL) pad_zero(ll);
}

void guard_against_overflow(limb_ll_t* ll) {
  limb_t might_overflow = ll->head->prev->limb & LIMB_MSB_MASK;
  if (might_overflow) {
    pad_zero(ll);
  }
}

void left_shift(limb_ll_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  limb_t carry = 0;
  limb_li_t* current = ll->head;
  for (len_t i = 0; i < ll->length; i++) {
    limb_t result = (current->limb << 1u) | carry;
    carry = (current->limb & LIMB_MSB_MASK) >> LIMB_MSB_INDEX;
    current->limb = result & LIMB_INT_MAX;
    current = current->next;
  }
}

void right_shift(limb_ll_t* ll) {
  guard_against_empty(ll);
  limb_t carry = 0;
  limb_li_t* current = ll->head->prev;
  for (len_t i = 0; i < ll->length; i++) {
    limb_t result = (current->limb >> 1) | carry;
    carry = (current->limb & 1u) << LIMB_MSB_INDEX;
    current->limb = result & LIMB_INT_MAX;
    current = current->prev;
  }
}

void plus_one(limb_ll_t* ll) {
  limb_li_t* current = ll->head;
  for (len_t i = 0; i < ll->length; i++) {
    limb_t carry = 0;
    current->limb = limb_addc(current->limb, 0, 1, &carry);
    if (!carry) break;
    current = current->next;
  }
}

void minus_one(limb_ll_t* ll) {
  limb_li_t* current = ll->head;
  for (len_t i = 0; i < ll->length; i++) {
    limb_t carry = 0;
    current->limb = limb_subc(current->limb, 0, 1, &carry);
    if (!carry) break;
    current = current->next;
  }
}

limb_ll_t* add(limb_ll_t* ll_a, limb_ll_t* ll_b) {
  if (ll_a->length < ll_b->length) {
    // We swap the values of the limb_ll_t struct instead
    // of the ptrs to the struct so that ptrs to ll_a and ll_b
    // remain unchanged
    
    // Swap head ptr
    {
      limb_li_t* temp = ll_a->head;
      ll_a->head = ll_b->head;
      ll_b->head = temp;
    }
    // Swap lengths
    {
      len_t temp = ll_a->length;
      ll_a->length = ll_b->length;
      ll_b->length = temp;
    }
  }
  
  guard_against_overflow(ll_a);
  
  limb_t carry_in = 0;
  limb_t carry_out = 0;
  limb_li_t* current_a = ll_a->head;
  limb_li_t* current_b = ll_b->head;
  len_t i;
  
  // Add limbs up to length of smaller operand
  for (i = 0; i < ll_b->length; i++) {
    current_a->limb = limb_addc(current_a->limb, current_b->limb, carry_in, &carry_out);
    carry_in = carry_out;
    current_a = current_a->next;
    current_b = current_b->next;
  }
  
  // Carry propagate until length of bigger operand
  for (; i < ll_a->length; i++) {
    current_a->limb = limb_addc(current_a->limb, 0, carry_in, &carry_out);
    if (!carry_out) break;
    carry_in = carry_out;
    current_a = current_a->next;
  }
  
  return ll_a;
}

void set_ith_bit(limb_ll_t* ll, len_t bit_index) {
    len_t desired_limb = bit_index / LIMB_BIT_LENGTH;
    len_t desired_bit = bit_index % LIMB_BIT_LENGTH;
    
    while (desired_limb >= ll->length) {
        pad_zero(ll);
    }
    
    if (desired_limb == ll->length - 1) {
      ll->head->prev->limb |= ((limb_t) 1) << desired_bit;
      return;
    }
    
    // TODO: perf using slow list traversal as fallback
    limb_li_t* current = ll->head;
    for (len_t i = 0; i < desired_limb; i++) {
      current = current->next;
    }
    current->limb |= ((limb_t) 1) << desired_bit;
}

limb_t get_ith_bit(limb_ll_t* ll, len_t bit_index) {
    len_t desired_limb = bit_index / LIMB_BIT_LENGTH;
    len_t desired_bit = bit_index % LIMB_BIT_LENGTH;
    
    if (desired_limb > ll->length) return 0;
    if (desired_limb == ll->length - 1) {
      return ll->head->prev->limb & (((limb_t) 1) << desired_bit);
    }
    
    // TODO: perf using slow list traversal as fallback
    limb_li_t* current = ll->head;
    for (len_t i = 0; i < desired_limb; i++) {
      current = current->next;
    }
    return current->limb & (((limb_t) 1) << desired_bit);
}

void canonicalize(limb_ll_t* ll) {
  if (ll->length == 0) return;
  while (ll->head->prev->limb == 0) {
    limb_li_t* removed = remove_at_tail(ll);
    return_limb_to_pool(removed);
  }
}

bool is_eq_one(limb_ll_t* ll) {  
  canonicalize(ll);
  if (ll->length != 1) return false;
  if (ll->head->limb != 1) return false;
  return true;
}

bool is_eq(limb_ll_t* ll_a, limb_ll_t* ll_b) {  
  canonicalize(ll_a);
  canonicalize(ll_b);
  
  if (ll_a->length != ll_b->length) return false;
  
  limb_li_t* current_a = ll_a->head;
  limb_li_t* current_b = ll_b->head;
  
  for (len_t i = 0; i < ll_a->length; i++) {
    if (current_a->limb != current_b->limb) return false;
    current_a = current_a->next;
    current_b = current_b->next;
  }
  return true;
}

len_t get_bit_length(limb_ll_t* ll) {
  canonicalize(ll);
  if (ll->length == 0) return 0;
  
  //printf("todo: perf: use __builtin_clz for speed");
  
  len_t available_bits = ll->length * LIMB_BIT_LENGTH;
  limb_t most_significant_byte = ll->head->prev->limb;
  len_t used_bits = 0;
  while (most_significant_byte != 0) {
    used_bits++;
    most_significant_byte >>= 1;
  }
  return available_bits + used_bits - LIMB_BIT_LENGTH;
}

limb_ll_t* copy_limb_list(limb_ll_t* ll) {
  limb_ll_t* copy = new_limb_list();
  limb_li_t* current = ll->head;
  for (len_t i = 0; i < ll->length; i++) {
    insert_at_tail(copy, new_limb_val(current->limb));
    current = current->next;
  }
  return copy;
}

void destroy_limb_list(limb_ll_t* ll) {
  while (ll->length > 0) {
    limb_li_t* removed = remove_at_head(ll);
    free(removed);
  }
  free(ll);
}

void return_limb_list_to_pool(limb_ll_t* ll) {
  while (ll->length > 0) {
    limb_li_t* removed = remove_at_head(ll);
    return_limb_to_pool(removed);
  }
  free(ll);
}


limb_ll_t* divide_by_three_limb(limb_li_t* li, len_t limb_index) {
  limb_ll_t* ll = new_limb_list();
  
  limb_t divide_result = li->limb / 3;
  limb_t remainder = li->limb % 3;
  limb_t correction = LIMB_DIVIDE_THREE_LUT[remainder];
  
  // We insert an extra correction factor which represents
  // the fractional bits to ensure correct division results
  for (len_t i = 0; i < limb_index + 1; i++) {
    insert_at_tail(ll, new_limb_val(correction));
  }
  insert_at_tail(ll, new_limb_val(divide_result));
  
  return ll;
}

// Division by 3 results are only correct for when the integer
// quotient results in an odd integer
limb_ll_t* divide_by_three(limb_ll_t* ll) {
  limb_ll_t* result = new_limb_list();
  limb_li_t* current = ll->head;
  for (uint32_t i = 0; i < ll->length; i++) {
    limb_ll_t* partial_div_result = divide_by_three_limb(current, i);
    add(result, partial_div_result);
    return_limb_list_to_pool(partial_div_result);
    current = current->next;
  }
  // Remove the extra correction factor for the fractional bits
  return_limb_to_pool(remove_at_head(result));
  // Guarantee an odd result per what's expected for reversing a Collatz encoding
  result->head->limb |= 1;
  return result;
}

void print_limb_list(limb_ll_t* ll) {
  limb_li_t* current = ll->head;
  for (uint32_t i = 0; i < ll->length; i++) {
    printf("%016lx  ", current->limb);
    current = current->next;
  }
}

limb_ll_t* collatz_encode(limb_ll_t* ll) {
  limb_ll_t* result = new_limb_list();
  len_t i = 0;

  if (ll->head == NULL) return result;

  while (!is_eq_one(ll)) {
    if (is_even(ll)) {
      // x / 2
      right_shift(ll);
    }
    else {
      // (3 x + 1) / 2 = x + (x + 1) / 2 = x + ((x + 1) >> 1)
      // since x is odd: = x + (x >> 1) + 1
      limb_ll_t* ll_half = copy_limb_list(ll);
      right_shift(ll_half);
      add(ll, ll_half);
      return_limb_list_to_pool(ll_half);
      plus_one(ll);
      set_ith_bit(result, i);
    }
    i++;
  }
  set_ith_bit(result, i);
  return result;
}

limb_ll_t* collatz_decode(limb_ll_t* ll) {
  limb_ll_t* result = new_limb_list();
  len_t bit_length = get_bit_length(ll);
  pad_zero(result);
  plus_one(result);

  if (ll->head == NULL) return result;
  
  for (len_t i = bit_length - 2; i != (~((len_t) 0)); i--) {
    left_shift(result);
    
    if (get_ith_bit(ll, i) != 0) {
      minus_one(result);
      limb_ll_t* div_result = divide_by_three(result);
      return_limb_list_to_pool(result);
      result = div_result;
    }
  }
  return result;
}


void print_debug() {
  printf("Debug:\n");
  printf("\tusing bitwidth: %llu\n", LIMB_BIT_LENGTH);
  printf("\tcalculated int_max: %llu\n", (unsigned long long) LIMB_INT_MAX);
  printf("\tcalculated int_max/3: %llu\n", (unsigned long long) LIMB_DIVIDE_THREE_LUT[1]);
  printf("\tcalculated 2*int_max/3: %llu\n", (unsigned long long) LIMB_DIVIDE_THREE_LUT[2]);
  printf("\n");
}

void init_pool() {
  // Malloc is still faster than us for new allocations
  // when first allocating so we won't pre-fill the pool
  pool = new_limb_list();
}

int test() {
  clock_t start, end;
  double cpu_time_used;
  
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(1));
  
  start = clock();
  
  for (len_t i = 0; i < 256*256*12; i++) {
    limb_ll_t* input = copy_limb_list(ll);
    limb_ll_t* collatz = collatz_encode(input);
    limb_ll_t* uncollatz = collatz_decode(collatz);
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
    
    return_limb_list_to_pool(input);
    return_limb_list_to_pool(collatz);
    return_limb_list_to_pool(uncollatz);
    
    plus_one(ll);
  }
  
  destroy_limb_list(ll);
  destroy_limb_list(pool);
  
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  
  printf("Passed tests: %f seconds\n", cpu_time_used);

  return 0;
}

int test_range() {
  clock_t start, end;
  double cpu_time_used;
  
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(3));
  
  start = clock();
  
  for (len_t i = 0; i < 1024; i++) {
    limb_ll_t* input = copy_limb_list(ll);
    limb_ll_t* collatz = collatz_encode(input);
    limb_ll_t* uncollatz = collatz_decode(collatz);
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

    return_limb_list_to_pool(input);
    return_limb_list_to_pool(collatz);
    return_limb_list_to_pool(uncollatz);
    
    left_shift(ll);
    minus_one(ll);
  }


  
  destroy_limb_list(ll);
  destroy_limb_list(pool);
  
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  
  printf("Passed tests: %f seconds\n", cpu_time_used);

  return 0;
}

int test_range2() {
  clock_t start, end;
  double cpu_time_used;
  
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(1));
  
  start = clock();
  
  for (len_t i = 0; i < 1024; i++) {
    limb_ll_t* input = copy_limb_list(ll);
    limb_ll_t* collatz = collatz_encode(input);
    limb_ll_t* uncollatz = collatz_decode(collatz);
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

    if (i == 1023) {
      printf("\nmain: input: \n");
      print_limb_list(ll);
      printf("\nmain: collatz: \n");
      print_limb_list(collatz);
      printf("\nmain: uncollatz: \n");
      print_limb_list(uncollatz);
      printf("\n");
    }

    return_limb_list_to_pool(input);
    return_limb_list_to_pool(collatz);
    return_limb_list_to_pool(uncollatz);
    
    left_shift(ll);
    plus_one(ll);
  }


  
  destroy_limb_list(ll);
  destroy_limb_list(pool);
  
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  
  printf("Passed tests: %f seconds\n", cpu_time_used);

  return 0;
}

void print_usage(char* prog_name) {
  fprintf(stderr, "Usage: %s <encode|decode> <input_file> <output_file>\n", prog_name);
  fprintf(stderr, "Usage: %s <test> <0|1|2>\n", prog_name);
}


void encode_main(int argc, char* argv[]) {

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
    limb_ll_t* ll = new_limb_list();

    limb_t limb;
    len_t actual_read = 0;
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
          len_t mini_limb_index = 0;
          size_t mini_data_units_read = 0;

          while (true) {
            mini_data_units_read = fread(&mini_limb[mini_limb_index++], 1, 1, in_file);
            if (mini_data_units_read != 1) break;
          }


          printf("info: read %hu bytes\n", mini_limb_index - 1);

          // Reconstruct limb from mini limb
          limb = 0;
          for (len_t i = 0; i < mini_limb_index - 1; i++) {
            printf("info: got byte %02x\n", mini_limb[i]);
            limb <<= 8;
            limb |= mini_limb[mini_limb_index - 2 - i];
          }

          printf("info: reconstructed limb: %016lx", limb);

          insert_at_tail(ll, new_limb_val(limb));

          break;
        }
        if (ferror(in_file)) {
          fclose(in_file);
          fclose(out_file);
          destroy_limb_list(ll);
          errx(EXIT_FAILURE, "err: failed to read from file");
        }
      }
      insert_at_tail(ll, new_limb_val(limb));
    }

    printf("\nread: %hu data units\n", actual_read);
    //print_limb_list(ll);
    //printf("\n");

    limb_ll_t* collatz;
    
    if (*argv[1] == 'e') {
      collatz = collatz_encode(ll);
    }
    else if (*argv[1] == 'd') {
      collatz = collatz_decode(ll);
    }
    else {
      print_usage(argv[0]);
      destroy_limb_list(ll);
      destroy_limb_list(pool);
      break;
    }

    
    limb_li_t* current = collatz->head;
    len_t actual_write = 0;
    size_t data_units_written;
    
    for (len_t i = 0; i < collatz->length; i++) {
      data_units_written= fwrite(&current->limb, sizeof(limb), 1, out_file);
      actual_write += data_units_written;
      if (data_units_written != 1) {
        fclose(in_file);
        fclose(out_file);
        errx(EXIT_FAILURE, "err: failed to write to file");
      }
      current = current->next;
    }

    printf("\nwrite: %hu data units\n", actual_write);

    destroy_limb_list(ll);
    destroy_limb_list(collatz);
    destroy_limb_list(pool);
  }
}

int main(int argc, char* argv[]) {
  print_debug();
  init_pool();

  if (argc != 4 && argc != 3) {
    print_usage(argv[0]);
    return 0;
  }
  
  if (argc == 4) encode_main(argc, argv);
  
  return 0;
}
