@ECHO off
SET /p ARCH=Target architecture (x86 / x64): 
SET /p BOOST_INCLUDE=Boost headers location: 
SET /p OPENSSL_INCLUDE=OpenSSL headers location: 
SET /p OPENSSL_LIB=OpenSSL lib location: 
SET /p WINDOWS_SDK_LIB=WindowsSDK lib location: 

qmake TgBotBoost.pro Makefile -spec win32-g++
mingw32-make -f Makefile.Release