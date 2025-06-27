pushd %~dp0
cd SerialPrograms
mkdir bin
cd bin
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.9.0\msvc2022_64\ -DCMAKE_TOOLCHAIN_FILE=%~dp0vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config RelWithDebInfo
cmake --build . --config Debug
robocopy ..\..\..\Packages\SerialPrograms\Resources\    Resources\ /s
robocopy ..\..\..\CommandLineTests\                     . /s
popd
