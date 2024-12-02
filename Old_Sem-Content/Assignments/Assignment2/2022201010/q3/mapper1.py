#!/usr/bin/env python3
"""mapper1.py"""

import sys

for line in sys.stdin:
    line = line.strip()
    line = line.split('\t')

    node = line[0]
    distance = line[1]
    neighbours = line[2] if len(line) > 2 and line[2] != "0" else 0

    try:
        distance = int(distance)
    except:
        continue

    print('{}\t{}\t{}'.format(node, distance, neighbours))

    if neighbours:
        neighbours = neighbours.strip().split(',')

        for neighbour in neighbours:
            child_node, child_distance = neighbour.strip().split(':', 1)

            try:
                child_distance = int(child_distance)
            except:
                continue
            
            child_distance += distance

            print('%s\t%s' % (child_node, child_distance))
