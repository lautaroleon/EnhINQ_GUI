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

    # Swabian Time Tagger SDK headers. source/Iterators.h aggregates
    # "measurements/*.h", which only the SDK ships -- without this the build
    # fails on measurements/ChannelGate.h. The Linux package installs under
    # /usr/include by default; override with:
    #   qmake TIMETAGGER_INC=/path/to/include
    isEmpty(TIMETAGGER_INC) {
        TIMETAGGER_INC = /usr/include
    }
    exists($$TIMETAGGER_INC/measurements/Countrate.h) {
        # Ahead of INCLUDEPATH (see the win32 note) so the SDK's TimeTagger.h
        # wins over the stale 2.16.2 copy vendored in source/.
        QMAKE_CXXFLAGS += -I$$TIMETAGGER_INC
        INCLUDEPATH += $$TIMETAGGER_INC
    } else {
        warning("Time Tagger SDK headers not found under $$TIMETAGGER_INC (no measurements/).")
        warning("If the build fails on measurements/*.h, pass TIMETAGGER_INC=<path> to qmake.")
    }
}

# Vendor SDKs for a Windows build. The qutools quTAG SDK ships its Windows
# binaries in ../lib/DLL_64bit (tdcbase.lib + tdcbase.dll, with FTD3XX.dll
# and libusb0.dll alongside as its own runtime dependencies).
#
# FTD3XX is deliberately NOT in LIBS: the project makes no direct D3XX calls,
# and the SDK ships only FTD3XX.dll with no import library -- tdcbase pulls
# it in itself. The DLL still has to sit next to the built .exe at runtime.
#
# TimeTagger.lib / TimeTagger.dll (Swabian Instruments) is not in the repo
# yet -- drop the Windows SDK's x64 files into ../lib/DLL_64bit to link it.
# Qt's mkspec maps "-lNAME" to "NAME.lib" for MSVC.
win32 {
    LIBS += -L$$PWD/../lib/DLL_64bit/ -ltdcbase

    # Swabian Time Tagger SDK, from the vendor's Windows installer.
    # Override the location with: qmake TIMETAGGER_DIR="C:/some/other/path"
    isEmpty(TIMETAGGER_DIR) {
        TIMETAGGER_DIR = "C:/Program Files/Swabian Instruments/Time Tagger"
    }
    !exists($$TIMETAGGER_DIR/driver/include/TimeTagger.h) {
        error("Time Tagger SDK not found at $$TIMETAGGER_DIR -- install it or pass TIMETAGGER_DIR=<path> to qmake.")
    }

    # Deliberately via QMAKE_CXXFLAGS, not INCLUDEPATH: qmake emits
    # "$(CXX) -c $(CXXFLAGS) $(INCPATH)", so this lands ahead of INCPATH and
    # the SDK's headers win over the stale copies vendored into source/
    # (source/TimeTagger.h is 2.16.2; the installed library is 2.22.4).
    # The SDK dropped its top-level Iterators.h aggregator and moved the
    # measurement headers into measurements/ -- source/Iterators.h still
    # serves as the aggregator and its includes all resolve here.
    # $$quote() on the INCLUDEPATH entry matters: the default location has a
    # space in it ("Program Files"), and unquoted qmake splits it into four
    # bogus -I flags.
    QMAKE_CXXFLAGS += -I\"$$TIMETAGGER_DIR/driver/include\"
    INCLUDEPATH += $$quote($$TIMETAGGER_DIR/driver/include)

    # Only the search path -- no -lTimeTagger. TimeTagger.h carries
    # #pragma comment(lib, "TimeTagger"/"TimeTaggerD"), so the right import
    # library is chosen per configuration; naming the release one here too
    # would drag it into a Debug link alongside TimeTaggerD.
    LIBS += -L\"$$TIMETAGGER_DIR/driver/x64\"

    # main() allocates MainWindow on the stack (main.cpp) and the object is
    # large -- qkdLines alone is 4*7*2000 pointers, ~450 KB. Linux gives the
    # main thread 8 MB of stack so this was never visible there; Windows
    # defaults to 1 MB and the process dies with STATUS_STACK_OVERFLOW
    # (0xC00000FD) before the window is ever shown. Match Linux's 8 MB.
    win32-msvc*: QMAKE_LFLAGS += /STACK:8388608
    else:win32-g++: QMAKE_LFLAGS += -Wl,--stack,8388608
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
