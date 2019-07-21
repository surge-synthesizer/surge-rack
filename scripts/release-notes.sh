#!/bin/bash

branch=`git rev-parse --abbrev-ref HEAD`

if [ $branch == "master" ]; then
  message="As such these are nightly builds and may be unstable."
else
  message="This is an officially supported release point of this software."
fi

cat <<- EOH
# Automatically generated SurgeRack release

These assets are built against branch $branch.
$message

The build date and most recent commits are:

EOH
date
echo ""
echo "Most recent commits:" 
echo ""
git log --pretty=oneline | head -5
