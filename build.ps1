$GenerateCmd = "cmake -S . -B ./build"
Invoke-Expression $GenerateCmd
$BuildCmd = "cmake --build ./build --config Release"
Invoke-Expression $BuildCmd 