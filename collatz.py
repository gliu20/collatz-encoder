""" defines functions for encrypting and decrypting using collatz """
import time
from bytes_int import is_even, get_bit_at_index, get_bit_length

# we define the collatz function as
# f(x) = {
#   x / 2, x is even
#   (3x + 1) / 2, x is odd
# }

# block size is 512 bits
# this is secure because we've only tested collatz on
# numbers up to 2 ** 68, but those only encode to around
# 550 bits at maximum.
# At this level, speeding up calculations with lookup tables
# or any kind of trivial optimization, would require an unpractical
# amount of memory, so theoretically it would help stave off
# brute force attacks
BLOCK_SIZE_ORDER = 11
BLOCK_SIZE = 1 << BLOCK_SIZE_ORDER  # 2 ** 11 = 2048
# a bunch of 1s with bit_length block_size
BLOCK_SIZE_MASK = (1 << (BLOCK_SIZE_ORDER + 1)) - 1

# at its core, the collatz encoding is variable length so
# even trying to create asics to accelerate this would be difficult
# since it requires an indeterminate amount of ram


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
    # `get_bit_length(num) - 2` to (and including) `0`
    for i in range(get_bit_length(num) - 2, -1, -1):
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
    """ keep encoding num until it is above the target bit length """
    encoded_num = encode(num)
    target_num = 1 << target_bit_length  # 2 ** target_bit_length

    # TODO for performance, consider if comparing target_num is
    # cheaper than the alternative
    # equivalent to get_bit_length(encoded_num) < target_bit_length
    while encoded_num <= target_num:
        encoded_num = encode(encoded_num)

    return encoded_num


def get_front_block(num):
    """ gets the first block of a number """
    bits_to_chop = get_bit_length(num) - BLOCK_SIZE

    assert bits_to_chop > 0, "num must be greater than block size"

    return num >> bits_to_chop


def get_back_block(num):
    """ gets the last block of a number """
    return num & BLOCK_SIZE_MASK


def shrink_to_block_size(num, is_verbose=False):
    """ shrinks num to block size while removing info """

    start = time.process_time_ns()

    encoded_num_1 = encode(num)

    if is_verbose:
        print(
            f'[Info] Encoded key block 1 in {(time.process_time_ns() - start) / 1e6} ms')

    start = time.process_time_ns()

    encoded_num_2 = encode(encoded_num_1)

    if is_verbose:
        print(
            f'[Info] Encoded key block 2 in {(time.process_time_ns() - start) / 1e6} ms')

    start = time.process_time_ns()

    encoded_num_3 = encode(encoded_num_2)

    if is_verbose:
        print(
            f'[Info] Encoded key block 3 in {(time.process_time_ns() - start) / 1e6} ms')

    block_1 = get_front_block(encoded_num_1)
    block_2 = get_back_block(encoded_num_2)

    # theoretically encoded_num_3 should be large enough
    # so that front and back aren't overlapping
    block_3 = get_front_block(encoded_num_3)
    block_4 = get_back_block(encoded_num_3)

    # blocks 1 and 3 cancel out patterns in the front block
    # blocks 2 and 4 cancel out patterns in the back block
    # we merge this together as output
    return block_1 ^ block_2 ^ block_3 ^ block_4


def encrypt(key, is_verbose=False):
    """ returns a function that encrypts with the key provided """
    key = shrink_to_block_size(encode_to_length(key, BLOCK_SIZE), is_verbose)

    def encrypt_helper(num):
        # expand key to length of message
        msg = encode(num)
        msg_length = get_bit_length(msg) - 1
        msg_key = encode_to_length(key, msg_length)

        return msg ^ msg_key
    return encrypt_helper


def decrypt(key, is_verbose=False):
    """ returns a function that decrypts with the key provided """
    key = shrink_to_block_size(encode_to_length(key, BLOCK_SIZE), is_verbose)

    def decrypt_helper(num):
        # expand key to length of message
        msg_length = get_bit_length(num) - 1
        msg_key = encode_to_length(key, msg_length)

        return decode(num ^ msg_key)
    return decrypt_helper
