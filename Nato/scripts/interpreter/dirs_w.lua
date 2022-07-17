#!/usr/bin/lua

function get_Dirs(word)

	if word == "back" then
		return "14"
	elseif word == "left" then
		return "37"
	elseif word == "down" then
		return "40"
	elseif word == "raise" then
		return "38"
	elseif word == "right" then
		return "39"
	elseif word == "slap" then
		return "13"
	elseif word == "tab" then
		return "9"
	elseif word == "delete" then
		return "46"
	elseif word == "departure" then
		return "27"
	end

end