#!/usr/bin/env python

for i in range(100, 110):
    for j in range (0, 10):
        if j == 5:
            continue
        print j

    print i
