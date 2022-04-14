QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/fileparser.cpp \
    src/main.cpp \
    src/mywidget.cpp

HEADERS += \
    external_files/MAC/Inc/MAC_Max_STA_Num.h \
    external_files/MAC/Inc/MAC_Timeline.h \
    external_files/MAC/Inc/MAC_Timers.h \
    external_files/MAC/Inc/MAC_debug.h \
    external_files/MAC/Inc/MAC_pkts_format.h \
    external_files/MAC/Inc/ModemProtocol.h \
    external_files/debug.h \
    external_files/eth.h \
    external_files/types.h \
    inc/fileparser.h \
    inc/mywidget.h \

FORMS += \
    mywidget.ui

win32: RC_FILE = res.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
