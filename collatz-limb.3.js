
// When the limb base is ((2 ** odd) - 2) it is divisible by both
// 2 and 3 which allows us to division by 2 and 3 in O(n) where
// n is the number of limbs
const LIMB_BIT_LENGTH = 7n;
const LIMB_BASE = 2n ** LIMB_BIT_LENGTH - 2n;
const LIMB_MAX_VAL = LIMB_BASE - 1n;
const LIMB_DIVIDE_BY_TWO_CORRECTION = LIMB_BASE / 2n;
const LIMB_DIVIDE_BY_THREE_CORRECTION = LIMB_BASE / 3n;

const property_tests_unary = {
    //test_round_trip_limbs: (int) => to_int(to_limbs(int)) === int,
    //test_is_even_even: (int) => is_even(to_limbs(2n * int)),
    //test_is_even_odd: (int) => !is_even(to_limbs(2n * int + 1n)),
    test_divide_by_two: (int) => to_int(divide_by_two(to_limbs(int))) == (int / 2n),
    test_divide_by_three: (int) => to_int(divide_by_three(to_limbs(int))) == (int / 3n),
    test_multiply_by_two: (int) => to_int(multiply_by_two(to_limbs(int))) == (int * 2n),
    test_plus_one: (int) => to_int(plus_one(to_limbs(int))) == (int + 1n),
    test_minus_one: (int) => to_int(minus_one(to_limbs(int))) == (int - 1n),
};

const property_tests_binary = {
    test_add: (int_a, int_b) => to_int(add(to_limbs(int_a), to_limbs(int_b))) == (int_a + int_b),
};

const rand_int_helper = () => Math.random() * 2 ** 31 | 0;
const rand = {
    rand_int: () => BigInt(rand_int_helper()),
    rand_even: () => rand.rand_int() * 2n,
    rand_odd: () => rand.rand_int() * 2n + 1n,
    rand_power_two: () => 2n ** (rand.rand_int() % 2048n),
    rand_power_two_plus_one: () => rand.rand_power_two() + 1n,
    rand_power_two_minus_one: () => rand.rand_power_two() - 1n,
    rand_power_two_plus_two: () => rand.rand_power_two() + 2n,
    rand_power_two_minus_two: () => rand.rand_power_two() - 2n,
    rand_power_two_plus_three: () => rand.rand_power_two() + 3n,
    rand_power_two_minus_three: () => rand.rand_power_two() - 3n,
    rand_power_base: () => LIMB_BASE ** (rand.rand_int() % 2048n),
    rand_power_base_minus_one: () => rand.rand_power_base() - 1n,
    rand_power_base_plus_one: () => rand.rand_power_base() + 1n,
}
const rand_rand_int = () => {
    const rand_types = Object.values(rand);
    const __rand_rand_int = () => rand_types[rand_int_helper() % rand_types.length]();
    
    let rand_value = __rand_rand_int();
    while (rand_value < 0n) {
        console.log('rejected neg value');
        rand_value = __rand_rand_int();
    }
    return rand_value;
};

function test() {
    const num_tests = 2048n;
    let tests_passed = {};
    for (let i = 0n; i < num_tests; i++) {
        const input_a = rand_rand_int();
        const input_b = rand_rand_int();
        for (let [test_name, test_func] of Object.entries(property_tests_unary)) {
            const result = test_func(input_a);
            tests_passed[test_name] ??= 0;
            tests_passed[test_name] += result;
            console.assert(result, `${test_name}: failed on ${input_a}`);
        }
        for (let [test_name, test_func] of Object.entries(property_tests_binary)) {
            const result = test_func(input_a, input_b);
            tests_passed[test_name] ??= 0;
            tests_passed[test_name] += result;
            console.assert(result, `${test_name}: failed on ${input_a},${input_b}`);
        }
    }
    console.table(tests_passed);
}

function to_limbs(int) {
    const limbs = [];
    while (int != 0n) {
        limbs.push(parseInt(int % LIMB_BASE));
        int /= LIMB_BASE;
    }
    return limbs;
}

function to_int(limbs) {
    let int = 0n;
    // From most significant limbs to least significant
    for (let i = limbs.length - 1; i >= 0; i--) {
        int *= LIMB_BASE;
        int += BigInt(limbs[i]);
    }
    return int;
}

function is_even(limbs) {
    return (limbs[0] & 1) === 0;
}
/*
function carry_propagate(limbs, expr) {
    
    let carry = 0;
    for (let i = 0; i < limbs.length; i++) {
        const result = expr(i) + carry;
        const result_mod_base = BigInt(result) % LIMB_BASE;
        limbs[i] = parseInt(result_mod_base);
        carry = parseInt(BigInt(result) / LIMB_BASE);
    }
    
    return limbs;
}

carry_propagate = carry_propagate_optim;*/

function carry_propagate(limbs, expr) {
    
    let overflow = 0;
    let carries = new Array(limbs.length).fill(0);
    
    
    for (let i = 0; i < limbs.length; i++) {
        const result = expr(i);
        limbs[i] = parseInt(BigInt(result) % LIMB_BASE);
        carries[i] = parseInt(BigInt(result) / LIMB_BASE);
        overflow += carries[i];
    }
    
    for (let i = 1; i < limbs.length; i++) {
        const result = limbs[i] + carries[i - 1];
        limbs[i] = parseInt(BigInt(result) % LIMB_BASE);
        carries[i] = parseInt(BigInt(result) / LIMB_BASE);
        overflow += carries[i];
    }
    
    console.assert(overflow === 0, 'err: invalid result due to overflow');
    
    return limbs;
}

function add(a, b) {
    if (a.length < b.length) [a, b] = [b, a];
    
    const might_overflow = a.at(-1);
    if (might_overflow) a.push(0);
    
    return carry_propagate(a, i => (a[i] ?? 0) + (b[i] ?? 0));
}

function plus_one(limbs) {
    if (limbs.length === 0) return [1];
    const might_overflow = limbs.at(-1);
    if (might_overflow) limbs.push(0);
    
    return carry_propagate(limbs, i => limbs[i] + (i == 0));
}

function minus_one(limbs) {    
    if (limbs.length === 0) return [-1];
    return carry_propagate(limbs, i => limbs[i] + parseInt(LIMB_MAX_VAL));
}

function divide_by_two(limbs) {
    if (limbs.length === 0) return limbs;
    
    // Since LIMB_MAX_VAL / 2 + LIMB_DIVIDE_BY_TWO_CORRECTION = MAX_VAL:
    // we dont have to carry propagate because it cant possibly overflow
    return limbs.map((_, i) => (limbs[i] >> 1) + ((limbs[i + 1] ?? 0) & 1) * parseInt(LIMB_DIVIDE_BY_TWO_CORRECTION));
}

function divide_by_three(limbs) {
    if (limbs.length === 0) return limbs;
    
    // Since LIMB_MAX_VAL / 3 + LIMB_DIVIDE_BY_THREE_CORRECTION = MAX_VAL:
    // we dont have to carry propagate because it cant possibly overflow
    return limbs.map((_, i) => ((limbs[i] / 3) | 0) + ((limbs[i + 1] ?? 0) % 3) * parseInt(LIMB_DIVIDE_BY_THREE_CORRECTION));
}

function multiply_by_two(limbs) {
    if (limbs.length === 0) return limbs;
    
    const might_overflow = limbs.at(-1);
    if (might_overflow) limbs.push(0);
    
    return carry_propagate(limbs, i => (limbs[i] << 1));
}

test();
