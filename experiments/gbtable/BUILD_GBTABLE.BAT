@echo off
echo Building GBTABLE EC and NC versions...

del /f /q GBTABLE.EOBJ GBTABLE.OBJ GBTABLE.EC GBTABLE.GEO GBTABLEEC.GEO 2>nul

mkmf
if errorlevel 1 goto fail

pmake GBTABLEEC.GEO
if errorlevel 1 goto fail

pmake GBTABLE.GEO
if errorlevel 1 goto fail

echo.
echo Build complete:
dir GBTABLE*.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
