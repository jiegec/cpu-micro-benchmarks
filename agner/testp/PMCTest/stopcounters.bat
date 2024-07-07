rem  Example .bat script to stop PMC counters.
rem  Must run as administrator

rem  set to current path
@setlocal enableextensions
@cd /d "%~dp0"

rem  Stop counters. The numbers don't have to match the values used for starting.

pmctest.exe stopcounters 1 9 100 311

pause