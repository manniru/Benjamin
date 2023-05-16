#!/usr/bin/lua
--Keyboard Alphabets
--Number after NATO would not repeat
--Key Code from input-event-codes.h

BUF="$1"
function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)")
end

function get_Nato(word)

	if word == "arch" then
		return "65"
	elseif word == "bravo" then
		return "66"
	elseif word == "catalina" then
		return "67"
	elseif word == "charlie" then
		return "67"
	elseif word == "delta" then
		return "68"
	elseif word == "echo" then
		return "69"
	elseif word == "fish" then
		return "70"
	elseif word == "golf" then
		return "71"
	elseif word == "hotel" then
		return "72"
	elseif word == "india" then
		return "73"
	elseif word == "jordan" then
		return "74"
	elseif word == "kilo" then
		return "75"
	elseif word == "limo" then
		return "76"
	elseif word == "mike" then
		return "77"
	elseif word == "november" then
		return "78"
	elseif word == "oscar" then
		return "79"
	elseif word == "papa" then
		return "80"
	elseif word == "quebec" then
		return "81"
	elseif word == "romeo" then
		return "82"
	elseif word == "sierra" then
		return "83"
	elseif word == "tango" then
		return "84"
	elseif word == "u" then
		return "85"
	elseif word == "vpn" then
		return "86"
	elseif word == "wake" then
		return "87"
	elseif word == "eggs" then
		return "88"
	elseif word == "yankee" then
		return "89"
	elseif word == "zed" then
		return "90"
	elseif word == "sim" then
		return "32"
	elseif word == "end" then
		return "35"
	elseif word == "home" then
		return "36"
	elseif word == "semi" then
		return "186"
	elseif word == "period" then
		return "190"
	elseif word == "comma" then
		return "188"
	elseif word == "dash" then
		return "189"
	elseif word == "equal" then
		return "187"
	elseif word == "plus" then
		return "107"
	elseif word == "slash" then
		return "191"
	elseif word == "canals" then -- backslash
		return "220"
	elseif word == "edge" then -- app menu key
		return "164"
		-- xdotool key shift+minus make problem when sleep
	elseif word == "bracket" then
		return "219"
	elseif word == "curly" then
		return "221"
	elseif word == "quote" then
		return "222"
	elseif word == "github" then
		return "20" -- caps lock
	end

end