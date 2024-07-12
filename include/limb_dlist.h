#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include "limb.h"

#define LL_INITIAL_SIZE 16

typedef struct limb_dlist {
  size_t length;
  size_t container_size;
  limb_t *handle;
} limb_dlist_t;


limb_t* new_limb_handle(size_t container_size);
limb_dlist_t* new_limb_list();

/**
 * Utilities to insert and remove at tail
 * ---
 * These functions do not automatically resize
 * the limb list and expects the caller to call
 * grow and shrink to ensure the limb list is
 * properly sized
 */
void canonicalize(limb_dlist_t* ll);
void insert_at_tail(limb_dlist_t* ll, limb_t limb);
void remove_at_tail(limb_dlist_t* ll);

void grow_limb_list(limb_dlist_t* ll);
void shrink_limb_list(limb_dlist_t* ll);
bool is_well_sized(limb_dlist_t* ll, size_t length);


/**
 * resize_limb_list always resizes the container to the container_size
 * resize_limb_list_to_length can accept any length and find
 * an appropriate container_size and call resize_limb_list if necessary
 */
void resize_limb_list(limb_dlist_t* ll, size_t container_size);
void resize_limb_list_to_length(limb_dlist_t* ll, size_t length);

void destroy_limb_list(limb_dlist_t* ll);
void print_limb_list(limb_dlist_t* ll);

void swap_limb_list(limb_dlist_t* a, limb_dlist_t* b);
void copy_limb_list(limb_dlist_t* dest, limb_dlist_t* src);
