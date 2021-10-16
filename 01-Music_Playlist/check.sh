#!/bin/bash
# Checher tema 1 SD 2020
NO_TESTS=20
EXEC=tema1
TEST_POINTS=(0 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4)
TIMEOUT_TIME=(0 0.005 0.005 0.005 0.005 0.005 0.005 0.005 0.005 0.005 0.005 0.01 0.01 0.01 0.01 0.01 0.05 0.05 0.05 0.05 0.05)
VALGRIND_TIMEOUT_TIME=(0 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50 50)
BONUS_POINTS=(0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0) #valgrind
TOTAL=0
BONUS=0
MAX_BONUS=20
ERRORS_NUMBER=0
CODING_STYLE_BONUS=0
CURRENT_DIRECTORY=`pwd`
ONE=1
TWO=2
MESSAGE_SIZE=12
PASSED_TESTS_GRADE=0

make build
mkdir "out"

function timeout_test() {
    test_no=$1
    input_path="in/test"$test_no".in"
    out_path="out/test"$test_no".out"

    # Get the input and ref files for the current test
    timeout=${TIMEOUT_TIME[$test_no]}

    (time timeout $timeout ./$EXEC $input_path $out_path) 2>time.err
    TEST_RESULT=$?
}

function valgrind_test() {
    test_no=$1
    input_path="in/test"$test_no".in"
    out_path="out/test"$test_no".out"

    # Get the input and ref files for the current test
    timeout=${VALGRIND_TIMEOUT_TIME[$test_no]}

    (time timeout $timeout valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 -q ./$EXEC $input_path $out_path) 2>time.err
    TEST_RESULT=$?
}

function check_valgrind_test() {
    test_no=$1

    echo -n "Test: $test_no ...................... "

    valgrind_test $test_no

    if [ "$TEST_RESULT" != "0" ]; then
        echo "DETECTED MEMORY LEAKS"
       	#tail -n 10 time.err
        rm -f time.err
        return
    else
        echo "NO MEMORY LEAKS"
        rm -f time.err
        BONUS=$(expr $BONUS + ${BONUS_POINTS[$test_no]})
        return
    fi
}

# Check a single test and add the result to the total score. This function
# should _always_ receive a test number as the first and only argument
function check_test() {
    test_no=$1

    #check valgrind first
    check_valgrind_test $1

    if [ "$TEST_RESULT" != "0" ]; then
        echo "Test: $test_no ...................... FAILED BECAUSE OF VALGRIND"
        rm -f time.err
        return
    fi

    timeout_test $test_no
    # Run the test, also consider the timeout
    if [ "$TEST_RESULT" != "0" ]; then
        echo "TIMEOUT [$timeout s]"
        rm -f time.err
		return
	fi

	out_path="out/test"$test_no".out"
	ref_path="ref/test"$test_no".ref"

	echo -n "Test: $test_no ...................... "

	# Check the result
	diff -bB -i $ref_path $out_path 2>&1 1> my_diff

	if test $? -eq 0; then
	    echo "PASS [${TEST_POINTS[$test_no]}p]"
	    TOTAL=$(expr $TOTAL + ${TEST_POINTS[$test_no]})
	    PASSED_TESTS_GRADE=$(($PASSED_TESTS_GRADE+1));
	else
	    echo "FAILED"
	    echo "Diff result:"
	    cat my_diff | tail -n 10
	fi

    # Clean up
    rm -f my_diff
    rm -f time.err
}

function checkBonus {
    printf '%*s\n' "${COLUMNS:-$(($(tput cols) - $ONE))}" '' | tr ' ' -
    echo "" > checkstyle.txt

    echo -ne "Coding style Errors:\n"
    for entry in *.{c,h}; do
	echo $entry
        if [[ -f $entry ]]; then
            ./cpplint.py "$entry" > checkstyle.txt
            YOUR_BONUS=`cat checkstyle.txt | grep "Total errors found: "| tr -dc '0-9'`
            ERRORS_NUMBER=$(( $ERRORS_NUMBER+$YOUR_BONUS ))
        fi
    done

    if [[ $ERRORS_NUMBER != "0" ]]; then
        printf '\nBonus: %*sFAIL' "${COLUMNS:-$(($(tput cols) - $MESSAGE_SIZE))}" '' | tr ' ' -
        echo -ne "\n\t\tYou have a total of $ERRORS_NUMBER coding style errors.\n"

    else
        echo -ne "\n\tNone?\n\t\tWow, next level!"
        printf '\n\nBonus: %*sOK' "${COLUMNS:-$(($(tput cols) - $MESSAGE_SIZE + $TWO ))}" '' | tr ' ' -

        CODING_STYLE_BONUS=$(($PASSED_TESTS_GRADE * $MAX_BONUS / $NO_TESTS))
        CODING_STYLE_BONUS=$(($CODING_STYLE_BONUS))
    fi
    rm -f checkstyle.txt
}

function printBonus {
    if [[ $ERRORS_NUMBER -gt 0 ]]; then
    printf '\n%*s' "${COLUMNS:-$(tput cols)}" '' | tr ' ' -
        echo -ne  "\n\tAnd you almost did it!\n"
    else
        echo -ne "\n\tDamn!\n"
    fi
    echo -ne "\n\t\tYou got a bonus of $CODING_STYLE_BONUS/$MAX_BONUS.\n\n"
}

# Check if the the exec exist
if test ! -x $EXEC; then
    echo "Exec file missing"
    exit 1
fi

echo ""
for ((i=1;i<=$NO_TESTS;i++)); do
	check_test $i
    echo ""
done

# And the restul is ...
echo "TOTAL: $TOTAL/80"
echo ""

checkBonus
printBonus

make clean &> /dev/null
rm -rf out
