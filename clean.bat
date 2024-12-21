@ECHO OFF
REM Clean Everything

ECHO "Cleaning everything..."

make -f "Makefile.engine.windows.mak" clean
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

make -f "Makefile.testbed.windows.mak" clean
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

dir /s *.o 

ECHO "All assemblies cleaned successfully."
