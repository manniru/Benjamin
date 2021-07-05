#!/bin/bash

DATA_DIR="data"

if [ -f "${DATA_DIR}/train/spk2gender" ]; then
	rm -r ${DATA_DIR}/train/*
fi

if [ -f "${DATA_DIR}/test/spk2gender" ]; then
	rm -r ${DATA_DIR}/test/*
fi

if [ -f "${DATA_DIR}/local/corpus.txt" ]; then
	rm "${DATA_DIR}/local/corpus.txt"
fi

if [ -f list_dir ]; then
	rm list_dir
fi

if [ -f list_file ]; then
	rm list_file
fi

if [ -f files ]; then
	rm files
fi
