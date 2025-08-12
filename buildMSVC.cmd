pushd %~dp0
cd SerialPrograms
mkdir binMSVC
cd binMSVC
@REM -DCMAKE_TOOLCHAIN_FILE=%~dp0vcpkg\scripts\buildsystems\vcpkg.cmake
@REM -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.9.1\msvc2022_64\
cmake --build . --config RelWithDebInfo
cmake --build . --config Debug
robocopy ..\..\..\Packages\SerialPrograms\Resources\    Resources\ /s
robocopy ..\..\..\CommandLineTests\                     . /s
popd
