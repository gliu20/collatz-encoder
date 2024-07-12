#include "limb_radix_common.h"
#include "limb_radix_pow2.h"

void set_ith_bit(limb_vec_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    pad_to_length(ll, desired_limb + 1);
    
    LL_INDEX(ll, desired_limb) |= ((limb_t) 1) << desired_bit;
}

limb_t get_ith_bit(limb_vec_t* ll, size_t bit_index) {
    size_t desired_limb = bit_index / LIMB_CONTAINER_BIT_LENGTH;
    size_t desired_bit = bit_index % LIMB_CONTAINER_BIT_LENGTH;
    
    return LL_INDEX(ll, desired_limb) & (((limb_t) 1) << desired_bit);
}
