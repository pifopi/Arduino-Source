pushd %~dp0
cd SerialPrograms
mkdir bin
cd bin
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.6.2\msvc2019_64\
cmake --build . --config RelWithDebInfo --parallel 10
robocopy ..\..\..\Packages\SerialPrograms\Resources\    Resources\ /s
robocopy ..\..\..\CommandLineTests\                     . /s
popd
