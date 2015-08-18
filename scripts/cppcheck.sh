#! /bin/sh

cppcheck --quiet \
         --template='{file}:{line} ({severity}, {id}) {message}' \
         --std=c++11 \
         --enable=style \
         --enable=performance \
         --enable=portability \
         --enable=information \
         --enable=missingInclude \
         --inline-suppr \
         -i 3rdparty \
         .
