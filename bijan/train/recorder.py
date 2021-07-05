import random
import os
import time
import sys

AUDIO_DIR = sys.argv[1]
SPEAKER = sys.argv[2]
REC_NUMBER = int(sys.argv[3])
print(AUDIO_DIR)

file_name_prefix = AUDIO_DIR + "/train/" + str(SPEAKER)
print(file_name_prefix)

for i in range(REC_NUMBER):
	digit1 = random.randint(0,9)
	digit2 = random.randint(0,9)
	digit3 = random.randint(0,9)
	file_name = str(digit1) + "_" + str(digit2) + "_" + str(digit3) + ".wav"
	file_path = str(file_name_prefix) + "/" + file_name
	script = "find " + file_name_prefix + " -name \"" + file_name + "\" > files"
	os.system(script)
	
	count = 0
	for line in open('files'): count += 1
	if count != 0:
		i -=1
		continue

	print(i+1, ": ", digit1, " ", digit2, " ", digit3, file_path)
	time.sleep(1)
	# start Recording
	script = "bijan/scripts/record.sh " + file_path + " 3"
	os.system(script)
