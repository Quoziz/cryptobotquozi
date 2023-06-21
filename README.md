# Steganography Bot
Этот проект представляет собой бота для стеганографии, который позволяет скрывать текстовые сообщения в изображениях.

# Установка
Клонируйте этот репозиторий на свой компьютер.
Установите необходимые зависимости, выполнив команду pip install -r requirements.txt в корневой директории проекта.
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
# Использование
Запустите бота, выполнив команду в корневой директории проекта.
Отправьте боту изображение, в которое вы хотите скрыть сообщение.
Отправьте боту текстовое сообщение, которое вы хотите скрыть в изображении.
Бот вернет вам обработанное изображение с скрытым сообщением.
# Лицензия - отсутствует 
