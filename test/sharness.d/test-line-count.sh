# This extension is copied from...


# Library of functions shared by all tests scripts, included by
# test-lib.sh.
#
# Copyright (c) 2005 Junio C Hamano
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/ .


# test_line_count checks that a file has the number of lines it
# ought to. For example:
#
#test_expect_success 'produce exactly one line of output' '
#do something >output &&
  #test_line_count = 1 output
#'
#
# is like "test $(wc -l <output) = 1" except that it passes the
# output through when the number of lines is wrong.

test_line_count () {
  if test $# != 3
  then
    error "bug in the test script: not 3 parameters to test_line_count"
  elif ! test $(wc -l <"$3") "$1" "$2"
  then
    echo "test_line_count: line count for $3 !$1 $2"
    cat "$3"
    return 1
  fi
}
