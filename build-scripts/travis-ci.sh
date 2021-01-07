#!/bin/bash

export GITHUB_USER=$(echo $TRAVIS_REPO_SLUG | cut -d "/" -f1)
export WORKING_DIR=$HOME/build/$GITHUB_USER

if [[ $1 == 'package' ]]; then
    VERSION=$(echo $TRAVIS_TAG | sed -e 's/v//g' -e 's/-alpha//g')
    [[ -z $VERSION ]] && VERSION='git-latest'
    export VERSION=$VERSION
fi

case "$TRAVIS_OS_NAME-$BUILD" in
    'linux-windows') ./build-scripts/platform/windows.sh $1 ;;
    'linux-native')  ./build-scripts/platform/linux.sh $1   ;;
    'osx-native')    ./build-scripts/platform/osx.sh $1     ;;
esac
