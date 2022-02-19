"""
abstracts away handling binary files as integers
"""
import bytes_int

def pass_through(num):
    """ null transformation on num """
    return num

def read_file(file, encoder=pass_through):
    """ reads file and transforms into an encoded integer """
    file_bytes = file.read()
    file_int_encoding = bytes_int.to_int(file_bytes)
    file_int_result = encoder(file_int_encoding)

    return file_int_result


def write_file(file, num, encoder=pass_through):
    """ transforms int into an encoded integer and stores it into a file """

    file_int_encoding = encoder(num)
    file_bytes = bytes_int.to_bytes_array(file_int_encoding)

    file.write(file_bytes)
