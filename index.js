
// returns encoding representation
// where the highest bit represents the i-th bit
// 1 bits represent using the odd branch
// 0 bits represent using the even branch
// the most significant bit (MSB) has no meaning
// and is present on all encodings (to make sure
// we always know the number of iterations we need
// for decoding)
const encodeCollatz = (input) => {
    const isEven = (bigInt) => (bigInt & 1n) === 0n;
    let encoding = 0n, i = 0n;

    while (input !== 1n) {
        if (isEven(input)) {
            input >>= 1n; // x / 2
        }
        else {
            const bigIntTimesThree = (input << 1n) + input; // 3 x
            input = (bigIntTimesThree + 1n) >> 1n; // (3 x + 1) / 2

            // set i-th bit of encoding to 1
            // this represents that we executed the odd branch
            // at the i-th position
            encoding |= (1n << i);
        }
        i++;
    }

    // mark the final i-th bit as set
    // so that we know the number of iterations
    return encoding | (1n << i);
}

const decodeCollatz = (input) => {
    const bitLengthOf = (input) => {
        let length = 0n;

        // keep shifting off bits until
        // we reach the Most Significant Bit
        // which is always set to 1 as defined
        // by our encoding
        while (input !== 1n) {
            length++;
            input >>= 1n;
        }

        return length;
    };

    const bitAtIndex = (input, index) => { 
        const bitMask = 1n << index;
        const bit = input & bitMask; 
        
        // right shift to bring bit down to 
        // a 1 or a 0 instead of a binary
        // number with lots of trailing 0s
        return bit >> index;
    }
    // collatz conjecture says that all numbers
    // when iterated by the collatz function
    // will always reach 1n. this means when we
    // apply in reverse, all numbers must start
    // from 1n and be transformed from there.
    // this is what we will do.
    let decoding = 1n;

    for (let i = bitLengthOf(input) - 1n; i >= 0n; i--) {
        // we defined the collatz function as
        // the modified format
        // where it is f(x) = {
        //      x / 2, x is even
        //      (3x + 1) / 2, x is odd
        // }
        // this means that for all branches,
        // we have to transform the decoding by
        // multiplying by 2

        decoding <<= 1n; // decoding *= 2

        if (bitAtIndex(input, i) !== 0n) {

            // these instructions are not
            // optimized with bitwise ops 
            // b/c there's no way to do so
            // (decoding - 1) / 3
            decoding -= 1n;
            decoding /= 3n;
        }
    }

    return decoding;
}