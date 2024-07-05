# Collatz encoder
A fast arbitrary-precision unsigned integer arithmetic library written in C optimized for computing numbers relevant to the Collatz Conjecture

# Optimizations
- Uses `2**odd - 2` as the radix to allow 
    - `O(n)` division by two and three (parallelizable to `O(1)` span complexity assuming `n` processors)
    - `O(n)` multiplication by two, addition, increment, and decrement (parallelizable to `O(log n)` span complexity assuming `n` processors)
    - `O(1)` `is_even` check
- Specialized dynamic array implementation using a circular buffer to allow `O(1)` inserts at both head and tail

# Current work in progress
- Enabling vectorization to allow SIMD optimizations
- Parallelizing with OpenMP