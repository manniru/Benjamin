#!/bin/bash

SI="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SI

./main.lua "$1"