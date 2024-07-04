const LIMB_BIT_LENGTH = 8;
const LIMB_MAX_VALUES = 2 ** LIMB_BIT_LENGTH;
const LIMB_INT_MAX = LIMB_MAX_VALUES - 1;
const LIMB_HIGH_BIT = 2 ** (LIMB_BIT_LENGTH - 1);

function big_int_to_limbs (int) {
    const limbs = [];
    while (int != 0n) {
        limbs.push(parseInt(int % BigInt(LIMB_MAX_VALUES)))
        int /= BigInt(LIMB_MAX_VALUES);
    }
    return limbs;
}
function limbs_to_big_int (limbs) {
    let int = 0n;
    for (let i = limbs.length - 1; i >= 0; i--) {
        int <<= BigInt(LIMB_BIT_LENGTH);
        int += BigInt(limbs[i]);
    }
    return int;
}

function is_even(limbs) {
    return limbs[0] & 0b1 === 0b1;
}

function times_two(limbs) {
    const hasSetHighBit = limbs.at(-1) & LIMB_HIGH_BIT;
    
    if (hasSetHighBit) {
        limbs.push(0);
    }
    
    for (let i = limbs.length - 1; i > 0; i--) {
        const high_bit = (limbs[i - 1] & LIMB_HIGH_BIT) >> (LIMB_BIT_LENGTH - 1);
        
        limbs[i] = (limbs[i] << 1) & LIMB_INT_MAX;
        limbs[i] |= high_bit;
    }
    
    limbs[0] = (limbs[0] << 1) & LIMB_INT_MAX;
    
    return limbs;
}

function divide_two(limbs) {
    for (let i = 0; i < limbs.length - 1; i++) {
        const high_bit = (limbs[i + 1] & 0b1) << (LIMB_BIT_LENGTH - 1);
        
        limbs[i] >>= 1;
        limbs[i] |= high_bit;
    }
    
    limbs[limbs.length - 1] >>= 1;
    
    return limbs;
}

function plus_one(limbs) {
    if (limbs.at(-1) !== 0) {
        limbs.push(0);
    }
    
    for (let i = 0; i < limbs.length; i++) {
        const result = limbs[i] + 1;
        const carry = result & LIMB_MAX_VALUES
        
        limbs[i] = result & LIMB_INT_MAX;
        
        if (!carry) break;
    }
    
    return limbs;
}


function add(limbs1, limb2) {
    let len1 = limbs1.length;
    let len2 = limbs2.length;
    let max_len = Math.max(len1, len2);
    
    if (len1 < len2) {
        const temp = limbs1;
        limbs1 = limbs2;
        limbs2 = temp;
        
        len1 = limbs1.length;
        len2 = limbs2.length;
    }
    
    
    if (limbs1.at(-1) !== 0) {
        limbs1.push(0);
        len1 = limbs1.length;
    }
    
    let i;
    for (i = 0; i < (len1 - 1) && i < (len2 - 1); i++) {
        const result = limbs1[i] + limbs2[i];
        const carry = result & LIMB_MAX_VALUES;
    
        limbs1[i] = result & LIMB_INT_MAX;
        limbs1[i + 1] += carry;
    }
    
    for (; i < max_len; i++) {
        
    }
    return limbs1;
}
