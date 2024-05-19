rm -rf build
cmake . -B build/ 
cmake --build build --config Release
./build/src/RobLy_Core 
