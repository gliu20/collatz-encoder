# Collatz encoder
A fast arbitrary-precision unsigned integer arithmetic library written in C optimized for computing numbers relevant to the Collatz Conjecture

# Optimizations
- Uses `2**odd - 2` as the radix to allow 
    - `O(n)` division by two and three (parallelizable to `O(1)` span complexity assuming `n` processors)
    - `O(n)` multiplication by two, addition, increment, and decrement (parallelizable to `O(log n)` span complexity assuming `n` processors)
    - `O(1)` `is_even` check
- Can easily convert between `2**n` and `2**odd - 2` radix
    - `O(n^2)` read bit by bit using right shift and `is_even` to convert into the `2**n` radix representation
    - `O(n^2)` left shift bit by bit and set the least significant bit based on `is_even` check to convert into `2**odd - 2` radix representation

# Current work in progress
- Enabling vectorization to allow SIMD optimizations
- Parallelizing with OpenMP
