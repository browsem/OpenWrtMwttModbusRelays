--brfUtils
local M ={}
--[[
Functions included
AddToTableByIdx
DS18B20ID
ExecuteToPID
GetCurrentProcessPID
GetFullFilePathFromOpk
FileExist
FuncOnTable
Indent
Load_config
lsof
MqttHeader
ReadFile
PrintTable
Sleep
TblCount
Timestamp
WaitForEnterKey
]]



package.path = package.path ..";/usr/lib/lua/?.lua"
local json = require "dkjson"

function M.AddToTableByIdx(tbl, idx, value)
	tbl[idx] = value
end

function M.DS18B20ID(numberOfSensors,Seed)
	local output={}

	--See if the parameters are given
	numberOfSensors=numberOfSensors or 1
	Seed=Seed or 234567
	
	--Start seeding
	math.randomseed(Seed)
	
	for SensNum =1,numberOfSensors do
		local id = "28"
		local randnum=0
		local idAdd=""
		for i = 1, 6 do			
			randnum=math.random(0, 255)
			idAdd=string.format("%02X", randnum)
			id = id .. idAdd
			--print (SensNum .. ":" .. i .. ":" ..randnum ..":"..idAdd)
		end		
		id = id .. string.format("%02X", math.random(0, 255)) -- CRC byte		
		--print ("id"..SensNum .. ":"..id)
	M.AddToTableByIdx(output,SensNum,id)

	end
	--M.PrintTable(output)
	return output
end

function M.ExecuteToPID(cmd,PathToPidFile)
	local cmdEx = ""
	if PathToPidFile then
		cmdEx = "sh -c '".. cmd .." & echo $! > "  .. PathToPidFile .. "'"
	else
		cmdEx = cmd
	end
	os.execute(cmdEx)
end

function M.FileExist(path)
  local f = io.open(path, "r")
  if f then
    f:close()
    return true
  else
    return false
  end
end

function M.GetCurrentProcessPID(OptPidFileNameToSaveInTmp)
	local handle = io.popen("echo $PPID")	
	local pid = handle:read("*a")
	if OptPidFileToSave then	
		local file = io.open("/tmp/..OptPidFileNameToSaveInTmp", "w")
	end
	return pid
end

function M.GetFullFilePathFromOpk(Packagename,Filename)
	
	local cmd = 'opkg files '.. Packagename ..' | grep '.. Filename
	local handle = io.popen(cmd)
	local result = handle:read("*l")
	handle:close()
	return result
end

function M.FuncOnTable(tbl,FunctionToCall)	
    for key, value in pairs(tbl) do
        FunctionToCall(value)
    end
end


function M.Indent(indentlevel,Chars)
	--Set the defaults
	indentlevel=indentlevel or 1
	Chars=Chars or "  "	
	return string.rep(Chars, indentlevel)
end

-- Load and parse JSON config
function M.Load_config(path)	
    local content = M.Read_file(path)		
    local config, pos, err = json.decode(content, 1, nil)
	if err then
        error("JSON decode error: " .. err)
    end
    return config
end	

function M.lsof(FileToCheck)
	local handle = io.popen("lsof " .. FileToCheck)	
	local Result = handle:read("*a")
	
	return Result
end


function M.MqttHeader()
	--Format output as json for mqtt
	local Output = "{\n"
	Output = Output ..M.Indent(1)..'"Time": "' ..  M.Timestamp() ..'",\n'	
	return Output
end

-- Function to read file contents
function M.Read_file(path)
    local file = io.open(path, "r")
    if not file then
        error("Could not open file: " .. path)
    end
    local content = file:read("*a")	
    file:close()	
    return content
end

function M.PrintTable(tbl)		
    for key, value in pairs(tbl) do
        print(key, value)
    end	
end



function M.Sleep(seconds)
  local start = os.clock()
  while os.clock() - start < seconds do end
end

function M.TblCount(tbl)
    local count = 0
	for _ in pairs(tbl) do
		count = count + 1
	end
	return count
end

function M.Timestamp()
	local handle = io.popen("date -Iseconds")
	local timestamp = handle:read("*a"):gsub("\n", "")
	handle:close()
	timestamp=timestamp:match("^(.-)%+")
	return timestamp
end

function M.WaitForEnterKey(WaitTxt,ContinueTxt)
	--Set default text strings
	WaitTxt=WaitTxt or "Press Enter to continue..."
	ContinueTxt=ContinueTxt or "Continuing program..."
	
	print(WaitTxt)
	io.read()
	print(ContinueTxt)
end

function M.WriteToSerial(serialPortPathIn, serielAnswIn,verboseFct)			
	if verboseFct(2) then print("serielAnsw: "..serielAnswIn) end
	
	--open the port for write				
	VirtualSerialPort, err = io.open(serialPortPathIn, "w")
	if not VirtualSerialPort then
		print("Error opening "..serialPortPathIn.. ":", err)
		return
	end
	VirtualSerialPort:write(serielAnswIn)	
	
	VirtualSerialPort:close()
end


return M


