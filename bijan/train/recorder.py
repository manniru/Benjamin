import random
import os
import time
import sys

AUDIO_DIR = sys.argv[1]
print(AUDIO_DIR)

 
N = int(input("Enter number of iteration for record: "))
hw = int(input("Enter destination [home(0), work(1)]: "))
speakerId = "home" if hw == 0 else "work"
file_name_prefix = AUDIO_DIR + "/train/" + str(speakerId)
print(file_name_prefix)

for i in range(N):
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

	print(i+1, ": ", digit1, " ", digit2, " ", digit3)
	time.sleep(1)
	# start Recording
	script = "bijan/scripts/record.sh " + file_name_prefix + " 3"
	os.system(script)
