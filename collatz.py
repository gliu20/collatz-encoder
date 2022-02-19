""" defines functions for encrypting and decrypting using collatz """

##### utility functions

is_even = lambda num : num & 1 == 0

def get_bit_length(num):
    """ gets bit length """
    length = 0

    # guard against infinite loop on invalid args
    assert num >= 1, "input number must be greater than or equal to 1"

    # the MSB must be 1 because we are working
    # with arbitrarily large integers; once we
    # reach 1, we know to stop
    while num != 1:
        length += 1
        num >>= 1

    return length

def get_bit_at_index(num, index):
    """ gets a single bit at the specified index from num """
    bit_mask = 1 << index
    bit = num & bit_mask

    # we have to right shift to the bit to get rid of all
    # the trailing zeros
    return bit >> index

##### main functions

# we define the collatz function as
# f(x) = {
#   x / 2, x is even
#   (3x + 1) / 2, x is odd
# }

def encode_collatz(num):
    """ takes num and encodes it into collatz (also a num)"""
    encoding = 0
    i = 0

    while num != 1:
        if is_even(num):
            # x / 2
            num >>= 1
        else:
            num_times_three = (num << 1) + num # 3x
            num = (num_times_three + 1) >> 1 # (3x + 1) / 2

            # set i-th bit to 1
            encoding |= (1 << i)
        i += 1

    # we mark the final (MSB) i-th bit as 1 so we know
    # the total amount of iterations for decoding
    return encoding | (1 << i)

def decode_collatz(num):
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
            decoding //= 3 # divide as ints

    return decoding
