#!/usr/bin/python3

import sys
import re

def processFile(fileName):
    print('Processing: ', fileName)

    with open(fileName) as f:
        lines = f.readlines()

    lineNumber = 0;
    for line in lines:
        if re.search("^#ifndef ", line):
            line2 = lines[lineNumber + 1]
            if re.search("^#define ", line2):
                lines[lineNumber] = "#pragma once\n"
                lines[lineNumber + 1] = ""
                break;
        lineNumber = lineNumber + 1

    for lineNumber in range(1,2):
        if re.search("^#endif", lines[-lineNumber]):
            lines[-lineNumber] = ""
            break;

    output = open(fileName, "w")
    output.writelines(lines)
    output.close()

paramNumber = 0
for fileName in sys.argv:
    if paramNumber > 0:
        processFile(fileName)
    paramNumber = paramNumber + 1
