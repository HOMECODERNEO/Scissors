QT  += core gui network multimedia
LIBS += -lgdi32
LIBS += -ld3d11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -LC:\Qt\Tools\OpenSSLv3\Win_x64\lib\libssl.lib

SOURCES += \
    animationsmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    popupmanager.cpp \
    savemanager.cpp \
    screenshotfloatingwindowviewer.cpp \
    screenshothistory.cpp \
    screenshothistoryviewer.cpp \
    screenshotprocess.cpp \
    screenshotprocesshighlightedarea.cpp \
    screenshotprocesstoolpopup.cpp \
    settingsform.cpp \
    soundmanager.cpp

HEADERS += \
    Defines.h \
    DrawData.h \
    Includes.h \
    animationsmanager.h \
    mainwindow.h \
    popupmanager.h \
    savemanager.h \
    screenshotfloatingwindowviewer.h \
    screenshothistory.h \
    screenshothistoryviewer.h \
    screenshotprocess.h \
    screenshotprocesshighlightedarea.h \
    screenshotprocesstoolpopup.h \
    settingsform.h \
    soundmanager.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    screenshotprocesstoolpopup.ui \
    settingsform.ui

CONFIG += resources_big

RESOURCES += \
    Resourse.qrc

RC_FILE += \
    Resourse.rc
