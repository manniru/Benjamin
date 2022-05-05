# written in python 3
# Create corpus.txt and text file

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float

AUD_DIR = sys.argv[1] #1
DAT_DIR = sys.argv[2] #2

WAV_PATH = f"{DAT_DIR}/wav.scp"
UTT_PATH = f"{DAT_DIR}/utt2spk"

def writeWord(spk_id, w_filename):
	utt_word = w_filename[:-4] #remove .wav

	WAV_BUF  = f"{spk_id}_{utt_word} "
	WAV_BUF += f"{AUD_DIR}/{spk_id}/{w_filename}\n"
	UTT_BUF  = f"{spk_id}_{utt_word} {spk_id}\n"

	TEXT_FILE.write(WAV_BUF)
	CORPUS_FILE.write(UTT_BUF)

if( AUD_DIR=="audio/train" ):
	list_dir = os.listdir(AUD_DIR)
else:
	list_dir = "test"

TEXT_FILE   = open(WAV_PATH, "w")
CORPUS_FILE = open(UTT_PATH, "w")

for dir in list_dir:
	#print("dir=" + dir)
	if( AUD_DIR=="audio/train" ):
		list_wav = os.listdir(AUD_DIR+"/"+dir)
	else:
		list_wav = os.listdir(AUD_DIR)
	
	for filename in list_wav:
		#print("file=" + filename)
		writeWord(dir, filename)
