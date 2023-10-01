#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg'` >> rc.cpp
$XGETTEXT `find . -name '*.cpp' -or -name '*.h' | grep -v '/tests/'` -o $podir/zanshin.pot
rm -f rc.cpp
