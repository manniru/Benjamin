require('nato_w')
require('modifiers_w')
require('apps')
require('meta')
require('spex')
require('digits_w')
require('dirs_w')
require('super')
require('type')

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
	output = get_Super(in_word)
	if output~=nil then
		k_type = 'super'
	end
end

if output==nil then
	output = get_Type(in_word)
	if output~=nil then
		k_type = 'type'
	end
end

print(k_type, output)

dbus_path = "--dest=com.binaee.rebound / com.binaee.rebound"
cmd = "dbus-send --session "
cmd = cmd .. dbus_path .. ".debug string:"
cmd = cmd .. in_word
print(cmd)

cmd = "dbus-send --session ."
cmd = cmd .. dbus_path .. "."
cmd = cmd .. k_type .. " string:"
cmd = cmd .. output
print(cmd)
