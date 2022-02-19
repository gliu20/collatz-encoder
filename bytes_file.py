"""
abstracts away handling binary files as integers
"""
import time
import bytes_int

def pass_through(num):
    """ null transformation on num """
    return num

def read_file(file, encoder=pass_through, is_verbose=False):
    """ reads file and transforms into an encoded integer """


    start = time.process_time_ns()

    file_bytes = file.read()
    file_int_encoding = bytes_int.to_int(file_bytes)

    if is_verbose:
        print(f'[Info] Read file in {(time.process_time_ns() - start) / 1e6} ms')

    start = time.process_time_ns()

    file_int_result = encoder(file_int_encoding)

    if is_verbose:
        print(f'[Info] Decoded file in {(time.process_time_ns() - start) / 1e6} ms')

    return file_int_result


def write_file(file, num, encoder=pass_through, is_verbose=False):
    """ transforms int into an encoded integer and stores it into a file """

    start = time.process_time_ns()

    file_int_encoding = encoder(num)

    if is_verbose:
        print(f'[Info] Encoded file in {(time.process_time_ns() - start) / 1e6} ms')
    

    start = time.process_time_ns()

    file_bytes = bytes_int.to_bytes_array(file_int_encoding)
    file.write(file_bytes)
    
    if is_verbose:
        print(f'[Info] Wrote file in {(time.process_time_ns() - start) / 1e6} ms')
