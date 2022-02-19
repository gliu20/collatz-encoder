""" defines functions for encrypting and decrypting using collatz """

from bytes_int import is_even, get_bit_at_index, get_bit_length

# we define the collatz function as
# f(x) = {
#   x / 2, x is even
#   (3x + 1) / 2, x is odd
# }


def encode(num):
    """ takes num and encodes it into collatz (also a num)"""
    encoding = 0
    i = 0

    while num != 1:
        if is_even(num):
            # x / 2
            num >>= 1
        else:
            num_times_three = (num << 1) + num  # 3x
            num = (num_times_three + 1) >> 1  # (3x + 1) / 2

            # set i-th bit to 1
            encoding |= (1 << i)
        i += 1

    # we mark the final (MSB) i-th bit as 1 so we know
    # the total amount of iterations for decoding
    return encoding | (1 << i)


def decode(num):
    """ decode collatz encoding"""
    # we start at 1 because by the collatz conjecture, we know
    # that everything is supposed to eventually make its way to 1
    decoding = 1

    # stop is -1 because we want to go from
    # `get_bit_length(num) - 1` to (and including) `0`
    for i in range(get_bit_length(num) - 1, -1, -1):
        # since we defined the collatz function
        # so that x / 2 is performed no matter the branch,
        # we must reverse that in all cases
        decoding <<= 1

        if get_bit_at_index(num, i) != 0:
            # reverse the odd branch of the collatz function
            # decoding = (decoding - 1) / 3
            decoding -= 1
            decoding //= 3  # divide as ints

    return decoding


def encode_to_length(num, target_bit_length):
    """ keep encoding num until it is the desired bit length """
    encoded_num = encode(num)
    target_num = 1 << target_bit_length  # 2 ** target_bit_length

    # TODO for performance, consider if comparing target_num is
    # cheaper than the alternative
    # equivalent to get_bit_length(encodeed_num) < target_bit_length
    while encoded_num <= target_num:
        encoded_num = encode(encoded_num)

    return encoded_num


def encrypt(key):
    """ returns a function that encrypts with the key provided """
    def encrypt_helper(num):
        return num
    return encrypt_helper


def decrypt(key):
    """ returns a function that decrypts with the key provided """
    def decrypt_helper(num):
        return num
    return decrypt_helper
