#!/bin/bash

# -xc "select C as language"
# -E "only run preprocessor"
# -v "print all commands run"
# - "use stdin as input file, instead of some file name."
gcc_incpath () { echo | gcc -xc -E -v - }
