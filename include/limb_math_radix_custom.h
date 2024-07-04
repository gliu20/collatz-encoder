#pragma once

#include "limb_list.h"

void add(limb_vec_t* a, limb_vec_t* b);
void plus_one(limb_vec_t* ll);
void minus_one(limb_vec_t* ll);
void left_shift(limb_vec_t* ll);
void right_shift(limb_vec_t* ll);
void divide_by_three(limb_vec_t* ll);
void fused_increment_divide_by_two(limb_vec_t* ll);