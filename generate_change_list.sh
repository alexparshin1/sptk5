#!/bin/bash

git log --since $1 --pretty=format:%s | grep -vE '(MC|Intermediate|Merge|Code cleanup|Debugging|Version)' | grep -vE '(Doxygen|documentation|compilation|missing|CMake)'