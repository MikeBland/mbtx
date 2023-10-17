DEFINES += SKY=1
QT += network \
      xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
greaterThan(QT_MAJOR_VERSION, 4): QMAKE_CXXFLAGS += -mno-ms-bitfields
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
    stamp-eepskye.h \
    customizesplashdialog.h \
    eeprom_rlc.h \
    VoiceAlarmDialog.h \
    TemplateDialog.h \
    wizarddialog.h \
    wizarddata.h \
    ../../common/telemetry.h \
    ../../common/reviewOutput.h \
    ../../common/node.h \
    ../../common/edge.h \
    GvarAdjustDialog.h \
    ProtocolDialog.h \
    SwitchDialog.h \
    loggingDialog.h \
    cellDialog.h \
    qcustomplot.h \
    logsdialog.h \
    inputdialog.h \
    inputslist.h
SOURCES += main.cpp \
    mainwindow.cpp \
    file.cpp \
    pers.cpp \
    modeledit.cpp \
    generaledit.cpp \
    mdichild.cpp \
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
    helpers.cpp \
    eeprom_rlc.cpp \
    VoiceAlarmDialog.cpp \
    TemplateDialog.cpp \
    wizarddata.cpp \
    wizarddialog.cpp \
    ../../common/telemetry.cpp \
    ../../common/reviewOutput.cpp \
    ../../common/node.cpp \
    ../../common/edge.cpp \
    GvarAdjustDialog.cpp \
    ProtocolDialog.cpp \
    switchDialog.cpp \
    loggingDialog.cpp \
    cellDialog.cpp \
    qcustomplot.cpp \
    logsdialog.cpp \
    inputdialog.cpp \
    inputslist.cpp
unix {
SOURCES += mountlist.cpp
}
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
    VoiceAlarmDialog.ui \
    TemplateDialog.ui \
    ../../common/telemetryDialog.ui \
    ../../common/reviewOutput.ui \
    GvarAdjustDialog.ui \
    ProtocolDialog.ui \
    SwitchDialog.ui \
    loggingDialog.ui \
    cellDialog.ui \
    logsdialog.ui \
    inputdialog.ui
TRANSLATIONS = eepskye_.ts    \
               eepskye_he.ts  \
               eepskye_pt.ts  \
               eepskye_ru.ts  \
               eepskye_de.ts  \
               eepskye_es.ts  \
               eepskye_fr.ts
RESOURCES += eepskye.qrc \
    eepskye.qrc
TARGET = eepskye

unix {
LANGS.path = /usr/bin/eepskyefiles
LANGS.files = lang/*.qm

ICON.path = /usr/bin/eepskyefiles
ICON.files += icone.svg

SHORTCUT.path = /usr/share/applications/
SHORTCUT.files += eepskye.desktop

BINFILE.files += eepskye
BINFILE.path = /usr/bin
BINFILE.commands = rm -rf /usr/bin/eepskye
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
ICON = eepskye.icns
}

OTHER_FILES +=
