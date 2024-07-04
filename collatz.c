

//#include <assert.h>
#include <err.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//#define NDEBUG
#define assert(_ignore)

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

typedef struct limb_li {
  limb_t limb;
  struct limb_li *prev;
  struct limb_li *next;
} limb_li_t;

typedef struct limb_ll {
  size_t length;
  limb_li_t *head;
} limb_ll_t;

#define POOL_MAX (256u*256u)

static limb_ll_t* pool;

// Only valid for length >= 1


#define FOR_EACH_CARRY_PROPAGATE(LL, EXPR_I) do { \
  limb_t _carry = 0; \
  limb_li_t* _current = (LL)->head; \
  _Pragma("clang loop distribute(enable)") \
  for (size_t i = 0; i < (LL)->length; i++) { \
    limb_t current_limb = _current->limb; \
    limb_t _result = (EXPR_I) + _carry; \
    _current->limb = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
    _current = _current->next;\
  } \
} \
while (0)

// Only valid for length >= 1
#define FOR_EACH_ADD_CARRY_PROPAGATE(LL_A, LL_B, EXPR_I) do { \
  limb_t _carry = 0; \
  limb_li_t* _current_a = (LL_A)->head; \
  limb_li_t* _current_b = (LL_B)->head; \
  for (size_t _i = 0; _i < (LL_A)->length; _i++) { \
    limb_t current_limb_a = _current_a->limb; \
    limb_t current_limb_b = _current_b->limb; \
    limb_t _result = (EXPR_I) + _carry; \
    _current_a->limb = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
    _current_a = _current_a->next;\
    _current_b = _current_b->next;\
  } \
} \
while (0)

// Only valid for length >= 1
#define FOR_EACH_CARRY_PROPAGATE_NEXT(LL, EXPR_I) do { \
  limb_t _carry = 0; \
  limb_li_t* _current = (LL)->head; \
  size_t i; \
  for (i = 0; i < (LL)->length - 1; i++) { \
    limb_t current_limb = _current->limb; \
    limb_t next_limb = _current->next->limb; \
    limb_t _result = (EXPR_I) + _carry; \
    _current->limb = _result % LIMB_BASE; \
    _carry = _result / LIMB_BASE; \
    _current = _current->next;\
  } \
  limb_t current_limb = _current->limb; \
  limb_t next_limb = 0; \
  limb_t _result = (EXPR_I) + _carry; \
  _current->limb = _result % LIMB_BASE; \
  _carry = _result / LIMB_BASE; \
} \
while (0)

limb_li_t* remove_at_head(limb_ll_t* ll);
void return_limb_to_pool(limb_li_t* li);
void print_limb_list(limb_ll_t* ll);

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
  
  if (ll->head == NULL) {
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
  
  if (removed == ll->head) {
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

void pad_to_length(limb_ll_t* ll, size_t length) {
  while (ll->length < length) pad_zero(ll);
}

void guard_against_empty(limb_ll_t* ll) {
  if (ll->head == NULL) pad_zero(ll);
}

void guard_against_overflow(limb_ll_t* ll) {
  if (ll->head->prev->limb != 0) {
    pad_zero(ll);
  }
}

void add(limb_ll_t* a, limb_ll_t* b) {
  // Use max + 1 to ensure room in case of overflow 
  size_t len = max(a->length, b->length) + 1;
  pad_to_length(a, len);
  pad_to_length(b, len);
  
  FOR_EACH_ADD_CARRY_PROPAGATE(a, b, current_limb_a + current_limb_b);
}

void plus_one(limb_ll_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, current_limb + (i == 0));
}

void minus_one(limb_ll_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, current_limb + LIMB_MAX_VAL);
}

void left_shift(limb_ll_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE(ll, current_limb << 1u);
}

void right_shift(limb_ll_t* ll) {
  guard_against_empty(ll);
  
  limb_li_t* current = ll->head;
  size_t i;
  
  for (i = 0; i < ll->length - 1; i++) {
    limb_t current_limb = current->limb;
    limb_t next_limb = current->next->limb;
    limb_t result = (current_limb / 2u) + (next_limb % 2u) * LIMB_DIVIDE_BY_TWO;
    current->limb = result;
    current = current->next;
  }
  limb_t current_limb = current->limb;
  limb_t next_limb = 0;
  limb_t result = (current_limb / 2u) + (next_limb % 2u) * LIMB_DIVIDE_BY_TWO;
  current->limb = result;
}

void divide_by_three(limb_ll_t* ll) {
  guard_against_empty(ll);
  
  // $$ 
  // \sum_{i=0}^{n} (a_i/3)b^i
  // = \sum_{i=0}^{n} \left( (a_i//3) + (a_{i+1}%3)(b/3) \right) b^i 
  // $$
  limb_li_t* current = ll->head;
  size_t i;
  
  for (i = 0; i < ll->length - 1; i++) {
    limb_t current_limb = current->limb;
    limb_t next_limb = current->next->limb;
    limb_t result = (current_limb / 3u) + (next_limb % 3u) * LIMB_DIVIDE_BY_THREE;
    current->limb = result;
    current = current->next;
  }
  limb_t current_limb = current->limb;
  limb_t next_limb = 0;
  limb_t result = (current_limb / 3u) + (next_limb % 3u) * LIMB_DIVIDE_BY_THREE;
  current->limb = result;
}

void fused_increment_divide_by_two(limb_ll_t* ll) {
  guard_against_empty(ll);
  guard_against_overflow(ll);
  
  FOR_EACH_CARRY_PROPAGATE_NEXT(ll, (current_limb / 2u) + (next_limb % 2u) * LIMB_DIVIDE_BY_TWO + (i == 0));
}

void set_ith_bit(limb_ll_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    pad_to_length(ll, desired_limb + 1);
    
    if (desired_limb == ll->length - 1) {
      ll->head->prev->limb |= ((limb_t) 1) << desired_bit;
      return;
    }
    
    // TODO: perf using slow list traversal as fallback
    limb_li_t* current = ll->head;
    for (size_t i = 0; i < desired_limb; i++) {
      current = current->next;
    }
    current->limb |= ((limb_t) 1) << desired_bit;
}

limb_t get_ith_bit_from_limb(limb_li_t* li, size_t desired_bit) {
    return li->limb & (((limb_t) 1) << desired_bit);
}

limb_t get_ith_bit(limb_ll_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    if (desired_limb > ll->length) return 0;
    if (desired_limb == ll->length - 1) {
      return get_ith_bit_from_limb(ll->head->prev, desired_bit);
    }
    
    // TODO: perf using slow list traversal as fallback
    if (desired_limb > ll->length / 2) {
      // Traverse backwards when its on the other side linked list
      limb_li_t* current = ll->head->prev;
      for (size_t i = ll->length - 1; i > desired_limb; i--) {
        current = current->prev;
      }
      return get_ith_bit_from_limb(current, desired_bit);
    }
    
    limb_li_t* current = ll->head;
    for (size_t i = 0; i < desired_limb; i++) {
      current = current->next;
    }
    return get_ith_bit_from_limb(current, desired_bit);
}

// If we traverse the limb list in reverse order, accelerates limb lookup by keeping
// an accel limb_li_t to avoid having to re-navigate to the current location each iteration
limb_t get_ith_bit_in_reverse_order(limb_li_t** accel, size_t* accel_pos, size_t bit_index) {
  size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
  size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
  
  if (desired_limb != *accel_pos) {
    *accel = (*accel)->prev;
    *accel_pos -= 1;
  }
  
  return get_ith_bit_from_limb(*accel, desired_bit);
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
  
  for (size_t i = 0; i < ll_a->length; i++) {
    if (current_a->limb != current_b->limb) return false;
    current_a = current_a->next;
    current_b = current_b->next;
  }
  return true;
}

size_t get_bit_length(limb_ll_t* ll) {
  canonicalize(ll);
  if (ll->length == 0) return 0;
  
  size_t available_bits = ll->length * LIMB_CONTAINER_BIT_LENGTH;
  // Counting bits in a loop may seem inefficient but this accounts
  // for far less than 1% of the runtime. Additionally smart compilers
  // look for common patterns like this and optimize it to a couple
  // instructions anyway (optimized to BSR in gcc, but clang doesnt optimize this)
  limb_t most_significant_byte = ll->head->prev->limb;
  size_t used_bits = 0;
  while (most_significant_byte != 0) {
    used_bits++;
    most_significant_byte >>= 1;
  }
  return available_bits + used_bits - LIMB_CONTAINER_BIT_LENGTH;
}

limb_ll_t* copy_limb_list(limb_ll_t* ll) {
  limb_ll_t* copy = new_limb_list();
  limb_li_t* current = ll->head;
  for (size_t i = 0; i < ll->length; i++) {
    insert_at_tail(copy, new_limb_val(current->limb));
    current = current->next;
  }
  return copy;
}

void copy_into_limb_list(limb_ll_t* dest, limb_ll_t* src) {
  limb_li_t* current_dest = dest->head;
  limb_li_t* current_src = src->head;
  size_t i;
  
  // Copy the shared part of the list
  for (i = 0; i < dest->length && i < src->length; i++) {
    current_dest->limb = current_src->limb;
    current_dest = current_dest->next;
    current_src = current_src->next;
  }
  
  // Remove the extra values in dest
  while (dest->length > src->length) {
    limb_li_t* removed = remove_at_tail(dest);
    return_limb_to_pool(removed);
  }
  
  // Copy the extra values from src
  for (; i < src->length; i++) {
    insert_at_tail(dest, new_limb_val(current_src->limb));
    current_src = current_src->next;
  }
}

void destroy_limb_list(limb_ll_t* ll) {
  while (ll->length > 0) {
    limb_li_t* removed = remove_at_head(ll);
    free(removed);
  }
  free(ll);
}

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

void return_limb_list_to_pool(limb_ll_t* ll) {
  while (ll->length > 0) {
    limb_li_t* removed = remove_at_head(ll);
    return_limb_to_pool(removed);
  }
  free(ll);
}

limb_ll_t* collatz_encode(limb_ll_t* ll) {
  limb_ll_t* result = new_limb_list();
  limb_ll_t* ll_half = new_limb_list();
  size_t i = 0;

  if (ll->head == NULL) return result;

  while (!is_eq_one(ll)) {
    if (is_even(ll)) {
      // x / 2
      right_shift(ll);
    }
    else {
      // (3 x + 1) / 2 = x + (x + 1) / 2 = x + ((x + 1) >> 1)
      // since x is odd: = x + (x >> 1) + 1 also works
      copy_into_limb_list(ll_half, ll);
      
      // let's use optimized: x + ((x + 1) >> 1)
      fused_increment_divide_by_two(ll_half);
      add(ll, ll_half);
      set_ith_bit(result, i);
    }
    i++;
  }
  set_ith_bit(result, i);
  return_limb_list_to_pool(ll_half);
  return result;
}

limb_ll_t* collatz_decode(limb_ll_t* ll) {
  limb_ll_t* result = new_limb_list();
  size_t bit_length = get_bit_length(ll);
  
  pad_zero(result);
  plus_one(result);

  if (ll->head == NULL) return result;
  
  limb_li_t* accel = ll->head->prev;
  size_t accel_pos = ll->length - 1;
  for (size_t i = bit_length - 2; i != (~((size_t) 0)); i--) {
    left_shift(result);
    
    if (get_ith_bit_in_reverse_order(&accel, &accel_pos, i) != 0) {
      minus_one(result);
      divide_by_three(result);
    }
  }
  return result;
}

void print_limb_list(limb_ll_t* ll) {
  limb_li_t* current = ll->head;
  for (uint32_t i = 0; i < ll->length; i++) {
    printf("%016llx  ", current->limb);
    current = current->next;
  }
}

void init_pool() {
  // Malloc is still faster than us for new allocations
  // when first allocating so we won't pre-fill the pool
  pool = new_limb_list();
}



int test() {
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(1));
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 256*256*12; i++) {
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
  }

  return 0;
}

int test_range() {
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(3));
  
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {
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
  }

  return 0;
}

int test_range2() {
  limb_ll_t* ll = new_limb_list();
  insert_at_tail(ll, new_limb_val(1));
  
  LOG_EXECUTION_TIME("Passed tests: %f seconds\n") {
    for (size_t i = 0; i < 1024; i++) {
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
      plus_one(ll);
    }
    
    destroy_limb_list(ll);
    destroy_limb_list(pool);
  }
  
  return 0;
}

void print_usage(char* prog_name) {
  fprintf(stderr, "Usage: %s <encode|decode> <input_file> <output_file>\n", prog_name);
  fprintf(stderr, "Usage: %s <test> <0|1|2>\n", prog_name);
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
    limb_ll_t* ll = new_limb_list();

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

    printf("\nread: %zu data units\n", actual_read);
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
    size_t actual_write = 0;
    size_t data_units_written;
    
    for (size_t i = 0; i < collatz->length; i++) {
      data_units_written= fwrite(&current->limb, sizeof(limb), 1, out_file);
      actual_write += data_units_written;
      if (data_units_written != 1) {
        fclose(in_file);
        fclose(out_file);
        errx(EXIT_FAILURE, "err: failed to write to file");
      }
      current = current->next;
    }

    printf("\nwrite: %zu data units\n", actual_write);

    destroy_limb_list(ll);
    destroy_limb_list(collatz);
    destroy_limb_list(pool);
  }
}

int main(int argc, char* argv[]) {
  init_pool();
  test_range2();
  return 0;
  //test_range();
  if (argc != 4 && argc != 3) {
    print_usage(argv[0]);
    return 0;
  }
  
  if (argc == 4) {
    LOG_EXECUTION_TIME("Encoded in %f seconds\n") encode_main(argv);
  }
  
  return 0;
}
