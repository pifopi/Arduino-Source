pushd %~dp0
cd SerialPrograms
mkdir binCLANG
cd binCLANG
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.9.1\msvc2022_64\ -T ClangCL
cmake --build . --config RelWithDebInfo
cmake --build . --config Debug
robocopy ..\..\..\Packages\SerialPrograms\Resources\    Resources\ /s
robocopy ..\..\..\CommandLineTests\                     . /s
popd
