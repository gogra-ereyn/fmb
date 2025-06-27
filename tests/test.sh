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
    local flags=("$@")
    local msg="${input[*]}"

    readarray -t output_cli < <("$bin" "${flags[@]}" "${input[@]}")
    readarray -t output_stdin < <("$bin" "${flags[@]}" "${input[@]}")

    local -i elen="${#expected[@]}"
    local -i outlen_cli="${#output_cli[@]}"
    local -i outlen_stdin="${#output_stdin[@]}"

    assert_eq $elen $outlen_cli || { echo -e "\033[0;31m✗\033[0m $msg : output.len != expected.len : FAILED" ; return 1 ; }
    assert_eq $outlen_cli $outlen_stdin || { echo -e "\033[0;31m✗\033[0m $msg : output_cli.len != output_stdin.len : FAILED" ; return 1 ; }

    for (( i=0;i<${#output_cli[@]}; ++i)); do
        if ! assert_eq "${output_cli[i]}" "${expected[i]}" "$msg"; then
            echo -e "\033[0;31m✗\033[0m [idx: $i] $msg"
            return 1
        fi
        if ! assert_eq "${output_cli[i]}" "${output_stdin[i]}" "$msg"; then
            echo -e "\033[0;31m✗\033[0m $msg : STDIN & CLI output mismatch."
            return 1
        fi
    done
    echo -e "\033[0;32m✓\033[0m $msg"
}


main() {
    "${sd}/../build.sh" || abort "build failed"

    local -i failed=0

    printf "\n\n"

    # basic smoll
    testcase 999 "999" || ((++failed))
    testcase 1024 "1K" || ((++failed))
    testcase 2048 "2K" || ((++failed))
    testcase 2000 "1.95K" || ((++failed))
    testcase 1234567  "1.18M" || ((++failed))

    # big bigga
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
    testcase 18446744073709551615 "16E" || ((++failed))

    # multiple nums to format
    testcase "999 1024 2048" "999 1K 2K" || ((++failed))
    testcase "18446744073709551615 1024 2048" "16E 1K 2K" || ((++failed))

    # testflags
    ## precision
    testcase 2000 "1.953K" -p 3 || ((++failed))
    testcase 2000 "2K" -p 0 || ((++failed))

    ## formatting - unit separator
    testcase 12288 "12_K" -s '_' || ((++failed))
    testcase 12288 "12-__-K" -s '-__-' || ((++failed))

    ## decimal
    testcase 2000 "2K" --decimal || ((++failed))
    testcase 1024 "1.02K" --decimal || ((++failed))

    testcase "18446744073709551615 1024 2048" "16E 1K 2K" || ((++failed))

    (( failed > 0 )) && return 1
    return 0
}



main "$@"
