QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = MeridianBankGUI

SOURCES += \
    main.cpp \
    BankService.cpp \
    LoginWindow.cpp \
    Pages.cpp \
    MainWindow.cpp

HEADERS += \
    BankTypes.h \
    BankService.h \
    UiHelpers.h \
    LoginWindow.h \
    Pages.h \
    MainWindow.h

# Keep the data files beside the project. In Qt Creator, set the Run working
# directory to the folder containing this .pro file.

win32:CONFIG += windows
