rem              make_a_obj.bat                     2014-04-16 Agner Fog

rem  compiles PMCTestA.cpp into PMCTestA32.obj and PMCTestA64.obj

rem  System requirements:
rem  Windows 2000 or NT or later
rem  Microsoft Visual C++ compiler or other C++ compiler

rem  You have to change all paths to the actual paths on your computer

rem  Set path to 32 bit compiler
set VSroot=C:\Program Files (x86)\Microsoft Visual Studio 11.0
set SDKroot=C:\Program Files (x86)\Windows Kits\8.0\
set path1=%path%
set path=%VSroot%\VC\bin;%VSroot%\Common7\IDE;%path1%

rem  Set path to *.h include files.
set include=%VSroot%\VC\include;%SDKroot%\Include\um;%SDKroot%\Include\shared

rem  Set path to *.lib library files. 
set LIB="%VSroot%\VC\lib;%SDKroot%\Lib\win8\um\x86"

rem compile 32 bit object file
cl /c /O2 /FoPMCTestA32.obj PMCTestA.cpp
if errorlevel 1 pause

rem compile 32bit exe file
rem cl /O2 /MT /Fepmctest.exe PMCTestA32.obj PMCTestB.cpp "%SDKroot%\Lib\win8\um\x86\uuid.lib" "%VSroot%\VC\lib\libcmt.lib" "%VSroot%\VC\lib\oldnames.lib"

cl /O2 /MT /Fepmctest.exe PMCTestA.cpp PMCTestB.cpp Advapi32.lib /link /LIBPATH:"%SDKroot%\Lib\win8\um\x86" /LIBPATH:"%VSroot%\VC\lib"
if errorlevel 1 pause




rem  Set path to 64 bit compiler
set path=%VSroot%\VC\bin\x86_amd64;%VSroot%\Common7\IDE;%path1%

rem  Set path to *.lib library files. 
set lib="%VSroot%\VC\lib\amd64"

rem compile 64 bit version
cl /c /O2 /FoPMCTestA64.obj PMCTestA.cpp
if errorlevel 1 pause

pause
