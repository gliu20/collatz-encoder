#pragma once

#include <stdbool.h>
#include "limb_dlist.h"

bool is_even(limb_dlist_t* ll);
void pad_zero(limb_dlist_t* ll);
void pad_to_length(limb_dlist_t* ll, size_t length);
void guard_against_empty(limb_dlist_t* ll);
void guard_against_overflow(limb_dlist_t* ll);

bool is_eq_one(limb_dlist_t* ll);
bool is_eq(limb_dlist_t* ll_a, limb_dlist_t* ll_b);

