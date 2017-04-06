#!/bin/sh

toexecute=".x treeBuilder.C+(\"$1\", \"$2\", \"$3\")"
echo $toexecute

root -l -b -q << EOF
    $toexecute
EOF
