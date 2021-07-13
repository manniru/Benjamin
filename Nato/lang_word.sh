#!/bin/bash

source path.sh

LEXICON_DIR="data/local/dict"
DICT="scripts/lang/cmudict.dict"

scripts/lang/lexicon.sh $LEXICON_DIR $DICT
scripts/lang/phones.sh $LEXICON_DIR
