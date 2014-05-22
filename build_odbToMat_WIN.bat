@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
REM Set Variables
set source=odbToMat
set abaqusIncludeDir=.\abaqusInclude\win
set abaqusLibDir=.\abaqusLib\win
set matIncludeDir=.\matInclude\win
set matLibDir=.\matLib\win
set odbToMatIncludeDir=.\odbToMatInclude
set odbToMatLibDir=.\odbToMatLib

setlocal

echo Extracting Abaqus API.....

call abaqus extractOdbApi -name abaqusAPI

echo Move libraries and headers.....

move .\abaqusAPI\lib\* %abaqusLibDir%

move .\abaqusAPI\include\* %abaqusIncludeDir%

echo Clean up API...........

del abaqusAPI /F

echo Resetting Library Search Path...

set PATH = %abaqusLibDir%;%matLibDir%;%odbToMatLibDir%;%PATH%

echo Compiling.............

chdir %odbToMatLibDir%

cl /c /GR /EHs /D_CRT_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /O2 /Oy- /DNDEBUG -DMX_COMPAT_32 /W0 /MD /TP /EHsc /DNDEBUG /DWIN32 /DTP_IP /D_CONSOLE /DNTI /DFLT_LIC /DOL_DOC /D__LIB__ /DHKS_NT /DABQ_NTI_NET /DFAR= /D_WINDOWS /DABQ_WIN86_64 /I..\%abaqusIncludeDir% /I..\%matIncludeDir% /I..\%odbToMatIncludeDir% .\%source%.cpp .\resultFunctions.cpp .\geometryFunctions.cpp .\convertFunctions.cpp

echo Linking...............

chdir ..

LINK /nologo /INCREMENTAL:NO /subsystem:console /machine:AMD64 /STACK:20000000 /NODEFAULTLIB:LIBC.LIB /NODEFAULTLIB:LIBCMT.LIB /DEFAULTLIB:OLDNAMES.LIB /NODEFAULTLIB:LIBIFCOREMD.LIB /NODEFAULTLIB:LIBMMD.LIB /DEFAULTLIB:kernel32.lib /DEFAULTLIB:user32.lib /DEFAULTLIB:advapi32.lib /FIXED:NO /LARGEADDRESSAWARE /out:.\%source%.exe %odbToMatLibDir%\%source%.obj %odbToMatLibDir%\resultFunctions.obj %odbToMatLibDir%\geometryFunctions.obj %odbToMatLibDir%\convertFunctions.obj /LIBPATH:%abaqusLibDir% ABQSMAOdbDdbOdb.lib ABQSMAOdbApi.lib ABQSMAOdbCore.lib ABQSMAOdbCoreGeom.lib ABQSMAOdbAttrEO.lib ABQSMAAbuBasicUtils.lib ABQSMABasShared.lib ABQSMABasCoreUtils.lib ABQSMAStiCAE_StableTime.lib ABQSMABasMem.lib ABQSMAAbuGeom.lib ABQSMARomDiagEx.lib ABQSMASspUmaCore.lib ABQSMASimInterface.lib ABQSMAMtxCoreModule.lib oldnames.lib user32.lib ws2_32.lib netapi32.lib advapi32.lib /LIBPATH:%matLibDir% libmx.lib libmat.lib


endlocal
