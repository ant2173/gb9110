@echo off
echo Building GBCPU EC and NC versions...

del /f /q GBCPU.EOBJ GBCPU.OBJ GBCPU.EC GBCPU.GEO GBCPUEC.GEO 2>nul

mkmf
if errorlevel 1 goto fail

pmake GBCPUEC.GEO
if errorlevel 1 goto fail

pmake GBCPU.GEO
if errorlevel 1 goto fail

echo.
echo Build complete:
dir GBCPU*.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
