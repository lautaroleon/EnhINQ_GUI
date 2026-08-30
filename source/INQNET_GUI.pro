QT       += core gui multimedia sql widgets serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
CONFIG += c++17
INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
TARGET = PROGRAM
TEMPLATE = app

# ---- Platform-specific bits ----
# The qutools tdcbase SDK (tdcdecl.h) picks its calling convention/DLL-import
# decoration off "#ifdef unix" -- LINUX/linux must stay unix-only or a
# Windows build would silently take the wrong (Unix) branch and fail to
# link against tdcbase's Windows import lib.
unix {
    INCLUDEPATH += /usr/lib/
    DEFINES += LINUX linux
    LIBS += -L$$PWD/../lib/ -lftd3xx -ltdcbase -lTimeTagger
    PKGCONFIG +=
    CONFIG += link_pkgconfig
}

# Vendor SDKs expected in ../lib for a Windows build (all three ship a
# Windows build per their own docs -- get the actual .lib/.dll names from
# whichever SDK version you install, these are best-guess placeholders):
#   ftd3xx.lib   / FTD3XX.dll     (FTDI D3XX driver -- linked transitively
#                                   via tdcbase; may not be needed directly)
#   tdcbase.lib  / tdcbase.dll    (qutools quTAG SDK)
#   TimeTagger.lib / TimeTagger.dll (Swabian Instruments Time Tagger SDK)
# Qt's mkspec maps "-lNAME" to "NAME.lib" for MSVC, so this LIBS line only
# needs the actual filenames to match once they're in place.
win32 {
    LIBS += -L$$PWD/../lib/ -lftd3xx -ltdcbase -lTimeTagger
}

SOURCES += main.cpp\
           dbcontrol.cpp \
    exfo_filters.cpp \
           gui_param.cpp \
           mainwindow.cpp \
           qcustomplot.cpp \
           qutag_adq.cpp \
           qutag_anl.cpp \
    ovdl.cpp \
    timetaggerultra.cpp

HEADERS  += mainwindow.h \
            dbcontrol.h \
    exfo_filters.h \
            gui_param.h \
            qcustomplot.h \
            qutag_adq.h \
            qutag_anl.h \
            tdcbase.h \
            tdcdecl.h \
            tdcstartstop.h \
    ovdl.h \
    timetaggerultra.h \
    typedefs.h

FORMS    += mainwindow.ui
RESOURCES += \
    resources.qrc
DISTFILES +=

# ---- Copy runtime config/calibration files next to the built executable ----
RUNTIME_DATA_DIR = $$PWD/../runtime_data
RUNTIME_DATA_FILES = \
    LastSeasonVariables.conf \
    databaseInfo.json \
    databaseLOG_Rates.txt \
    databaseLOG_logic.txt \
    exfofilters.json

win32 {
    for(FILE, RUNTIME_DATA_FILES) {
        copydata.commands += $$QMAKE_COPY $$shell_path($$RUNTIME_DATA_DIR/$$FILE) $$shell_path($$OUT_PWD) $$escape_expand(\\n\\t)
    }
} else {
    for(FILE, RUNTIME_DATA_FILES) {
        copydata.commands += $(COPY) $$shell_path($$RUNTIME_DATA_DIR/$$FILE) $$shell_path($$OUT_PWD) $$escape_expand(\\n\\t)
    }
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
