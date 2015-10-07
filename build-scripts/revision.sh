#!/bin/bash

git show-ref --tags | grep "`git log -n 1 --pretty='%H'`" > /dev/null 2>&1

if [[ $? == 1 ]]; then
    REVISION=$(git log -n 1 --pretty='%h' 2> /dev/null)
    [[ $REVISION ]] && sed -i -e "s/.*/git:$REVISION/g" VERSION
fi

cat VERSION
