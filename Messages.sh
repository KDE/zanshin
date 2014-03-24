#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$XGETTEXT `find . -name \*.h -o -name \*.cpp | grep -v '/tests/'` -o $podir/zanshin.pot
rm -f rc.cpp
