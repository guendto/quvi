#!/bin/sh
#
# autogen.sh for quvi
# Copyright (C) 2011  Toni Gundogdu
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#
set -e

source=.gitignore
cachedir=autom4te.cache

gen_cmdline()
{
  echo "Generate cmdline..."
  gengetopt < src/quvi/cmdline.ggo \
    -C --unamed-opts=URL --output-dir src/quvi --no-version || exit 1
}

gen_manual()
{
  echo "Generate manual..."
  MAN=doc/man1/quvi.1 ; POD=$MAN.pod ; VN=`./gen-ver.sh`
  pod2man -c "quvi manual" -n quvi -s 1 -r "$VN" "$POD" "$MAN"
  return $?
}

cleanup()
{
  echo "WARNING
This will remove the files specified in the $source file. This will also
remove the $cachedir/ directory with all of its contents.
  Bail out now (^C) or hit enter to continue."
  read n1
  [ -f Makefile ] && make distclean
  for file in `cat $source`; do # Remove files only.
    [ -e "$file" ] && [ -f "$file" ] && rm -f "$file"
  done
  [ -e "$cachedir" ] && rm -rf "$cachedir"
  rmdir -p config.aux 2>/dev/null
  exit 0
}

help()
{
  echo "Usage: $0 [-c|-h]
-h  Show this help and exit
-c  Make the source tree 'maintainer clean'
Run without options to (re)generate the configuration files."
  exit 0
}

while [ $# -gt 0 ]
do
  case "$1" in
    -c) cleanup;;
    -h) help;;
    *) break;;
  esac
  shift
done

mkdir -p m4
echo "Generate configuration files..."
autoreconf -if && gen_cmdline && gen_manual &&
  echo "You can now run 'configure'"
