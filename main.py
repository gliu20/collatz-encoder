""" main entry point for collatz encryption """
import argparse
import time
import collatz


def init_parser():
    """ initializes parser """
    parser_result = argparse.ArgumentParser(
        prog="collatz_encrypt.py",
        description="Encryption using the collatz conjecture"
    )

    parser_result.add_argument('--key', dest='key', action='store')

    parser_result.add_argument('mode', choices=['encrypt', 'decrypt'])
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

    with args['source_file'] as source_file, args['dest_file'] as dest_file:
        print(args['mode'] + 'ing ' + source_file.name + '...')

    #print(collatz.encode_collatz(123456))
    start = time.process_time_ns()

    print(collatz.decode_collatz(collatz.encode_collatz(302259724151636153425594064330573516206175434074410015247432505138313889132724302533056815511972705930121804821991786330777142588226069160857513564677062105160430282709875299083238693171443220569300631794525976806604329232433799742762670892798300960706490402006030336405212956982107283777330360030656102243038692310366075321319470089702014492386775349940722556817976056396461070965489560498813829919708664822124420190184034084759969755024141883019429800104841944223829086764974972355979158874803431083974232441380384038430393150443731344808589671556765713097714148451687342938999384426211997190106910675496037652687440839860922602355398798426609527011016356723256925459161040255004317361975528466129669356586812913514508642410715024143080)))

    duration = time.process_time_ns() - start
    print("done in ",duration / 1e6, " ms")
    #source_file = args['source_file']
    #dest_file = args['dest_file']
