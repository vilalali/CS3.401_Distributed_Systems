#!/usr/bin/env python3
"""reducer1.py"""

import sys

current_dist = None
current_node = None
current_path = None
current_neighbours = 0
node = 0
for line in sys.stdin:
    line = line.strip().split('\t')

    if len(line) < 2:
        continue

    node = line[0]
    distance = line[1]
    if len(line) == 2:
        neighbours = 0
    if len(line) == 3:
        neighbours = line[2] if line[2] != "0" else 0

    try:
        distance = int(distance)
    except:
        continue

    if current_node == node:
        if distance < current_dist:
            current_dist = distance
        
        if neighbours != 0:
            current_neighbours = neighbours
    else:
        if current_node:
            print('{}\t{}\t{}'.format(current_node, current_dist, current_neighbours))
        current_node = node
        current_dist = distance
        current_neighbours = neighbours

if current_node == node:
    print('{}\t{}\t{}'.format(current_node, current_dist, current_neighbours))
