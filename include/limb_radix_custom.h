#pragma once

#include "limb_dlist.h"

void add(limb_dlist_t* a, limb_dlist_t* b);
void plus_one(limb_dlist_t* ll);
void minus_one(limb_dlist_t* ll);
void left_shift(limb_dlist_t* ll);
void right_shift(limb_dlist_t* ll);
void divide_by_three(limb_dlist_t* ll);
void multiply_by_three(limb_dlist_t* ll);
void multiply_by_three_and_increment(limb_dlist_t* ll);
void divide_by_three_optim(limb_dlist_t* ll, limb_dlist_t* buffer);
void fused_increment_divide_by_two(limb_dlist_t* ll);
