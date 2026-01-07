TEMPLATE = app
TARGET = raqcoin-qt
macx:TARGET = "Raqcoin-Qt"
VERSION = 0.1.0
INCLUDEPATH += src src/json src/qt
QT += network
QT += core
QT += gui 
QT += widgets
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += thread
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 
CONFIG += -static

# Default build configuration for Windows/Cross-compile
win32 {
    isEmpty(USE_QRCODE) {
        USE_QRCODE = 1
    }
    isEmpty(USE_UPNP) {
        USE_UPNP = 1
    }
    isEmpty(USE_IPV6) {
        USE_IPV6 = 1
    }
    # Disable GPU mining by default as requested
    isEmpty(USE_GPU) {
        USE_GPU = -
    }
    isEmpty(RELEASE) {
        RELEASE = 1
    }
    # Fix for system include paths
    QMAKE_CFLAGS_ISYSTEM = -I
}
# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively
BOOST_LIB_SUFFIX=
#BOOST_INCLUDE_PATH=/home/tao/deps/boost_1_65_0
#BOOST_LIB_PATH=/home/tao/deps/boost_1_65_0/stage/lib
#BDB_INCLUDE_PATH=/home/tao/deps/db-5.1.29/build_unix
#BDB_LIB_PATH=/home/tao/deps/db-5.1.29/build_unix
#OPENSSL_INCLUDE_PATH=/home/tao/deps/openssl-1.1.0g/include
#OPENSSL_LIB_PATH=/home/tao/deps/openssl-1.1.0g
#MINIUPNPC_INCLUDE_PATH=/home/tao/deps/miniupnp
#MINIUPNPC_LIB_PATH=/home/tao/deps/miniupnp/miniupnpc

!win32:!macx {
    BOOST_LIB_PATH=/usr/lib/x86_64-linux-gnu
    BDB_LIB_PATH=/usr/lib/x86_64-linux-gnu
    OPENSSL_LIB_PATH=/usr/lib/x86_64-linux-gnu
    MINIUPNPC_LIB_PATH=/usr/lib/x86_64-linux-gnu
    UUID_LIB_PATH=/usr/lib/x86_64-linux-gnu
}
win32 {
    MXE_PATH=/usr/lib/mxe/usr/x86_64-w64-mingw32.static
    BOOST_LIB_PATH=$$MXE_PATH/lib
    BDB_LIB_PATH=$$MXE_PATH/lib
    OPENSSL_LIB_PATH=$$MXE_PATH/lib
    MINIUPNPC_LIB_PATH=$$MXE_PATH/lib
    UUID_LIB_PATH=$$MXE_PATH/lib
    QRENCODE_INCLUDE_PATH=$$MXE_PATH/include
    QRENCODE_LIB_PATH=$$MXE_PATH/lib
    BOOST_LIB_SUFFIX=-mt
    BOOST_THREAD_LIB_SUFFIX=_win32-mt
}
#QRENCODE_INCLUDE_PATH=/usr/include
#QRENCODE_LIB_PATH=/usr/lib/x86_64-linux-gnu
#PNG_LIB_PATH=/usr/lib/x86_64-linux-gnu

isEmpty(MINIUPNPC_LIB_PATH) {
    macx:MINIUPNPC_LIB_PATH = /opt/homebrew/lib
}

isEmpty(MINIUPNPC_INCLUDE_PATH) {
    macx:MINIUPNPC_INCLUDE_PATH = /opt/homebrew/include
}

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    message(Building Release)
    # Mac: compile for maximum compatibility (10.5, 32-bit)
    macx:QMAKE_CXXFLAGS += -isysroot /Volumes/Expand/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    macx:QMAKE_CFLAGS += -isysroot /Volumes/Expand/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    macx:QMAKE_LFLAGS += -isysroot /Volumes/Expand/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

    !win32:!macx {
        # Linux: static link and extra security (see: https://wiki.debian.org/Hardening)
        LIBS += -Wl,-Bstatic -Wl,-z,relro -Wl,-z,now
        QMAKE_LFLAGS += -no-pie
    }
}

!win32 {
    # for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
    QMAKE_CXXFLAGS *= -fstack-protector-all 
    QMAKE_LFLAGS *= -fstack-protector-all
    # Exclude on Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
    # This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
# on Windows: enable GCC large address aware linker flag
win32:QMAKE_LFLAGS *= -std=c++11 -static
CONFIG += console
win32:QMAKE_LFLAGS -= -Wl,--gc-sections
win32:QMAKE_LFLAGS_RELEASE -= -Wl,--gc-sections
win32:QMAKE_LFLAGS -= -Wl,-s
win32:QMAKE_LFLAGS_RELEASE -= -Wl,-s


contains(USE_ZMQ, 1) {
    message(Building with ZMQ support)
    DEFINES += USE_ZMQ
    LIBS += -lzmq
    HEADERS +=     src/zmqabstractnotifier.h \
    	src/zmqconfig.h \
    	src/zmqnotificationinterface.h \
    	src/zmqpublishnotifier.h \

    SOURCES +=     src/zmqabstractnotifier.cpp \
    	src/zmqnotificationinterface.cpp \
    	src/zmqpublishnotifier.cpp \
}

macx:USE_ASSEMBLY = 1
contains(USE_ASSEMBLY, 1) {
    message(Building with assembly support)
    !win32:LIBS += $$PWD/src/rainbow18/librainbowpro_unix.a
    win32:LIBS += $$PWD/src/rainbow18/librainbowpro_win.a
    macx {
        LIBS += $$PWD/src/rainbow18/librainbowpro_unix_arm64.a
    } else {
        LIBS += $$PWD/src/rainbow18/librainbowpro_unix.a
    }
} else {
    message(Building with Non-assembly support)
    !win32:LIBS += $$PWD/src/rainbow18/librainbowpro_unix_p.a
    win32:LIBS += $$PWD/src/rainbow18/librainbowpro_win.a
    TARGET = raqcoin-qt-p
}

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    INCLUDEPATH += $$QRENCODE_INCLUDE_PATH
    LIBS += -L$$QRENCODE_LIB_PATH -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP MINIUPNP_STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$MINIUPNPC_LIB_PATH/libminiupnpc.a
#    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}



# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "USE_IPV6=1" ( enabled by default; default)
#  or: qmake "USE_IPV6=0" (disabled by default)
#  or: qmake "USE_IPV6=-" (not supported)
contains(USE_IPV6, -) {
    message(Building without IPv6 support)
} else {
    count(USE_IPV6, 0) {
        USE_IPV6=1
    }
    DEFINES += USE_IPV6=$$USE_IPV6
}

contains(RAQCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += RAQCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

macx:USE_GPU = -
contains(USE_GPU, -) {
    message(Building without GPU mining)
} else {
    USE_GPU = 1
    DEFINES += USE_GPU=$$USE_GPU
    INCLUDEPATH +=/usr/local/cuda/include
    LIBS += $$PWD/src/obj/libgpumining.a
    #INCLUDEPATH += /usr/local/cuda/lib64
    LIBS += /usr/lib/x86_64-linux-gnu/libcudart_static.a
    #LIBS += /usr/local/cuda/lib64/libcudart_static.a
    #LIBS += -L /usr/local/cuda/lib64 -lcudart

    unix {
        gengpumining.commands = cd $$PWD/src; $(MAKE) -f makefile-libgpumining.unix clean; $(MAKE) -f makefile-libgpumining.unix
        QMAKE_CLEAN += $$PWD/src/obj/libgpumining.a; cd $$PWD/src; $(MAKE) -f makefile-libgpumining.unix clean
    }
    gengpumining.depends = FORCE
    gengpumining.target = $$PWD/src/obj/libgpumining.a
    QMAKE_EXTRA_TARGETS += gengpumining
    PRE_TARGETDEPS += $$PWD/src/obj/libgpumining.a
}

INCLUDEPATH += src/leveldb/include src/leveldb/helpers
LIBS += $$PWD/src/leveldb/libleveldb.a $$PWD/src/leveldb/libmemenv.a
!win32 {
    # we use QMAKE_CXXFLAGS_RELEASE even without RELEASE=1 because we use RELEASE to indicate linking preferences not -O preferences
    LEVELDB_OPT = $$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE
    macx:LEVELDB_OPT += -isysroot /Volumes/Expand/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    genleveldb.commands = cd $$PWD/src/leveldb && CC=$$QMAKE_CC CXX=$$QMAKE_CXX $(MAKE) OPT=\"$$LEVELDB_OPT\" libleveldb.a libmemenv.a
} else {
    # make an educated guess about what the ranlib command is called
    isEmpty(QMAKE_RANLIB) {
        QMAKE_RANLIB = $$replace(QMAKE_STRIP, strip, ranlib)
    }
    LIBS += -lshlwapi
    genleveldb.commands = cd $$PWD/src/leveldb && dos2unix build_detect_platform && CC=$$QMAKE_CC CXX=$$QMAKE_CXX TARGET_OS=OS_WINDOWS_CROSSCOMPILE $(MAKE) OPT=\"$$QMAKE_CXXFLAGS $$QMAKE_CXXFLAGS_RELEASE\" libleveldb.a libmemenv.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libleveldb.a && $$QMAKE_RANLIB $$PWD/src/leveldb/libmemenv.a
}
genleveldb.target = $$PWD/src/leveldb/libleveldb.a
genleveldb.depends = FORCE
PRE_TARGETDEPS += $$PWD/src/leveldb/libleveldb.a
QMAKE_EXTRA_TARGETS += genleveldb
# Gross ugly hack that depends on qmake internals, unfortunately there is no other way to do it.
QMAKE_CLEAN += $$PWD/src/leveldb/libleveldb.a; cd $$PWD/src/leveldb ; $(MAKE) clean

# regenerate src/build.h
!win32|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = $$OUT_PWD/build/build.h
    PRE_TARGETDEPS += $$OUT_PWD/build/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

QMAKE_CXXFLAGS_WARN_ON =-std=c++11 -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += src src/json src/qt src/pqcrypto
HEADERS += src/qt/raqcoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/sendcoinsdialog.h \
    src/qt/addressbookpage.h \
    src/qt/signverifymessagedialog.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/raqcoinaddressvalidator.h \
    src/validationinterface.h \
	src/pqcrypto/api.h \
	src/pqcrypto/blas_config.h \
	src/pqcrypto/blas.h \
	src/pqcrypto/blas_u64.h \
	src/pqcrypto/gf16.h \
	src/pqcrypto/hash_len_config.h \
	src/pqcrypto/hash_utils.h \
	src/pqcrypto/mpkc_config.h \
	src/pqcrypto/mpkc.h \
	src/pqcrypto/pqcrypt_argchk.h \
	src/pqcrypto/pqcrypt_cfg.h \
	src/pqcrypto/pqcrypt_cipher.h \
	src/pqcrypto/pqcrypt_macros.h \
	src/pqcrypto/pqcrypto.h \
	src/pqcrypto/pqcrypt_prng.h \
	src/pqcrypto/pqcrypt_util.h \
	src/pqcrypto/rainbow_16.h \
	src/pqcrypto/rainbow_config.h \
	src/pqcrypto/random.h \
	src/pqcrypto/rng.h \
	src/pqcrypto/sha256.h \
	src/pqcrypto/sha384.h \
	src/pqcrypto/sha512.h \
	src/pqcrypto/ctaes.h \
	src/pqcrypto/aes.h \
    src/alert.h \
    src/addrman.h \
    src/base58.h \
    src/checkpoints.h \
    src/compat.h \
    src/sync.h \
    src/util.h \
    src/hash.h \
    src/uint256.h \
    src/idxlut.h \
    src/serialize.h \
    src/main.h \
    src/gpumining.h\
    src/miner.h \
	src/diskpubkeypos.h\
    src/net.h \
    src/key.h \
    src/db.h \
    src/walletdb.h \
    src/script.h \
    src/init.h \
    src/bloom.h \
    src/mruset.h \
    src/checkqueue.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/raqcoinamountfield.h \
    src/wallet.h \
    src/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/qt/walletview.h \
    src/qt/walletstack.h \
    src/qt/walletframe.h \
    src/raqcoinrpc.h \
    src/qt/overviewpage.h \
    src/qt/csvmodelwriter.h \
    src/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/raqcoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/protocol.h \
    src/qt/notificator.h \
    src/qt/paymentserver.h \
    src/allocators.h \
    src/ui_interface.h \
    src/qt/rpcconsole.h \
    src/version.h \
    src/netbase.h \
    src/clientversion.h \
    src/txdb.h \
    src/leveldb.h \
    src/threadsafety.h \
    src/limitedmap.h \
    src/qt/splashscreen.h

SOURCES += src/qt/raqcoin.cpp \
    src/qt/raqcoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/raqcoinaddressvalidator.cpp \
    src/validationinterface.cpp \
	src/pqcrypto/ctaes.cpp \
	src/pqcrypto/aes.cpp \
	src/pqcrypto/rijndael.cpp \
	src/pqcrypto/rijndael_tab.cpp \
	src/pqcrypto/fortuna.cpp \
	src/pqcrypto/hash_utils.cpp \
	src/pqcrypto/pqcrypt_argchk.cpp \
	src/pqcrypto/rainbow_16.cpp \
	src/pqcrypto/random.cpp \
	src/pqcrypto/rng.cpp \
	src/pqcrypto/sha256.cpp \
	src/pqcrypto/sha384.cpp \
	src/pqcrypto/sha512.cpp \
	src/pqcrypto/sign.cpp \
    src/alert.cpp \
    src/version.cpp \
    src/sync.cpp \
    src/util.cpp \
    src/hash.cpp \
	src/base58.cpp \
    src/netbase.cpp \
    src/key.cpp \
    src/script.cpp \
    src/idxlut.cpp \
    src/main.cpp \
    src/miner.cpp \
	src/diskpubkeypos.cpp\
    src/init.cpp \
    src/net.cpp \
    src/bloom.cpp \
    src/checkpoints.cpp \
    src/addrman.cpp \
    src/db.cpp \
    src/walletdb.cpp \
    src/qt/clientmodel.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/raqcoinstrings.cpp \
    src/qt/raqcoinamountfield.cpp \
    src/wallet.cpp \
    src/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/qt/walletview.cpp \
    src/qt/walletstack.cpp \
    src/qt/walletframe.cpp \
    src/raqcoinrpc.cpp \
    src/rpcdump.cpp \
    src/rpcnet.cpp \
    src/rpcmining.cpp \
    src/rpcwallet.cpp \
    src/rpcblockchain.cpp \
    src/rpcrawtransaction.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/raqcoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/paymentserver.cpp \
    src/qt/rpcconsole.cpp \
    src/noui.cpp \
    src/leveldb.cpp \
    src/txdb.cpp \
    src/qt/splashscreen.cpp

RESOURCES += src/qt/raqcoin.qrc

FORMS += src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/signverifymessagedialog.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/optionsdialog.ui

contains(USE_QRCODE, 1) {
    HEADERS += src/qt/qrcodedialog.h
    SOURCES += src/qt/qrcodedialog.cpp
    FORMS += src/qt/forms/qrcodedialog.ui
}

contains(RAQCOIN_QT_TEST, 1) {
SOURCES += src/qt/test/test_main.cpp \
    src/qt/test/uritests.cpp
HEADERS += src/qt/test/uritests.h
DEPENDPATH += src/qt/test
QT += testlib
TARGET = raqcoin-qt_test
DEFINES += RAQCOIN_QT_TEST
  macx: CONFIG -= app_bundle
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/raqcoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/raqcoin_*.ts)
#QMAKE_LRELEASE = /usr/lib/x86_64-linux-gnu/qt5/bin/lrelease
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += README.md \
    doc/*.rst \
    doc/*.txt \
    src/qt/res/raqcoin-qt.rc \
    src/test/*.cpp \
    src/test/*.h \
    src/qt/test/*.cpp \
    src/qt/test/*.h

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = 
    win32:BOOST_LIB_SUFFIX = -mgw44-mt-s-1_50
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/homebrew/opt/berkeley-db@5/lib
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = 
}

isEmpty(BDB_INCLUDE_PATH) {
    macx:BDB_INCLUDE_PATH = /opt/homebrew/opt/berkeley-db@5/include
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = /opt/homebrew/opt/boost/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = /opt/homebrew/opt/boost/include
}

isEmpty(OPENSSL_LIB_PATH) {
    macx:OPENSSL_LIB_PATH = /opt/homebrew/opt/openssl@3/lib
}

isEmpty(OPENSSL_INCLUDE_PATH) {
    macx:OPENSSL_INCLUDE_PATH = /opt/homebrew/opt/openssl@3/include
}

win32:DEFINES += WIN32
win32:RC_FILE = src/qt/res/raqcoin-qt.rc

win32:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!win32:!macx {
    DEFINES += LINUX
    LIBS += -lrt -ldl
    # _FILE_OFFSET_BITS=64 lets 32-bit fopen transparently support large files.
    DEFINES += _FILE_OFFSET_BITS=64
}

macx:HEADERS += src/qt/macdockiconhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm
macx:HEADERS += src/qt/macnotificationhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macnotificationhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0 OBJC_OLD_DISPATCH_PROTOTYPES=1
macx:ICON = src/qt/res/icons/raqcoin.icns
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread
macx:QMAKE_INFO_PLIST = share/qt/Info.plist

# Set libraries and includes at end, to use platform-defined defaults if not overridden
#QT_INCLUDE_PATH=/usr/include/x86_64-linux-gnu/qt5
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH $$QT_INCLUDE_PATH

#LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
#LIBS += -lssl -lcrypto  -ldb_cxx
# -lgdi32 has to happen after -lcrypto (see  #681)
win32:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32 -liphlpapi -lrpcrt4
#LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
win32:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX
win32 {
    LIBS += -L$$BOOST_LIB_PATH -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX -lboost_chrono$$BOOST_LIB_SUFFIX
    LIBS += -L$$BDB_LIB_PATH -ldb_cxx
    LIBS += -L$$OPENSSL_LIB_PATH -lssl -lcrypto
    LIBS += -L$$UUID_LIB_PATH -luuid
}

!win32 {
    !macx:LIBS += $$BOOST_LIB_PATH/libboost_system.a
    LIBS += $$BOOST_LIB_PATH/libboost_filesystem.a  $$BOOST_LIB_PATH/libboost_program_options.a
    LIBS += $$BOOST_LIB_PATH/libboost_thread.a  $$BOOST_LIB_PATH/libboost_chrono.a

    macx {
        LIBS += -L$$BDB_LIB_PATH -ldb_cxx
    } else {
        LIBS += $$BDB_LIB_PATH/libdb_cxx.a
    }

    LIBS += $$OPENSSL_LIB_PATH/libssl.a  $$OPENSSL_LIB_PATH/libcrypto.a
    !macx:LIBS += $$UUID_LIB_PATH/libuuid.a
}


contains(RELEASE, 1) {
    !win32:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic
        LIBS += -ldl
    }
}

system($$QMAKE_LRELEASE -silent $$TRANSLATIONS)
