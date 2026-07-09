#!/bin/bash

echo '<ul>' > change_list.txt
git log --since $1 --pretty=format:%s \
  | grep -vEi '(MC|Intermediate|Merge|Code cleanup|Debugging|Working|Version|optimizing|updating)' \
  | grep -vEi '(Doxygen|documentation|optimization|optimisation|compilation|build fix|missing|CMake|unit test)' \
  | grep -vE '^(Refactor|Review|Fixed|Code review|Optimized|comments)' \
  | grep -vE '^Updated.*class\.?' \
  | sort -u \
  | sed -re 's|^(.*)$|  <li>\1.</li>|;s|\.\.|.|' >> change_list.txt
echo '</ul>'>> change_list.txt
