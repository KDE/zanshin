#! /bin/sh

# SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

gitarchive()
{
	project=$1
	tree=$2
	compression=$3
	extension=$4

	git archive --format=tar --prefix=$project-$tree/ $tree | $compression -c > $project-$tree.tar.$extension
}

buildarchive()
{
	project=$1
	tree=$2

	tar zxf $project-$tree.tar.gz \
	&& cd $project-$tree \
	&& mkdir build \
	&& cd build \
	&& cmake -DKDE4_BUILD_TESTS=ON ../ \
	&& make $MAKEOPTS \
	&& cd ../..
}

runtests()
{
	project=$1
	tree=$2

	cd $project-$tree/build \
	&& make test \
	&& cd ../..
}

# Looking for the .git directory to be sure we're
# at the top level of the working directory

while ! test -e ".git"; do
	cd ..
	if test "$PWD" = "/"; then
		echo "We're not in a git working directory!"
		exit 1
	fi
done

project=`basename $PWD`


for tree in $*; do
	echo "Generating archives for $project $tree"

	gitarchive $project $tree "gzip" "gz"
	gitarchive $project $tree "bzip2" "bz2"

	echo "Building"
	buildarchive $project $tree "tar.gz" || exit 1

	echo "Testing"
	runtests $project $tree "tar.gz" || exit 1

	echo "Cleanup"
	rm -r ./$project-$tree
done;
