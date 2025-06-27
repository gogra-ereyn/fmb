#!/bin/bash

declare -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
declare bin="$sd/../build/fmb"

assert() {
    local actual="$1"
    local expected="$2"
    local message="${3:-Assertion}"
    if [ "$actual" = "$expected" ]; then
        echo -e "\033[0;32m✓\033[0m $message"
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
    local input="$1"; shift ;
    local expected=$1 ; shift ;
    local msg="${1:-$input}"
    local output=
    output="$("$bin" $input)"
    assert_eq "$output" "$expected" "$msg"
}


main() {
    "${sd}/../build.sh" || abort "build failed"

    local -i failed=0

    printf "\n\n"


    testcase 999 "999B"

    testcase 1000 "1KB"

    (( failed > 0 )) && return 1
    return 0
}



main "$@"
