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
		return "30"
	elseif word == "bravo" then
		return "48"
	elseif word == "catalina" then
		return "46"
	elseif word == "charlie" then
		return "46"
	elseif word == "delta" then
		return "32"
	elseif word == "echo" then
		return "18"
	elseif word == "fish" then
		return "33"
	elseif word == "golf" then
		return "34"
	elseif word == "hotel" then
		return "35"
	elseif word == "india" then
		return "23"
	elseif word == "jordan" then
		return "36"
	elseif word == "kilo" then
		return "37"
	elseif word == "limo" then
		return "38"
	elseif word == "mike" then
		return "50"
	elseif word == "november" then
		return "49"
	elseif word == "oscar" then
		return "24"
	elseif word == "papa" then
		return "25"
	elseif word == "quebec" then
		return "16"
	elseif word == "romeo" then
		return "19"
	elseif word == "sierra" then
		return "31"
	elseif word == "tango" then
		return "20"
	elseif word == "u" then
		return "22"
	elseif word == "eggs" then
		return "45"
	elseif word == "vpn" then
		return "47"
	elseif word == "wake" then
		return "17"
	elseif word == "yankee" then
		return "21"
	elseif word == "zed" then
		return "44"
	elseif word == "sim" then
		return "57"
	elseif word == "end" then
		return "107"
	elseif word == "home" then
		return "102"
	elseif word == "semi" then
		return "39"
	elseif word == "period" then
		return "52"
	elseif word == "comma" then
		return "51"
	elseif word == "dash" then
		return "12"
	elseif word == "level" then
		return "13"
	elseif word == "plus" then
		return "78"
	elseif word == "slash" then
		return "53"
	-- xdotool key shift+minus make problem when sleep
	elseif word == "bracket" then
		return "26"
	elseif word == "curly" then
		return "27"
	elseif word == "quote" then
		return "40"
	elseif word == "github" then
		return "58" -- caps lock
	end

end