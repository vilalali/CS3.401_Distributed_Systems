#!/usr/bin/env python3
"""mapper0.py"""

import sys

for line in sys.stdin:
    line = line.strip().split('\t')
    node = line[0]
    adj_node = []

    if len(line[1]) > 0:
        adj_node = line[1].strip().split(' ')

    for n in adj_node:
        print('{}\t{}'.format(node, n))