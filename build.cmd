pushd %~dp0
cd SerialPrograms
mkdir bin
cd bin
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.8.2\msvc2022_64\
cmake --build . --config RelWithDebInfo --parallel 10
robocopy ..\..\..\Packages\SerialPrograms\Resources\    Resources\ /s
robocopy ..\..\..\CommandLineTests\                     . /s
popd
