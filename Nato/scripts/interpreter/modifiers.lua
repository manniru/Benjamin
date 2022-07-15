#!/usr/bin/lua
-- Modifier Keys Like Control and Alt

function get_Modifiers(word)

	if word == "control" then
		return "29"
	elseif word == "alt" then
		return "56"
	elseif word == "shift" then
		return "42"
	elseif word == "super" then
		return "125"
	end

end