#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "limb.h"


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

limb_t* new_limb_handle(size_t size_power_2);
limb_vec_t* new_limb_list();
void grow_limb_list(limb_vec_t* ll);
void shrink_limb_list(limb_vec_t* ll);
void insert_at_tail(limb_vec_t* ll, limb_t limb);
void insert_at_head(limb_vec_t* ll, limb_t limb);
void remove_at_tail(limb_vec_t* ll);
void remove_at_head(limb_vec_t* ll);
void destory_limb_list(limb_vec_t* ll);
void print_limb_list(limb_vec_t* ll);
