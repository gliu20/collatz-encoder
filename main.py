""" main entry point for collatz encryption """
import argparse
import time
import bytes_file
import collatz


def init_parser():
    """ initializes parser """
    parser_result = argparse.ArgumentParser(
        prog="collatz_encrypt.py",
        description="Encryption using the collatz conjecture"
    )

    parser_result.add_argument('--key', dest='key', action='store')

    parser_result.add_argument('mode', choices=['encrypt', 'decrypt','encode','decode'])
    parser_result.add_argument('source_file', type=argparse.FileType('rb'))
    parser_result.add_argument('dest_file', type=argparse.FileType('wb'))

    return parser_result


def get_args(parser_result):
    """ gets arguments passed to program """
    args_result = vars(parser_result.parse_args())

    return args_result


if __name__ == "__main__":

    parser = init_parser()
    args = get_args(parser)

    start = time.process_time_ns()

    with args['source_file'] as source_file, args['dest_file'] as dest_file:
        mode = args['mode']
        print(mode + ' ' + source_file.name + '...')

        source_file = args['source_file']
        dest_file = args['dest_file']

        if mode == "encode":
            FILE_RESULT = bytes_file.read_file(source_file, collatz.encode_collatz)
            bytes_file.write_file(dest_file, FILE_RESULT)

        if mode == "decode":
            FILE_RESULT = bytes_file.read_file(source_file, collatz.decode_collatz)
            bytes_file.write_file(dest_file, FILE_RESULT)

        if mode == "encrypt":
            FILE_RESULT = bytes_file.read_file(source_file, collatz.encrypt)
            bytes_file.write_file(dest_file, FILE_RESULT)

        if mode == "decrypt":
            FILE_RESULT = bytes_file.read_file(source_file, collatz.decrypt)
            bytes_file.write_file(dest_file, FILE_RESULT)

        source_file.close()
        dest_file.close()



    duration = time.process_time_ns() - start
    print("done in ",duration / 1e6, " ms")
    