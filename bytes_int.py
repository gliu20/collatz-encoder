"""
utility functions for converting between byte arrays and ints
and also dealng with common bit operations with ints
"""


def is_even(num):
    """ determine if num is even """
    return num & 1 == 0


def get_bit_length_legacy(num):
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


def get_bit_length(num):
    """ gets bit length """
    # we have to subtract by 1 because we don't count the MSB
    # as part of the bit length (for legacy reasons)
    return num.bit_length() - 1


def get_bit_at_index(num, index):
    """ gets a single bit at the specified index from num """
    bit_mask = 1 << index
    bit = num & bit_mask

    # we have to right shift to the bit to get rid of all
    # the trailing zeros
    return bit >> index

# we're assuming everything is big-endian


def to_int(bytes_array):
    """ convert to int """
    return int.from_bytes(bytes_array, byteorder='big', signed=False)


def to_bytes_array(num):
    """ convert int to bytes array """
    bit_length = get_bit_length(num)
    byte_length = (bit_length // 8) + 1

    return num.to_bytes(byte_length, 'big')
