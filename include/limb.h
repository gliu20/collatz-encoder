#pragma once

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

