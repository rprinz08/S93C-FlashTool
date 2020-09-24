#!/usr/bin/python3

import sys
import re
import argparse
import binascii


IS_DEBUG = False
rx1 = re.compile(r"^\s*([0-9a-fA-F]{4})\s+:\s+([0-9a-fA-F]{16})(?:\s+:\s+([0-9a-fA-F]{2}))?\s*$")

def process(infile, outfile, use_crc=False):
    with open(outfile, 'wb') as f_out:
        with open(infile) as f_in:
            line = f_in.readline()
            line_cnt = 1

            while line:
                line = line.strip()

                if len(line) > 0 and line[0] != '#':
                    m = rx1.match(line)
                    if m:
                        addr = m.group(1)
                        data = m.group(2)
                        crc = m.group(3)

                        if len(data) % 2 != 0:
                            raise Exception("Input line ({}) data length must be "
                                            "multiple of 2"
                                            .format(line_cnt))

                        if use_crc and crc is None:
                            raise Error("Input line ({}) CRC missing"
                                        .format(line_cnt))

                        bin = binascii.unhexlify(data)
                        if use_crc:
                            crc = binascii.unhexlify(crc)[0]
                            c = 0
                            for i in bin:
                                c = c + i
                            c = c % 256
                            print("{:02x} == {:02x}".format(c, crc))
                            if crc != c:
                                raise Exception("Input line ({}) CRC ({:02x}) "
                                                "missmatch ({:02x})"
                                                .format(line_cnt, crc, c))
                        f_out.write(binascii.unhexlify(data))
                    else:
                        raise Exception("Input line ({}), addr ({}) format error"
                                        .format(line_cnt, addr))

                line = f_in.readline()
                line_cnt += 1


def main():
    parser = argparse.ArgumentParser(description='Convert HEX dump to binary file')
    parser.add_argument('infile', type=str, nargs='?',
                        help='Hex file as input',
                        default='dump.hex')
    parser.add_argument('outfile', type=str, nargs='?',
                        help='binary file as output',
                        default='dump.bin')
    parser.add_argument('-c', '--crc', action='store_true', help='enable CRC mode')
    parser.add_argument('-D', '--debug', action='store_true', help='enable debug mode')
    args = parser.parse_args()
    IS_DEBUG = args.debug

    print("Convert hex file ({}) to binary file ({})".format(
        args.infile, args.outfile))
    try:
        process(args.infile, args.outfile, args.crc)
    except Exception as ex:
        print("Error: {}".format(ex))
    print("Done")

if __name__ == "__main__":
    main()

