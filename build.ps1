param
(
    [ValidateSet("MSVC", "CLANG")]
    [string]$Compiler = "MSVC",

    [ValidateSet("NONE", "VCPKG", "CONAN")]
    [string]$PackageManager = "NONE"
)

enum Compiler
{
    MSVC
    CLANG
}
$CompilerAsEnum = [Compiler]::$Compiler
switch ($CompilerAsEnum)
{
    MSVC
    {
        $cmakeCompilerAdditionalParam = ""
    }
    CLANG
    {
        $cmakeCompilerAdditionalParam = "-T ClangCL"
    }
}

enum PackageManager
{
    NONE
    VCPKG
    CONAN
}
$PackageManagerAsEnum = [PackageManager]::$PackageManager
switch ($PackageManagerAsEnum)
{
    NONE
    {
        $cmakePackageManagerAdditionalParam = ""
    }
    VCPKG
    {
        $cmakePackageManagerAdditionalParam = "-DCMAKE_TOOLCHAIN_FILE=$PSScriptRoot\vcpkg\scripts\buildsystems\vcpkg.cmake"
    }
    CONAN
    {
        $cmakePackageManagerAdditionalParam = "-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake"
    }
}

pushd $PSScriptRoot
Set-Location SerialPrograms
$binDirectory = "bin\" + [PackageManager]::$PackageManager + "\" + [Compiler]::$Compiler
Write-Output "Building in $binDirectory"
if (-not(Test-Path $binDirectory -PathType Container))
{
    New-Item -path $binDirectory -ItemType Directory
}
Set-Location $binDirectory

switch ($PackageManagerAsEnum)
{
    CONAN
    {
        conan install ..\..\.. --output-folder=. --build=missing -s "&:build_type=RelWithDebInfo" -s build_type=Release -s compiler.cppstd=23
    }
}
cmake ..\..\.. -DCMAKE_PREFIX_PATH=C:\Qt\6.9.2\msvc2022_64\ $cmakeCompilerAdditionalParam $cmakePackageManagerAdditionalParam
cmake --build . --config RelWithDebInfo
robocopy ..\..\..\..\..\Packages\SerialPrograms\Resources\  Resources\ /s
robocopy ..\..\..\..\..\CommandLineTests\                   . /s
popd
