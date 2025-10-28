#!/usr/bin/env lua

--Variables 
local Shortlist=false
local ArgIdx = 1
--Read all arguments
local Install=true
local PkgList={}
local NotTesting=true
local verbose=false
local LookingForOptions=true



function PrintHelp(FromNum)
	print (FromNum)
	print ("	ReinstallPackages.lua [options] ")
	print ("	or")
	print ("	ReinstallPackages.lua [options] [file1] [file2] ... ")
	print ("	")
	print ("	options: -h|--help, print this help page.")
	print ("		-v|--verbose, print more info as we go along.")
	print ("		-t|--test, dont actually install or uninstall anything.")
	print ("		remove, if added as the first comment then only remove the packages.")
	print ("	")
	print ("	files: if no files are added, the script will run on *.all.ipk")
	print ("		   if you want to only manhandle one package, enter the package name..")
	print ("	")
	print ("	General function ")
	print ("	The script will try to establich a dependency tree, and remove/reinstall in order.")
	print ("	The scritp will force a removal of packages, so be warned.")

end

function PrintExHelp(FromNum)
	print  (FromNum)
	print ("	Options MUST come before files")
	print ("	")
	PrintHelp(1)
	print ("	")
	print ("	Options MUST come before files")
	os.exit(0)
end

local function get_pkg_depends_from_installed_ipkg(pkginfo)
    -- Extract dependencies from the opkg info output
    for line in pkginfo:gmatch("[^\r\n]+") do
        if line:match("^Depends:") then
            return line:match("^Depends:%s*(.+)")
        end
    end
    return nil
end

function get_pkg_depends_from_IPK_file(ipk_path)
	local cmd = 'tar -Oxzf "' .. ipk_path .. '" ./control.tar.gz | tar -xzO | grep "^Depends:"'
	local handle = io.popen(cmd)
	local result = handle:read("*l")
	handle:close()

	local deps_line = result:match("Depends:%s*(.+)")
	local deps = {}
	for dep in string.gmatch(deps_line or "", "[^,]+") do
		table.insert(deps, dep:match("^%s*(.-)%s*$"))  -- trim whitespace
	end
	
	return deps
end

function get_pkg_name(ipk_path)
  local cmd = 'tar -Oxzf "' .. ipk_path .. '" ./control.tar.gz | tar -xzO | grep "^Package:" | cut -d" " -f2'
  local handle = io.popen(cmd)
  local result = handle:read("*l")
  handle:close()
  return result
end

function match_files_glob(glob)
  local cmd = 'ls ' .. glob .. ' 2>/dev/null'
  local handle = io.popen(cmd)
  local result = handle:read("*a")
  handle:close()

  local files = {}
  for filename in result:gmatch("[^\r\n]+") do	
    table.insert(files, filename)
  end
  return files
end

function opkg(cmd, file)
 
  local cmdex = "opkg ".. cmd .." " .. file
  if NotTesting then	 
	local handle = io.popen(cmdex)
	local result = handle:read("*a")
	if verbose then print (cmdex) end
	print (result)
	handle:close()
  else
    print (cmdex)
  end
	
end
  

function is_package_installed(pkg_name)
  local cmd = "opkg list-installed | cut -d' ' -f1"
  local handle = io.popen(cmd)
  local result = handle:read("*a")
  handle:close()

  for line in result:gmatch("[^\r\n]+") do
    if line == pkg_name then
      return true
    end
  end
  return false
end

function FileExist(path)
  local f = io.open(path, "r")
  if f then
    f:close()
    return true
  else
    return false
  end
end

local function get_pkg_info(searchName)
    local handle = io.popen("opkg info " .. searchName)
    local output = handle:read("*a")
    handle:close()	
    if output == "" then return nil end	
    local result = {}
    for line in output:gmatch("[^\r\n]+") do
        local key, value = line:match("^(%w+):%s*(.+)")
        if key and value then
			
            result[key] = value
        end
    end
	

    return {
        FileName = result["Filename"],
        PkgName = result["Package"],
        isInstalled = true, -- assuming it's installed if info is returned
        depends = get_pkg_depends_from_installed_ipkg(output),
        round = 1,
		FileFound = false
    }
end


function PutFilesInlist(PkgList,SearchName)	
	local orgSearchName=SearchName
	if SearchName then
		if not FileExist(SearchName) then
			SearchName=SearchName .."*.all.ipk"
		end
	else
		SearchName= "*.all.ipk"
	end
	print (SearchName)
	
	--Does the file exist, or is the package installed
	local matches = match_files_glob(SearchName)
	if #matches > 0 then
		print("✅ Found files:")
		for _, f in ipairs(matches) do
			--get the package name and dependencies and add them as well
			local PkgName=get_pkg_name(f)
			table.insert(PkgList,{FileName = f,PkgName=PkgName,isInstalled=is_package_installed(PkgName), depends=get_pkg_depends_from_IPK_file(f), round=1, FileFound = true})							
		end
	else
		--So no file was found, lets see if it was installed
		if is_package_installed(orgSearchName) then
			print("✅ Found package as installed:")			
			table.insert(PkgList,get_pkg_info(orgSearchName))						
		else
			print("❌ No matching files found.")			
		end
	end		
end


while arg[ArgIdx] do	
	--We have some arguments lets see what we goto
	if arg[ArgIdx]=="-h" or  arg[ArgIdx]=="--help" then
		if not LookingForOptions then 
			PrintExHelp(2)
		else
			PrintHelp(2)
		end	
		os.exit(0)
	elseif arg[ArgIdx]=="-t" or  arg[ArgIdx]=="--test" then
		if not LookingForOptions then PrintExHelp(3) end
		--make sure we look for the remove thing
		NotTesting=false
	elseif arg[ArgIdx]=="-v" or  arg[ArgIdx]=="--verbose" then
		if not  LookingForOptions then PrintExHelp(4) end
		--make sure we look for the remove thing
		verbose=true
	elseif arg[ArgIdx]=="remove" then
		if not LookingForOptions then PrintExHelp(5) end
		Install=false
	else
		LookingForOptions=false
		Shortlist=true
		PutFilesInlist(PkgList,arg[ArgIdx])
	end	
	ArgIdx=ArgIdx+1
end

if not Shortlist then
	--creating a long list
	PutFilesInlist(PkgList)
end

if next(PkgList) == nil then
	print("No packages found, do nothing")
	os.exit(0)
end
	
print("lets start building the dependency tree")

local done = false	
local currlevel =1
local maxlevel =1

while (not done) do
	--does other packages depend on tpPkg(i) ?
	--todo look for cyclic dependencies
	for i, pkgI in ipairs(PkgList) do
		local DoesDepend = false
		if pkgI.round == currlevel then
			--check in all packages
			for j, pkgJ in ipairs(PkgList) do
				--if not the same package, and we arent dont with package(j)					
				if  i~=j and pkgJ.round >= currlevel then
					--go through all the dependencies of pkgJ
					for k, Depend in ipairs(pkgJ.depends) do
						--Does pkg(i) show up in depends of pkg(j) ?	
						if pkgI.PkgName == Depend then
							--Add one to pkgI.round'                           
							pkgI.round=currlevel+1
							maxlevel = currlevel + 1
							if verbose then
								print (pkgI.PkgName .. " has increased round value to: ".. pkgI.round)                        								
							end
							--Make a fast exit
							DoesDepend = true 
							break
						end
					end
				end
				if DoesDepend then
					--Make a fast exit
					break
				end	
			end
		end
	end
	if currlevel > maxlevel then
		done = true
	end
	currlevel= currlevel+1
	if verbose then
		for i, pkgI in ipairs(PkgList) do
			print(pkgI.PkgName .." : " .. pkgI.round .. ", currlevel: " ..currlevel..", maxlevel: " ..maxlevel)
						--FileName = f,PkgName=PkgName,isInstalled=is_package_installed(PkgName), depends=get_pkg_depends(f),round=1})							
		end
		print ("done: " .. tostring(done) )
	end
	
end	

--we have now found all dependencies, and made a nice little tree.
if verbose then
	print ()
	print ("we have now found all dependencies, and made a nice little tree.")
	
end
for idx = 1,maxlevel do

	if verbose then 
		print ()
		print ("Uninstall round: " .. idx) 
	end
	for i, pkgI in ipairs(PkgList) do
		if pkgI.round==idx then
			--Should be safe to uninstall
			if is_package_installed(pkgI.PkgName) then
				opkg("remove --force-depends",pkgI.PkgName)
			else
				print ("Package " .. pkgI.PkgName .. " is not installed")
			end
		end
	end
end

local opkgUpdated=false
if Install then
	if verbose then
		print ()
		print ("Now we reverse the tree, to install the packages again.")		
	end
	for idx = maxlevel,1,-1 do
		if verbose then 
			print ()
			print ("installation round: " .. idx) 
		end
		for i, pkgI in ipairs(PkgList) do
			if pkgI.round==idx then
				--Should be ready to install
				--if the package is present then				
				if not FileExist(pkgI.FileName) then
					print ("trying to install from web")
					if not opkgUpdated then
						opkg("update ","")	
						opkgUpdated=true
					end
					opkg("install ",pkgI.PkgName)					
				else
					opkg("install ",pkgI.FileName)	
				end					
				if NotTesting then
					print ("Package " .. pkgI.PkgName .. " is installed\n")
				else
					print ("Package " .. pkgI.PkgName .. " is (test) installed\n")
				end
			end
		end
	end
end
