# written in python 3
# Create corpus.txt and text file

import random
import os
import sys
import subprocess # runing bash cmd
import math # floor float

AUD_DIR = sys.argv[1] #1
DAT_DIR = sys.argv[2] #2
SUB_DIR = sys.argv[3] #3

WRD_FILE    = "word_list"
TEXT_PATH   = f"{DAT_DIR}/{SUB_DIR}/text"
CORPUS_PATH = f"{DAT_DIR}/local/corpus.txt"
def writeWord(spk_id, w_filename):
	w_filename = w_filename[:-4] #remove .wav
	utt_word   = w_filename.split('.')[0]
	word_id    = utt_word.split('_')

	TEXT_BUF   = f"{spk_id}_{w_filename} "
	CORPUS_BUF = ""

	for i in range(0,len(word_id)):
		TEXT_BUF   += lexicon[int(word_id[i])]
		TEXT_BUF   += " "
		CORPUS_BUF += lexicon[int(word_id[i])]
		CORPUS_BUF += " "

	TEXT_BUF   = TEXT_BUF[:-1] #remove last char
	CORPUS_BUF = CORPUS_BUF[:-1] #remove last char
	TEXT_BUF   += "\n"
	CORPUS_BUF += "\n"
 
	TEXT_FILE.write(TEXT_BUF)
	CORPUS_FILE.write(CORPUS_BUF)

list_dir = os.listdir(AUD_DIR)

word_file = open(WRD_FILE)
lexicon = word_file.read().splitlines()

TEXT_FILE   = open(TEXT_PATH, "w")
CORPUS_FILE = open(CORPUS_PATH, "w")

for dir in list_dir:
	#print("dir=" + dir)
	list_wav = os.listdir(AUD_DIR+"/"+dir)
	for filename in list_wav:
		#print("file=" + filename)
		writeWord(dir, filename)
