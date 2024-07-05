#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "limb.h"

#define LIMB_LIST_INITIAL_HEAD_OFFSET 4
#define LIMB_LIST_INITIAL_SIZE 4

#define LL_POWER_2_TO_SIZE(POWER_2) (1u << (POWER_2))
#define LL_POWER_2_TO_MASK(POWER_2) ((1u << (POWER_2)) - 1u)
#define LL_CIRCULAR_INDEX_MASK(LL, I, MASK) (((I) + (LL)->head_offset) & (MASK))
#define LL_CIRCULAR_INDEX(LL, I) (LL_CIRCULAR_INDEX_MASK(LL, I, LL_POWER_2_TO_MASK((LL)->size_power_2)))

#define LL_INDEX(LL, I) (LL)->handle[LL_CIRCULAR_INDEX(LL, I)]

/**
 * Pay special attention to the LL_TAIL macro;
 * make sure you don't access OOB if ll->length is 0
 */
#define LL_HEAD(LL) LL_INDEX(LL, 0)
#define LL_TAIL(LL) LL_INDEX(LL, (LL)->length - 1u)

typedef struct limb_vec {
  size_t head_offset;
  size_t size_power_2;
  size_t length;
  limb_t *handle;
} limb_vec_t;

limb_t* new_limb_handle(size_t size_power_2);
limb_vec_t* new_limb_list();


void canonicalize(limb_vec_t* ll);
void copy_limb_list(limb_vec_t* dest, limb_vec_t* src);

void grow_limb_list(limb_vec_t* ll);
bool shrink_limb_list(limb_vec_t* ll);
void resize_limb_list_to_length(limb_vec_t* ll, size_t length);
void grow_limb_list_to_length(limb_vec_t* ll, size_t length);
void shrink_limb_list_to_length(limb_vec_t* ll, size_t length);


/**
 * Utilities to insert and remove at head/tail
 * ---
 * These functions do not automatically resize
 * the limb list and expects the caller to call
 * grow and shrink to ensure the limb list is
 * properly sized
 */
void insert_at_tail(limb_vec_t* ll, limb_t limb);
void insert_at_head(limb_vec_t* ll, limb_t limb);
void remove_at_tail(limb_vec_t* ll);
void remove_at_head(limb_vec_t* ll);

void destroy_limb_list(limb_vec_t* ll);
void print_limb_list(limb_vec_t* ll);
