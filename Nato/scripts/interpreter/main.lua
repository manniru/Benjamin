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

output = get_Nato(arg[1])
if output~=nil then
	k_type = 'nato'
end

if output==nil then
	output = get_Modifiers(arg[1])
	if output~=nil then
		k_type = 'modifiers'
	end
end

if output==nil then
	output = get_Apps(arg[1])
	if output~=nil then
		k_type = 'apps'
	end
end

if output==nil then
	output = get_Meta(arg[1])
	if output~=nil then
		k_type = 'meta'
	end
end

if output==nil then
	output = get_Spex(arg[1])
	if output~=nil then
		k_type = 'spex'
	end
end

if output==nil then
	output = get_Digits(arg[1])
	if output~=nil then
		k_type = 'digits'
	end
end

if output==nil then
	output = get_Dirs(arg[1])
	if output~=nil then
		k_type = 'dirs'
	end
end

if output==nil then
	output = get_Super(arg[1])
	if output~=nil then
		k_type = 'super'
	end
end

if output==nil then
	output = get_Type(arg[1])
	if output~=nil then
		k_type = 'type'
	end
end

print(k_type, output)

--DBUS_PATH="--dest=com.binaee.rebound / com.binaee.rebound"

