#! /bin/sh

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


echo "Starting quality checks for $project"

echo "Automated Tests"
makeobj test || exit 1

echo "CppCheck"
./scripts/cppcheck.sh
