#!/bin/bash

total='0'
CODING_STYLE_BONUS=0
MAX_BONUS=20

valgrind_test() {
    (time timeout 50 valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -q ./$EXEC < $test_file) &>/dev/null
    if [ "$?" != "0" ]; then
        echo -n "DETECTED MEMORY LEAKS - "
        return 1
    else
        return 0
    fi
}

valgrind_test_hll() {
    (time timeout 50 valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -q ./$EXEC $test_file) &>/dev/null
    if [ "$?" != "0" ]; then
        echo -n "DETECTED MEMORY LEAKS - "
        return 1
    else
        return 0
    fi
}

hll() {
    hll_ref_results=(10000 15000 20000 25000 25000)
    hll_ref_tolerances=(1000 1500 2000 2500 2500)
    hll_points=(4 5 6 7 8)

    echo "Testing HyperLogLog"

    for i in ${!hll_ref_results[@]}; do
        echo -n "$i. "
        test_file="tests_hll/test${i}.txt"

        EXEC=hll
        result=`./hll "$test_file"`

        difference=$(( ${hll_ref_results[i]} - $result ))
        abs_diff=${difference#-}

        points=`echo "(${abs_diff} < ${hll_ref_tolerances[i]}) * ${hll_points[i]}" | bc`
        valgrind_test_hll 
        VALGRIND_OK=$?
        [[ $points -gt 0 && $VALGRIND_OK -eq 0 ]] \
            && echo "passed .......... $points points" \
            || echo "failed";
        total="$total+$points"
    done

    echo ""
}

make && (echo ""; hll; echo "total = $(echo $total | bc)/30"; make clean &> /dev/null)
