@echo off
echo Building GBFS v1.4c EC and NC versions...

del /f /q GBFS.EOBJ GBFS.OBJ GBFS.EC GBFS.GEO GBFSEC.GEO 2>nul
del /f /q MAKEFILE DEPENDENCIES.MK 2>nul

mkmf
if errorlevel 1 goto fail

pmake depend
if errorlevel 1 goto fail

pmake GBFSEC.GEO
if errorlevel 1 goto fail

pmake GBFS.GEO
if errorlevel 1 goto fail

echo.
echo Build complete:
dir GBFS*.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
