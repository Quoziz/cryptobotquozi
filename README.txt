Выберите папку для устноавки.
Установите MinGW https://www.ics.uci.edu/~pattis/common/handouts/mingweclipse/mingw.html
Не забудьте добавить папку mingw-version\bin в PATH
Распакуйте архив c boost
Установите OpenSSL https://slproweb.com/products/Win32OpenSSL.html 
Установите WindowsSDK https://learn.microsoft.com/ru-ru/windows/apps/windows-app-sdk/
Запустите build.bat:
ARCH - x64
BOOST_INCLUDE - папка, куда вы распаковали boost; например: D:\CppLibs\boost\boost_1_82_0
OPENSSL_INCLUDE - например, D:\CppLibs\OpenSSL-Win64\include - именно так, не до самой глубокой папки!
OPENSSL_LIB - например, D:\CppLibs\OpenSSL-Win64\lib
WINDOWS_SDK_LIB - например, C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64