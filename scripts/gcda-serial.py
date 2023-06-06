#!/usr/bin/env python3

from argparse import ArgumentParser
from os import path
from sys import exit
from re import search
from enum import Enum
from dataclasses import dataclass
from typing import Generator,List
from subprocess import check_call

class ScanState(Enum):
    IDLE = 0
    EMIT = 1
    BYTE = 2
    FILE = 3
    DATA = 4

@dataclass
class ScanFile:
    file: str
    size: int
    data: List[str]

    def vailed(self):
        return len(self.data) == self.size
    
    def write(self):
        if self.vailed():
            data = bytearray(map(lambda x: int(x,16), self.data))
            with open(self.file, 'wb+') as f:
                f.write(data)
                f.flush()
                return True
        return False


def scan(file) -> Generator[ScanFile, None, None]:
    with open(file, 'r') as log:
        state = ScanState.IDLE
        sfile = ScanFile('none', 0, [])
        while line := log.readline():
            line = line.strip()
            match state:
                case ScanState.IDLE:
                    if line == 'Emitting':
                        state = ScanState.EMIT
                case ScanState.EMIT:
                    match = search(r'(.*) bytes for', line)
                    if match:
                        sfile.size = int(match.group(1))
                        state = ScanState.FILE
                case ScanState.FILE:
                    sfile.file = line
                    state = ScanState.DATA
                case ScanState.DATA:
                    match = search(r'(.*): (.*)', line)
                    if match:
                        sfile.data.extend(match.group(2).split())
                    if sfile.file == line:
                        yield sfile
                        sfile = ScanFile('none', 0, [])
                        state = ScanState.IDLE

parser = ArgumentParser('gcda-2serial')
parser.add_argument('file',help='choose on file')

if __name__ == '__main__':
    args = parser.parse_args()
    if not path.exists(args.file):
        print(f'{args.file} is not exists')
        exit(1)
    for file in scan(args.file):
        if not file.write():
            print(f'{file.file} has error')
        else:
            check_call(f'arm-none-eabi-gcov {file.file}', shell=True)
