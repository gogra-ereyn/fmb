#!/bin/bash

main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    local -r bd="${sd}/build"
    [[ ! -d "$bd" ]] && mkdir -p "$bd" &>/dev/null
    cmake -S "$sd" -B "$bd"
    cmake --build "$bd" -j 4
}

main "$@"

