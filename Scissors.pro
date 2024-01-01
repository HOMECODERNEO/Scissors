QT  += core gui winextras network
LIBS += -lgdi32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animationsmanager.cpp \
    customhistorybutton.cpp \
    main.cpp \
    mainwindow.cpp \
    popup.cpp \
    savemanager.cpp \
    screenshotfloatingwindowviewer.cpp \
    screenshothistory.cpp \
    screenshothistoryviewer.cpp \
    screenshotprocess.cpp \
    screenshotprocesshighlightedarea.cpp \
    settingsform.cpp \
    translatormanager.cpp \
    versionchecker.cpp

HEADERS += \
    Defines.h \
    Includes.h \
    animationsmanager.h \
    customhistorybutton.h \
    mainwindow.h \
    popup.h \
    savemanager.h \
    screenshotfloatingwindowviewer.h \
    screenshothistory.h \
    screenshothistoryviewer.h \
    screenshotprocess.h \
    screenshotprocesshighlightedarea.h \
    settingsform.h \
    translatormanager.h \
    versionchecker.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    settingsform.ui

RESOURCES += \
    Resourse.qrc

RC_FILE += \
        Resourse.rc
