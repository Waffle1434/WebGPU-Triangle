git submodule update --init

cd C:\
git clone https://github.com/emscripten-core/emsdk.git
rem icacls "C:\emsdk" /grant Everyone:(OI)(CI)F /T
cd emsdk
CALL emsdk.bat install latest
CALL emsdk.bat activate latest --permanent
CALL emsdk_env.bat
PAUSE