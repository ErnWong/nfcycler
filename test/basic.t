#!/bin/sh

test_description='Test basic functionality'

. ./sharness.sh

test_expect_success 'missing argument should exit(EX_USAGE)' "
  test_expect_code 64 nfcycler
"

test_expect_success 'too many arguments should exit(EX_USAGE)' "
  test_expect_code 64 nfcycler echo echo
"

test_expect_success 'should exit when child exits' "
  nfcycler 'echo 1 && while read line
    do
      if [ \"\$line\" -gt 9 ]; then
        exit 0
      fi
      echo \"\$((line + 1))\"
    done' &
  sleep 1 &&
  ! kill -0 \"\$!\"
"

test_expect_success '--quiet should not output anything' "
  nfcycler --quiet 'echo 1 && while read line
    do
      if [ \"\$line\" -gt 9 ]; then
        exit 0
      fi
      echo \"\$((line + 1))\"
    done' &>should-be-empty &&
  test_line_count = 0 should-be-empty
"

test_expect_success '--print-payload should stdout the right output' "
  seq 1000 > expected &&
  nfcycler --print-payload 'echo 1 &&
    while read line
    do
      if [ \"\$line\" -gt 999 ]; then
        exit 0
      fi
      echo \"\$((line + 1))\"
    done' >actual &&
  test_cmp actual expected
"

test_done

# vi: set ft=sh :
