TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR="./build"

SOURCES += \
        Telegram/tgbot.cpp \
        main.cpp \
        simpletlsconnection.cpp \
        xorencryption.cpp

HEADERS += \
    Telegram/message.h \
    Telegram/tgbot.h \
    simpletlsconnection.h \
    stegocontainer.h \
    tests.h \
    xorencryption.h

INCLUDEPATH += "$$(BOOST_INCLUDE)" + "$$(OPENSSL_INCLUDE)"

win32 {
    contains($$(ARCH), x86) {
        LIBS += -L"$$(OPENSSL_LIB)" -llibssl -llibcrypto
        LIBS += -L"$$(WINDOWS_SDK_LIB)" -lWS2_32
    } else {
        LIBS += -L"$$(OPENSSL_LIB)" -llibssl -llibcrypto
        LIBS += -L"$$(WINDOWS_SDK_LIB)" -lWS2_32
    }
}