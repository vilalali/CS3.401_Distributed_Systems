#!/usr/bin/env python3
"""mapper0.py"""

import sys

for line in sys.stdin:
    friend1, friend2 = line.strip().split(' ')

    print("{}\t{}".format(friend1, friend2))
    print("{}\t{}".format(friend2, friend1))
     
