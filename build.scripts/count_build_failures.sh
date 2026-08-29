#!/bin/sh

echo Number of failures in XMQ unit tests per OS.

printf '%-14s %s\n' "OS" "Failures"
echo '───────────────────────'

for test_log_file in $(ls ~/build/logs/xmq_unit_tests.*)
do
    os=$(echo $test_log_file | sed -re 's/.*xmq_unit_tests\.(.*)\.log/\1/')
    n=$(grep -cE '^\[  FAILED  \] [A-Za-z]' $test_log_file 2>/dev/null) #'
    printf '%-14s %s\n' "$os" "$((n/2))"
done
