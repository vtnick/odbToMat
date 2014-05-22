@echo off

REM Set Variables
set source=odbToMat
set abaqusIncludeDir=.\abaqusInclude\win
set abaqusLibDir=.\abaqusLib\win
set matIncludeDir=.\matInclude\win
set matLibDir=.\matLib\win
set odbToMatIncludeDir=.\odbToMatInclude
set odbToMatLibDir=.\odbToMatLib

setlocal

echo Resetting Library Search Path...

set PATH=%abaqusLibDir%;%matLibDir%;%odbToMatLibDir%;%PATH% 

REM -odb .\odbs\Frame_Freq -matPath .\work\Frame_Freq_test
.\%source%.exe %*

endlocal
