
const _bitLengthOf = (input) => {
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

    for (let i = _bitLengthOf(input) - 1n; i >= 0n; i--) {
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

const encodeString = (input) => {
    const charBitLength = 7n;
    const isValidChar = (charCode) => charCode < 127n;

    let encoding = 0n;

    for (let i = 0; i < input.length; i++) {
        const charCode = BigInt(input.charCodeAt(i));
        const bitOffset = charBitLength * BigInt(i);
        if (isValidChar(charCode)) {
            encoding |= charCode << bitOffset;
        }
        // we siliently ignore invalid characters
    }

    return encoding;
}

const decodeString = (input) => {
    const charBitLength = 7n;
    const bitMask = 127n;
    let decoding = "";

    while (input !== 0n) {
        const charCode = Number(input & bitMask);

        // add decoded char to back and move on to next char
        decoding += String.fromCharCode(charCode);
        input >>= charBitLength;
    }

    return decoding;
}

const _encodeKeyToLength = (key, desiredBitLength) => {
    let encodedKey = encodeCollatz(key);
    let targetValue = 1n << desiredBitLength; // 2 ** desiredBitLength

    // equivalent to _bitLengthOf(encoding) < desiredBitLength
    while (encodedKey <= targetValue) {
        encodedKey = encodeCollatz(encodedKey);
    }

    return encodedKey;
}

const _encodeKeyToLengthAndDeleteInfo = (key, desiredBitLength) => {
    const minBitLength = 2048n;
    const deleteBitLength = minBitLength >> 1n;

    let encodedKey = _encodeKeyToLength(key, minBitLength) >> deleteBitLength;
    let targetValue = 1n << desiredBitLength; // 2 ** desiredBitLength

    // equivalent to _bitLengthOf(encoding) < desiredBitLength
    while (encodedKey <= targetValue) {
        // we expand key by collatz encoding it and then we
        // delete the least significant bits 
        // so that reversing is more difficult because
        // information has been lost
        encodedKey = encodeCollatz(encodedKey);
        encodedKey >>= deleteBitLength;
    }

    return encodedKey;
}

const encryptCollatz = (input, key) => {
    const plainTextEncoding = encodeCollatz(encodeString(input));
    const plainTextEncodingLength = _bitLengthOf(plainTextEncoding);
    const encodedKey = _encodeKeyToLengthAndDeleteInfo(encodeString(key), plainTextEncodingLength);
    const cipherTextEncoding = encodedKey ^ plainTextEncoding;

    console.log(encodedKey.toString(2));
    console.log(plainTextEncoding.toString(2));
    console.log(cipherTextEncoding.toString(2));

    return cipherTextEncoding;
}

const decryptCollatz = (input, key) => {
    const cipherText = input;
    const cipherTextLength = _bitLengthOf(cipherText);
    const encodedKey = _encodeKeyToLengthAndDeleteInfo(encodeString(key), cipherTextLength);

    const plainTextEncoding = encodedKey ^ cipherText;

    return decodeString(decodeCollatz(plainTextEncoding));
}