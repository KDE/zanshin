#! /bin/sh
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/'` -o $podir/libkdepim.pot

