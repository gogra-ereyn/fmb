#!/bin/bash

abort() {
   local msg="$1"
   local -i ec="${2-1}"
   log "$msg" fatal
   exit $ec
}

main() {
    local -r sd="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    local -r bd="${sd}/build"
    "${sd}/build.sh" || abort "build failed"
    ctest --output-on-failure --test-dir "$bd" "$@"
}

main "$@"
