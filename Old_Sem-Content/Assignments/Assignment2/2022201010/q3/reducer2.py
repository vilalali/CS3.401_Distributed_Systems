#!/usr/bin/env python3
"""reducer2.py"""

import sys

for line in sys.stdin:
    line = line.strip().split('\t')

    node = line[0]
    distance = line[1]
    
    if int(distance) <= int(10):
        print('{}\t{}'.format(node, distance))
