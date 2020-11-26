#ifndef GAME_H
#define GAME_H

#include <QtCore>
#include <QThread>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <vector>
#include <random>

#include "positions_and_dice.h"

using namespace std;

static int global_color = 5;

class game : public QThread
{
    Q_OBJECT
private:
    bool game_complete;
    bool turn_complete;
    unsigned int game_delay;
    positions_and_dice relative;
    int dice_result;
    std::random_device rd;
    std::mt19937 gen;
    std::vector<int> relativePosition();
   
    int rel_to_fixed(int relative_piece_index);
    void send_them_home(int index);
    void move_start(int fixed_piece);
    int next_turn(unsigned int delay);
    static void msleep(unsigned long msecs){
        QThread::msleep(msecs);
    }
public:
    bool isGlobe(int index);
    int isGlobeCustom(int index);
    int isStar(int index);    
    int isOccupied(int index); //see if it is occupied and return the piece number

    int color;
    std::vector<int> player_positions;
    void rollDice(){
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<> dis(1, 6);
        dice_result = dis(generator);
    }
    int getDiceRoll() {return dice_result; }
    game();
    void setGameDelay(unsigned int mili_seconds){ game_delay = mili_seconds; }
    void reset();
signals:
    void player1_start(positions_and_dice);
    void player2_start(positions_and_dice);
    void player3_start(positions_and_dice);
    void player4_start(positions_and_dice);

    void player1_end(std::vector<int>);
    void player2_end(std::vector<int>);
    void player3_end(std::vector<int>);
    void player4_end(std::vector<int>);

    void update_graphics(std::vector<int>);
    void set_color(int);
    void set_dice_result(int);
    void declare_winner(int);
    void close();

public slots:
    void turnComplete(bool win);
    void movePiece(int relative_piece); //check game rules
    void save_data(std::string filepath, double d1,double d2,double d3,double d4,double d5,double d6);
protected:
    void run();
};

#endif // GAME_H
