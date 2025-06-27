#!/bin/bash

declare -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare bin="$sd/../build/fmb"

assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"
    if [ "$actual" = "$expected" ]; then
        return 0
    else
        echo -e "\033[0;31m✗\033[0m $message"
        echo -e "Expected: '$expected'"
        echo -e "Actual:   '$actual'"
        return 1
    fi
}

assert_eq() {
    assert "$1" "$2" "$3"
}


testcase() {

    local input=($1); shift ;
    local expected=($1); shift ;
    local msg="${1:-"${input[*]}"}"

    local output=
    readarray -t output < <("$bin" "${input[@]}")

    echo "input len=${#input[@]}: '${input[*]}'"
    echo "expected len='${#expected[@]}' '${expected[*]}'"
    echo "output len=${#output[@]} '${output[*]}'"

    local -i elen="${#expected[@]}"
    local -i outlen="${#output[@]}"

    assert_eq $elen $outlen

    for (( i=0;i<${#output[@]}; ++i)); do
        if ! assert_eq "${output[i]}" "${expected[i]}" "$msg"; then
            echo -e "\033[0;31m✗\033[0m [idx: $i] $msg : FAILED"
            return 1
        fi
    done

    echo -e "\033[0;32m✓\033[0m $msg"
}


main() {
    "${sd}/../build.sh" || abort "build failed"

    local -i failed=0

    printf "\n\n"

    testcase 999 "999" || ((++failed))
    testcase 1024 "1K" || ((++failed))
    testcase 2048 "2K" || ((++failed))
    testcase 2000 "1.95K" || ((++failed))
    testcase 1234567  "1.18M" || ((++failed))

    testcase $(( 1<<20 )) "1M" || ((++failed))
    testcase $(( (1<<20) + 10000 )) "1.01M" || ((++failed))
    testcase $(( (1<<20)*12 )) "12M" || ((++failed))
    testcase $(( 1<<30 )) "1G" || ((++failed))
    testcase $(( (1<<30) + (1<<25) )) "1.03G" || ((++failed))

    # big boy nums
    testcase 1099511627776 "1T" || ((++failed))
    testcase $((1<<50)) "1P" || ((++failed))
    testcase 152921504606846976 "135.82P" || ((++failed))
    testcase 305843009213693952 "271.64P" || ((++failed))
    testcase 229382256910270464 "203.73P" || ((++failed))
    testcase 18446744073709551615 "16E" "test handles max u64 value" || ((++failed))

    # multiple input nums
    testcase "999 1024 2048" "999 1K 2K" || ((++failed))
    testcase "18446744073709551615 1024 2048" "16E 1K 2K" || ((++failed))

    (( failed > 0 )) && return 1
    return 0
}



main "$@"
