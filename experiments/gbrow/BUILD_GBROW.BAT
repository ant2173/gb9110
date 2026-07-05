@echo off
echo Building GBROW EC and NC versions...

del /f /q GBROW.EOBJ GBROW.OBJ GBROW.EC GBROW.GEO GBROWEC.GEO 2>nul

mkmf
if errorlevel 1 goto fail

pmake GBROWEC.GEO
if errorlevel 1 goto fail

pmake GBROW.GEO
if errorlevel 1 goto fail

echo.
echo Build complete:
dir GBROW*.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
