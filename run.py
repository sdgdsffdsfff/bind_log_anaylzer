
import os
from sys import argv

def split_info(line):
    exp = line.split(' ')
    if len(exp) != 10:
        return None
    ip, port = exp[4].split('#')
    return ip
    pass

def anayse_file(f, d):
    dst = open(d, 'w')
    for line in open(f).xreadlines():
        ip = split_info(line)
        if ip:
            dst.write(ip + '\n')
    dst.close()

def proc_all():
    for root, dirs, files in os.walk('./'):
        print files
    pass

if __name__ == '__main__':
    # anayse_file(argv[1], argv[2])
    proc_all()