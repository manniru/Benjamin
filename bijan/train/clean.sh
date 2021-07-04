#!/bin/bash

OUT_DIR="$1"
DATA_DIR="data"

if [ -f "${DATA_DIR}/train/spk2gender" ]; then
	rm "${DATA_DIR}/train/spk2gender"
fi

if [ -f "${DATA_DIR}/test/spk2gender" ]; then
	rm "${DATA_DIR}/test/spk2gender"
fi

if [ -f "${DATA_DIR}/train/text" ]; then
	rm "${DATA_DIR}/train/text"
fi

if [ -f "${DATA_DIR}/test/text" ]; then
	rm "${DATA_DIR}/test/text"
fi

if [ -f "${DATA_DIR}/local/corpus.txt" ]; then
	rm "${DATA_DIR}/local/corpus.txt"
fi

if [ -f "$DATA_DIR/train/wav.scp" ]; then
	rm "$DATA_DIR/train/wav.scp"
fi

if [ -f "$DATA_DIR/test/wav.scp" ]; then
	rm "$DATA_DIR/test/wav.scp"
fi

if [ -f "$DATA_DIR/train/utt2spk" ]; then
	rm "$DATA_DIR/train/utt2spk"
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
