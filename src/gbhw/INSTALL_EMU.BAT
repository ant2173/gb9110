@echo off
echo Installing to both emulator targets...

copy /y GBHWEC.GEO "C:\PCGEOS\Target\n9110v10.ec\server\World\ExtrApps\GBHWEC.GEO"
copy /y FLAPPY.GB  "C:\PCGEOS\Target\n9110v10.ec\server\World\ExtrApps\FLAPPY.GB"

copy /y GBHW.GEO   "C:\PCGEOS\Target\n9110v10.nc\server\World\ExtrApps\GBHW.GEO"
copy /y FLAPPY.GB  "C:\PCGEOS\Target\n9110v10.nc\server\World\ExtrApps\FLAPPY.GB"

echo Done.
