@echo off
echo Building GBPROF EC and NC versions...

del /f /q GBPROF.EOBJ GBPROF.OBJ GBPROF.EC GBPROF.GEO GBPROFEC.GEO 2>nul

mkmf
if errorlevel 1 goto fail

pmake GBPROFEC.GEO
if errorlevel 1 goto fail

pmake GBPROF.GEO
if errorlevel 1 goto fail

echo.
echo Build complete:
dir GBPROF*.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
