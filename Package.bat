@echo off

REM Enable Delayed Variable Expansion
SETLOCAL ENABLEDELAYEDEXPANSION

REM === 1. Packaging settings ===
set PROJECT=GifClipper
set DIR_PROJECT=%~dp0
set DIR_LICENCE=%DIR_PROJECT%LICENCE.md
set DIR_README=%DIR_PROJECT%README.md
set DIR_EXE=%DIR_PROJECT%..\build-GifClipper-Desktop_Qt_6_5_0_MinGW_64_bit-Release\%PROJECT%.exe

IF exist "%DIR_EXE%" (
    echo "Found release executable for GifClipper (%DIR_EXE%)"
) ELSE (
    echo "Could not find release executable, ensure you have built a release build and try again. (%DIR_EXE%)"
    pause
    exit /b
)

REM === 2. Prepare the output folder ===
REM Ask the user for build number
set /p BUILD_NUMBER="Please enter build number (ex: 0,1, 5, 150): "

REM Check deploy directory, ensure it exists and is empty
set DIR_DEPLOY=%DIR_PROJECT%..\Packages\%PROJECT%_%BUILD_NUMBER%
IF exist "%DIR_DEPLOY%" (
    REM Ask the user if the directory should be removed
    set /p REMOVE_DIRECTORY="Deploy directory already exists, remove it and continue? (y/n): "
    echo You entered: "!REMOVE_DIRECTORY!"
    REM Remove the directory if the user chooses to do so
    IF /i "!REMOVE_DIRECTORY!"=="y" (
        echo "Cleaning deploy directory..."
        rd /s /q "%DIR_DEPLOY%"
        if exist "%DIR_DEPLOY%" (
            echo "Failed to remove the deploy directory. Exiting ..."
            pause
            exit /b
        )
    ) ELSE (
        echo "The deploy directory needs to be empty. Exiting ..."
        pause
        exit /b
    )
)
mkdir "%DIR_DEPLOY%"
cd /d "%DIR_DEPLOY%"
echo "Deploy directory created."

REM === 3. Copy package files in output ===
copy "%DIR_EXE%" "%DIR_DEPLOY%"
copy "%DIR_README%" "%DIR_DEPLOY%"
copy "%DIR_LICENCE%" "%DIR_DEPLOY%"

REM Copy FFMPEG's binaries and dll dependencies
xcopy /E /I "%DIR_PROJECT%QFFmpeg\ffmpeg-5.1.2-full_build-shared\bin" "%DIR_DEPLOY%"
for /R "%DIR_PROJECT%QFFmpeg\ffmpeg-5.1.2-full_build-shared\lib\" %%f in (*.dll) do (
    echo "%%f"
    copy "%%f" "%DIR_DEPLOY%"
)

echo "Copied executable and external libs"

REM === 4. Run windeployqt to deploy Qt stuff ===
windeployqt.exe --qmldir "%appdata%\..\Local\Qt\6.5.0\mingw_64\bin" GifClipper.exe --no-translations
echo "Finished deploying with windeployqt."
cd /d %DIR_PROJECT%


pause