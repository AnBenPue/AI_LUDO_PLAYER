#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T10:40:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ludo
TEMPLATE = app


SOURCES += ./src/main.cpp \
        ./src/dialog.cpp \
    ./src/game.cpp \
    ./src/ludo_player.cpp \
    ./src/Q_table.cpp \
    ./src/ludo_player_random.cpp

HEADERS  += ./headers/dialog.h \
    ./headers/game.h \
    ./headers/ludo_player.h \
    ./headers/positions_and_dice.h \
    ./headers/Q_table.h \
    ./headers/ludo_player_random.h

FORMS    += ./src/dialog.ui

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wunused
