#!/usr/bin/lua
-- Modifier Keys Like Control and Alt

function get_Modifiers(word)

	if word == "control" then
		return "17"
	elseif word == "alt" then
		return "18"
	elseif word == "shift" then
		return "16"
	elseif word == "super" then
		return "91"
	end

end