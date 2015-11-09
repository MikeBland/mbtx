QT += network \
      xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
include(../../common/qextserialport.pri)
HEADERS += mainwindow.h \
    file.h \
    pers.h \
    myeeprom.h \
    modeledit.h \
    generaledit.h \
    mdichild.h \
    helpers.h \
    mixerdialog.h \
    burnconfigdialog.h \
    avroutputdialog.h \ 
    simulatordialog.h \
    donatorsdialog.h \
    printdialog.h \
    preferencesdialog.h \
    mixerslist.h \
    downloaddialog.h \
    customizesplashdialog.h \
    serialdialog.h \
    VoiceAlarmDialog.h \
    TemplateDialog.h \
    wizarddialog.h \
    wizarddata.h \
    ../../common/telemetry.h \
    ../../common/reviewOutput.h \
    ../../common/node.h \
    ../../common/edge.h
SOURCES += main.cpp \
    mainwindow.cpp \
    file.cpp \
    pers.cpp \
    modeledit.cpp \
    generaledit.cpp \
    mdichild.cpp \
    helpers.cpp \
    mixerdialog.cpp \
    burnconfigdialog.cpp \
    avroutputdialog.cpp \ 
    simulatordialog.cpp \
    donatorsdialog.cpp \
    printdialog.cpp \
    preferencesdialog.cpp \
    mixerslist.cpp \
    downloaddialog.cpp \
    customizesplashdialog.cpp \
    serialdialog.cpp \
    VoiceAlarmDialog.cpp \
    TemplateDialog.cpp \
    wizarddialog.cpp \
    wizarddata.cpp \
    ../../common/telemetry.cpp \
    ../../common/reviewOutput.cpp \
    ../../common/node.cpp \
    ../../common/edge.cpp
TEMPLATE = app
FORMS += modeledit.ui \
    generaledit.ui \
    mixerdialog.ui \
    burnconfigdialog.ui \
    avroutputdialog.ui \
    simulatordialog.ui \
    donatorsdialog.ui \
    printdialog.ui \
    preferencesdialog.ui \
    downloaddialog.ui \
    customizesplashdialog.ui \
    serialdialog.ui \
    SimSerialDialog.ui \
    VoiceAlarmDialog.ui \
    TemplateDialog.ui \
    ../../common/reviewOutput.ui \
    ../../common/telemetryDialog.ui
TRANSLATIONS = eepe_.ts    \
               eepe_he.ts  \
               eepe_pt.ts  \
               eepe_ru.ts  \
               eepe_de.ts  \
               eepe_es.ts  \
               eepe_fr.ts
RESOURCES += eepe.qrc
TARGET = eepe

unix {
LANGS.path = /usr/bin/eepefiles
LANGS.files = lang/*.qm

ICON.path = /usr/bin/eepefiles
ICON.files += icon.svg

SHORTCUT.path = /usr/share/applications/
SHORTCUT.files += eepe.desktop

BINFILE.files += eepe
BINFILE.path = /usr/bin
BINFILE.commands = rm -rf /usr/bin/eepe
#This removes old eepe file or directory

INSTALLS = BINFILE
INSTALLS += LANGS
INSTALLS += ICON
INSTALLS += SHORTCUT
}

win32 {
RC_FILE += icon.rc
}

mac {
ICON = eepe.icns
}

OTHER_FILES +=
