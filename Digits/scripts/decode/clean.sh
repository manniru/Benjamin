#!/bin/bash

AUDIO_DIR="$1"
WAV_DIR="$2"
DECODE_PATH="$3"

LOG_DIR="$DECODE_PATH/log"
RESULT_PATH="$DECODE_PATH/result"

rm -rf $AUDIO_DIR/* $WAV_DIR/*
rm -rf $LOG_DIR/* $RESULT_PATH/*

