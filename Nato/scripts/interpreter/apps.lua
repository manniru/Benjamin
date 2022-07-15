#!/usr/bin/lua

function get_Apps(word)

	if word == "firefox" then
		return "1"
	elseif word == "files" then
		return "2"
	elseif word == "spotify" then
		return "3"
	elseif word == "atom" then
		return "4"
	elseif word == "ding" then
		return "5"
	elseif word == "link" then
		return "6"
	elseif word == "sleep" then
		return "7"
	end

end