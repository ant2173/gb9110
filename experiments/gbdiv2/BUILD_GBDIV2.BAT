@echo off
setlocal

echo Building GBDIV2 v1.6 NC DIV-v2 benchmark...

del /f /q GBDIV2.EOBJ GBDIV2.OBJ GBDIV2.EC GBDIV2.NC GBDIV2.GEO GBDIV2EC.GEO 2>nul
del /f /q MAKEFILE DEPENDENCIES.MK 2>nul

mkmf
if errorlevel 1 goto fail

echo Running "pmake depend".
pmake depend
if errorlevel 1 goto fail

echo.
echo Building one object: GOC symbols ON, Peanut-GB scopes locally OFF...
pmake GBDIV2.GEO
if errorlevel 1 goto fail

echo.
echo NC build complete:
dir GBDIV2.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
endlocal
