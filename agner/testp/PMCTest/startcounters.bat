rem  Example .bat script to start PMC counters.
rem  Must run as administrator

rem  set to current path
@setlocal enableextensions
@cd /d "%~dp0"

rem  Set counters. Modify the numbers to fit your purpose.
rem  See the end of PMCTestA.cpp for possible numbers

pmctest.exe startcounters 1 9 100 311

pause