#!/usr/bin/env python3
"""mapper2.py"""

import sys

for line in sys.stdin:
    line = line.strip().split('\t')

    if len(line) < 2:
        continue

    node = line[0]
    distance = line[1]
    
    print('{}\t{}'.format(node, distance))
