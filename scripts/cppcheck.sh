#! /bin/sh

# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
