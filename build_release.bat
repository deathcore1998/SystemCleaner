@echo off
echo Building SystemCleaner...

cmake --build build --config Release
if errorlevel 1 goto error

if exist release rmdir /s /q release
mkdir release
mkdir release\icons

echo Copying files...
copy build\Release\SystemCleaner.exe release\ >nul
xcopy source\icons release\icons\ /E /I >nul

echo Done! Run: release\SystemCleaner.exe
pause
goto end

:error
echo Build failed!
pause

:end