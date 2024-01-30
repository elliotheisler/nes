#!/bin/bash

# -xc "select C as language"
# -E "only run preprocessor"
# -v "print all commands run"
# - "use stdin as input file, instead of some file name."
gcc_incpath () { echo | gcc -xc -E -v - ; }

# select some columns from kevtris nestest.log
# cut -c'1-4,48-73,86-'

# test cpu with nestest.nes
alias cputest='scons bin/test_Cpu && bin/test_Cpu > tmp.log && nvim -d tmp.log test/kevtris_nestest/formatted_nestest.log'
# start tmuxinator config with correct args
tmuxinator .tmuxinator.yaml project_root='.'
