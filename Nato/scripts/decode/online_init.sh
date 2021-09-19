#!/bin/bash
# Generate online2 config files for TRI1 and TRI3

SCR="steps/online/prepare_online_decoding.sh"

MODE="tri1"
$SCR data/train data/lang exp/$MODE exp/$MODE/final.mdl exp/${MODE}_online

MODE="tri3"
$SCR data/train data/lang exp/$MODE exp/$MODE/final.mdl exp/${MODE}_online
