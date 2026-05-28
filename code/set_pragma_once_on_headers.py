#!/usr/bin/python3

import sys
import re

def process_file(file_name):
    print('Processing: ', file_name)

    with open(file_name) as f:
        lines = f.readlines()

    line_number = 0;
    for line in lines:
        if re.search("^#ifndef ", line):
            line2 = lines[line_number + 1]
            if re.search("^#define ", line2):
                lines[line_number] = "#pragma once\n"
                lines[line_number + 1] = ""
                break;
        line_number = line_number + 1

    for line_number in range(1,2):
        if re.search("^#endif", lines[-line_number]):
            lines[-line_number] = ""
            break;

    output = open(file_name, "w")
    output.writelines(lines)
    output.close()

paramNumber = 0
for fileName in sys.argv:
    if paramNumber > 0:
        process_file(fileName)
    paramNumber = paramNumber + 1
