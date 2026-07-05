@echo off
setlocal

echo Building GBPROF v1.5j NC single-object selective-debug version...

del /f /q GBPROF.EOBJ GBPROF.OBJ GBPROF.EC GBPROF.NC GBPROF.GEO GBPROFEC.GEO 2>nul
del /f /q GBENG.EOBJ GBENG.OBJ GBPDATA.EOBJ GBPDATA.OBJ 2>nul
del /f /q MAKEFILE DEPENDENCIES.MK 2>nul

copy /y LOCSYM.MK LOCAL.MK >nul
mkmf
if errorlevel 1 goto fail

echo Running "pmake depend".
pmake depend
if errorlevel 1 goto fail

echo.
echo Building one object: GOC symbols ON, engine scopes locally OFF...
pmake GBPROF.GEO
if errorlevel 1 goto fail

echo.
echo NC build complete:
dir GBPROF.GEO
goto end

:fail
echo.
echo BUILD FAILED
exit /b 1

:end
endlocal
