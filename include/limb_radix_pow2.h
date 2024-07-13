#pragma once

#include "limb_dlist.h"

void set_ith_bit(limb_dlist_t* ll, size_t bit_index);
limb_t get_ith_bit(limb_dlist_t* ll, size_t bit_index);
size_t get_bit_length(limb_dlist_t* ll);
