#!/usr/bin/env lua
local json = require("dkjson")

-- We can delimit literal strings also by matching double square brackets [[...]]. 
-- Literals in this bracketed form may run for several lines, may nest, and do not 
-- interpret escape sequences. 
-- from https://www.lua.org/pil/2.4.html
local json_string = [[
{
  "numbers": [ 2, 3, -20.23e+2, -4 ],
  "currency": "hello world"
  "name":"高科"
}
]]

print("json_string:", json_string)
print("----------------------------")

local obj, pos, err = json.decode(json_string, 1, nil)
if err then
	print("Error:", err)
else
	print ("currency", obj.currency)
	for i = 1, #obj.numbers do 
		print(i, obj.numbers[i])
	end	 
end	

-- How to handle the property which not exist.
if obj.name == nil then
	print("obj.name is not exist!!!")
else
	print("obj.name is: ", obj.name)
end

-- How to get the type of json object.
if type(obj) == 'table' then
	print("type of obj is table")
end 

local meta_table = getmetatable(obj)
if meta_table.__jsontype then
	print("__jsontype: is", meta_table.__jsontype)
end

-- Example2 parse array in json --
print("\n\n-- Example2 parse array in json --\n")
local json_string2 = [[ 
[ {"name":"viton"}, {"name":"xiaokl"} ]
]]

local obj2, pos2, err2 = json.decode(json_string2, 1, nil)
if obj2 then
local meta_table2 = getmetatable(obj2)
	if meta_table2.__jsontype == "array" then
		for i=1, #obj2 do
			print(obj2[i].name)
		end
	end
end

function safeDivide(a, b)
    if b == 0 then
        return nil, "Division by zero"
    end
    return a / b
end

print ("\n\n\n")
local result, err = safeDivide(10, 0)
print (err)
if err then
    print("Error:", err)
end
local result, err = safeDivide(10, 1)
if err then
    print("Error:", err)
end
