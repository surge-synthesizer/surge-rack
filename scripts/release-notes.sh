#!/bin/bash

cat <<- EOH
# Automatically generated SurgeRack release

This release is automatically generated every time we push surge-rack
to master. As such it is the latest version of our code and may be
unstable, unusable, unsuitable for human consumption, and so on.

These assets were built against
https://vcvrack.com/downloads/Rack-SDK-1.dev.b4bd09a.zip

The build date and most recent commits are:

EOH
date
echo ""
echo "Most recent commits:" 
echo ""
git log --pretty=oneline | head -5
