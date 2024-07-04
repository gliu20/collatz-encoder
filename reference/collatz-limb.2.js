const LIMB_BIT_LENGTH = 8;
const LIMB_NUM_VALUES = 2 ** LIMB_BIT_LENGTH;
const LIMB_INT_MAX = LIMB_NUM_VALUES - 1;
const LIMB_MSB_INDEX = LIMB_BIT_LENGTH - 1;
const LIMB_MSB_MASK = 2 ** LIMB_MSB_INDEX;
const LIMB_CARRY_MASK = 2 ** LIMB_BIT_LENGTH;

const LIMB_DIVIDE_THREE_LUT = [
    0,
    (LIMB_INT_MAX / 3) | 0,
    (2 * LIMB_INT_MAX / 3) | 0
];

function to_limbs(int) {
    const limb_base = BigInt(LIMB_NUM_VALUES);
    const limbs = [];
    while (int != 0n) {
        limbs.push(parseInt(int % limb_base));
        int /= limb_base;
    }
    return limbs;
}

function to_int(limbs) {
    const limb_bits = BigInt(LIMB_BIT_LENGTH);
    let int = 0n;
    // From most significant limbs to least significant
    for (let i = limbs.length - 1; i >= 0; i--) {
        int <<= limb_bits;
        int += BigInt(limbs[i]);
    }
    return int;
}

function is_even(limbs) {
    return (limbs[0] & 1) === 0;
}

function left_shift(limbs) {
    const will_overflow = limbs.at(-1) & LIMB_MSB_MASK;
    if (will_overflow) limbs.push(0);
    let carry = 0;
    for (let i = 0; i < limbs.length; i++) {
        const result = (limbs[i] << 1) | carry;
        carry = (limbs[i] & LIMB_MSB_MASK) >> LIMB_MSB_INDEX;
        limbs[i] = result & LIMB_INT_MAX;
    }
    return limbs;
}

function right_shift(limbs) {
    let carry = 0;
    for (let i = limbs.length - 1; i >= 0; i--) {
        const result = (limbs[i] >> 1) | carry;
        carry = (limbs[i] & 1) << LIMB_MSB_INDEX;
        limbs[i] = result & LIMB_INT_MAX;
    }
    return limbs;
}

function plus_one(limbs) {
    const might_overflow = limbs.at(-1) & LIMB_MSB_MASK;
    if (might_overflow) limbs.push(0);
    for (let i = 0; i < limbs.length; i++) {
        const result = limbs[i] + 1;
        const carry = result & LIMB_CARRY_MASK;
        limbs[i] = result & LIMB_INT_MAX;
        if (!carry) break;
    }
    return limbs;
}

function minus_one(limbs) {
    let carry = 0;
    for (let i = 0; i < limbs.length; i++) {
        const result = limbs[i] + LIMB_INT_MAX + carry;
        carry = (result & LIMB_CARRY_MASK) >> LIMB_BIT_LENGTH;
        limbs[i] = result & LIMB_INT_MAX;
    }
    return limbs;
}

function add(limbs1, limbs2) {
    if (limbs1.length < limbs2.length) 
        [limbs1, limbs2] = [limbs2, limbs1];
    
    const might_overflow = limbs1.at(-1) & LIMB_MSB_MASK;
    if (might_overflow) limbs1.push(0);
    
    let carry = 0;
    for (let i = 0; i < limbs1.length; i++) {
        const result = (limbs1[i] ?? 0) + (limbs2[i] ?? 0) + carry;
        carry = (result & LIMB_CARRY_MASK) >> LIMB_BIT_LENGTH;
        limbs1[i] = result & LIMB_INT_MAX;
    }
    return limbs1;
}

function divide_by_three_helper(limbs, limb_index) {
    const divide_result = (limbs[limb_index] / 3) | 0;
    const remainder = limbs[limb_index] % 3;
    const correction_factor = LIMB_DIVIDE_THREE_LUT[remainder];
    const result_limbs = new Array(limb_index + 1).fill(correction_factor);
    result_limbs[limb_index] = divide_result;
    
    // Add an extra correction factor for extra precision for the last limb
    result_limbs.unshift(correction_factor);
    return result_limbs;
}

// This function is only valid for numbers that when divided by 3
// result in a number that is odd. This is guaranteed when
// reversing a Collatz encoding
function divide_by_three(limbs) {
    let result_limbs = [0];
    for (let i = 0; i < limbs.length; i++) {
        result_limbs = add(divide_by_three_helper(limbs, i), result_limbs);
    }
    // Remove the extra correction factor
    result_limbs.shift();
    // Guarantee an odd result per what's expected for reversing a Collatz encoding
    result_limbs[0] |= 1;
    return result_limbs;
}

function set_ith_bit(limbs, bit_index) {
    const desired_byte = (bit_index / LIMB_BIT_LENGTH) | 0;
    const desired_bit = bit_index % LIMB_BIT_LENGTH;
    
    while (desired_byte >= limbs.length) {
        limbs.push(0);
    }
    
    limbs[desired_byte] |= 1 << desired_bit;
    return limbs;
}

function get_ith_bit(limbs, bit_index) {
    const desired_byte = (bit_index / LIMB_BIT_LENGTH) | 0;
    const desired_bit = bit_index % LIMB_BIT_LENGTH;
    const bit_at_index = limbs[desired_byte] & (1 << desired_bit);
    
    return bit_at_index;
}

function canonicalize(limbs) {
    while (limbs.at(-1) === 0) {
        limbs.pop();
    }
    return limbs;
}

function eq_one(limbs) {
    canonicalize(limbs);
    if (limbs.length !== 1) return false;
    if (limbs[0] != 1) return false;
    return true;
}

function get_bit_length(limbs) {
    canonicalize(limbs);
    if (limbs.length === 0) return 0;
    const available_bits = limbs.length * LIMB_BIT_LENGTH;
    let most_significant_byte = limbs.at(-1);
    let used_bits = 0;
    while (most_significant_byte !== 0) {
        used_bits++;
        most_significant_byte >>= 1;
    }
    return available_bits + used_bits - LIMB_BIT_LENGTH;
}

function collatz_encode(limbs) {
    const encoding = [];
    let i = 0;
    while (!eq_one(limbs)) {
        if (is_even(limbs)) {
            // x / 2
            right_shift(limbs)
        }
        else {
            // (3 x + 1) / 2 = x + (x + 1) / 2 = x + ((x + 1) >> 1)
            // since x is odd: = x + (x >> 1) + 1
            limbs = plus_one(add(limbs, right_shift(limbs.slice(0))));
            set_ith_bit(encoding, i);
        }
        i++;
    }
    set_ith_bit(encoding, i);
    return encoding;
}

function collatz_decode(limbs) {
    let decoding = [1];
    const bit_length = get_bit_length(limbs);
    
    for (let i = bit_length - 2; i >= 0; i--) {
        left_shift(decoding);
        
        if (get_ith_bit(limbs, i) !== 0) {
            minus_one(decoding);
            decoding = divide_by_three(decoding);
        }
    } 
    
    return decoding;
}
