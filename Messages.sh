#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$EXTRACT_TR_STRINGS `find . -name \*.h -o -name \*.cpp -o -name \*.ui | grep -v '/tests/'` -o $podir/zanshin_qt.pot
rm -f rc.cpp
