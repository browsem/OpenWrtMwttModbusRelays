#!/bin/sh

BuildPack(){

cd $basedir/
TmpDir="${basedir}TmpDir"
mkdir $TmpDir


cd $basedir/CONTROL
#echo tar -vczf $TmpDir/control.tar.gz ./*
tar -czf $TmpDir/control.tar.gz ./*

cd $basedir/data
#echo tar -vczf $TmpDir/data.tar.gz ./*
tar -czf $TmpDir/data.tar.gz ./*


#echo $packagenameComp
echo "2.0" > $TmpDir/debian-binary
cd $TmpDir
tar -czf $RootDir/$packagenameComp ./*

rm -rf $TmpDir
echo "finished building ${packagename} ipk"
}


RootDir=$PWD
for i in $(seq 1 4); do	
	if [ "$i" -eq 1 ]; then	
		packagename='lua-brfUtils'
		version=0.8
	elif [ "$i" -eq 2 ]; then	
		packagename='SerialMqttArduinoDs18b20_Relays'
		version=0.8	
	elif [ "$i" -eq 3 ]; then	
		packagename='TasmotaSimulator'
		version=0.8	
	elif [ "$i" -eq 4 ]; then	
		packagename='BrfHeatControl'
		version=0.8	
	else
		exit 1
	fi
	packagenameComp="${packagename}.${version}.all.ipk"
	
	basedir="${RootDir}/${packagename}.ipk/"
	echo "building ${packagenameComp}" 
	echo "in folder"
	echo "${basedir}"
	BuildPack
	cd $RootDir
done


