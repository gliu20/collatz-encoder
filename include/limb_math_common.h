#pragma once

#include <stdbool.h>
#include "limb_list.h"

bool is_even(limb_vec_t* ll);
void pad_zero(limb_vec_t* ll);
void pad_to_length(limb_vec_t* ll, size_t length);
void guard_against_empty(limb_vec_t* ll);
void guard_against_overflow(limb_vec_t* ll);

bool is_eq_one(limb_vec_t* ll);
bool is_eq(limb_vec_t* ll_a, limb_vec_t* ll_b);
size_t get_bit_length(limb_vec_t* ll) ;

