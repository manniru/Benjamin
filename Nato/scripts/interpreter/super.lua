#!/usr/bin/lua
-- Meta Commands in Super Mode

function get_Super(word)

	if word == "meta" then
		return "101"
	elseif word == "colon" then
		return "102"
	elseif word == "switch" then
		return "103"
	elseif word == "kick" then
		return "104"
	elseif word == "comment" then
		return "105"
	elseif word == "copy" then
		return "106"
	elseif word == "paste" then
		return "107"
	elseif word == "side" then
		return "108"
	end

end