#!/usr/bin/lua

require('nato')
require('modifiers')
require('apps')
require('meta')
require('spex')
require('digits')
require('dirs')
require('super')
require('type')

in_word = arg[1]
output = get_Nato(in_word)
if output~=nil then
	k_type = 'nato'
end

if output==nil then
	output = get_Modifiers(in_word)
	if output~=nil then
		k_type = 'modifier'
	end
end

if output==nil then
	output = get_Apps(in_word)
	if output~=nil then
		k_type = 'apps'
	end
end

if output==nil then
	output = get_Meta(in_word)
	if output~=nil then
		k_type = 'meta'
	end
end

if output==nil then
	output = get_Spex(in_word)
	if output~=nil then
		k_type = 'spex'
	end
end

if output==nil then
	output = get_Digits(in_word)
	if output~=nil then
		k_type = 'digit'
	end
end

if output==nil then
	output = get_Dirs(in_word)
	if output~=nil then
		k_type = 'dirs'
	end
end

if output==nil then
	output = get_Type(in_word)
	if output~=nil then
		k_type = 'type'
	end
end

if output==nil then
	output = get_Super(in_word)
	if output~=nil then
		k_type = 'super'
	end
end

dbus_path = "--dest=com.binaee.rebound / com.binaee.rebound"
cmd_debug = "dbus-send --session "
cmd_debug = cmd_debug .. dbus_path .. ".debug string:"
cmd_debug = cmd_debug .. in_word

cmd = "dbus-send --session "
cmd = cmd .. dbus_path .. "."
cmd = cmd .. k_type .. " string:"
cmd = cmd .. output

print(k_type, output)
print(cmd_debug)
print(cmd)

os.execute(cmd_debug)
os.execute(cmd)