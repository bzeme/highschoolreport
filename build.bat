set SDCC_FLAGS= --opt-code-size
set SDCC_LDFLAGS= --iram-size 256 --opt-code-size
call clean.bat
FOR %%i IN (*.c) DO sdcc %SDCC_FLAGS% -c %%i
sdcc *.rel -o clock.ihx  %SDCC_LDFLAGS%
packihx clock.ihx > clock.hex
pause