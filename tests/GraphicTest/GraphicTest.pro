#-------------------------------------------------
#
# Project created by QtCreator 2013-08-01T12:25:30
#
#-------------------------------------------------

TARGET = GraphicTest
TEMPLATE = app

QT	+= core gui

JOKER_ROOT = $${_PRO_FILE_PWD_}/../..

INCLUDEPATH += $${JOKER_ROOT}/libs

include($${JOKER_ROOT}/libs/PhTools/PhTools.pri)
include($${JOKER_ROOT}/libs/PhGraphic/PhGraphic.pri)

HEADERS  += \
	GraphicTestView.h \
	MainWindow.h \

SOURCES += main.cpp \
	GraphicTestView.cpp \
    MainWindow.cpp

FORMS += \
	MainWindow.ui

mac{
	PATH = "/../Resources/"
	QMAKE_POST_LINK += echo $${RESOURCES_PATH};
	QMAKE_POST_LINK += cp $${JOKER_ROOT}/data/img/look.png $${RESOURCES_PATH}/../Resources/;
	QMAKE_POST_LINK += cp $${JOKER_ROOT}/data/fonts/Bedizen.ttf $${RESOURCES_PATH}/../Resources/;
	QMAKE_POST_LINK += cp $${JOKER_ROOT}/data/fonts/SWENSON.TTF $${RESOURCES_PATH}/../Resources/;
}
win32{
	RESOURCES_PATH = $$shell_path(./debug/)
	PATH = ""

	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($${JOKER_ROOT}\data\img\look.png) $${RESOURCES_PATH} $${CS}
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($${JOKER_ROOT}\data\fonts\Bedizen.ttf) $${RESOURCES_PATH} $${CS}
	QMAKE_POST_LINK += $${QMAKE_COPY} $$shell_path($${JOKER_ROOT}\data\fonts\SWENSON.TTF) $${RESOURCES_PATH} $${CS}
}


CONFIG(release, debug|release) {
	mac {
		QMAKE_POST_LINK += macdeployqt $${TARGET}.app -dmg;
	}

}

DEFINES += PATH_TO_RESSOURCES=\\\"$$PATH\\\"

