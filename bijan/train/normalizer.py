from pydub import AudioSegment

def match_target_amplitude(sound, target_dBFS):
	change_in_dBFS = target_dBFS - sound.dBFS
	return sound.apply_gain(change_in_dBFS)

file_name_raw="./2_2_7"
file_name= file_name_raw + ".wav"
file_name_normalized = file_name_raw + "_normalized.wav"
sound = AudioSegment.from_wav(file=file_name)
normalized_sound = match_target_amplitude(sound, -10.0)
normalized_sound.export(file_name_normalized, format="wav")
