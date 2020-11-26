#ifndef LUDO_PLAYER_H
#define LUDO_PLAYER_H

#include <random>
#include <iomanip>
#include <vector>
#include <QObject>
#include <iostream>

#include "positions_and_dice.h"
#include "Q_table.h"
#include "game.h"
using namespace std;

class ludo_player : public QObject
{
    Q_OBJECT
private:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_end_of_turn;
    int dice_roll;
    int make_decision();
    uint num_of_rules = 10;
    int Table_size = pow(2, num_of_rules);

    std::vector<int> state_d1;
    std::vector<int> state_d2;
    std::vector<int> state_d3;
    std::vector<int> state_d4;
    std::vector<int> state_final;

    std::vector<int> *state_d1_ptr;
    std::vector<int> *state_d2_ptr;
    std::vector<int> *state_d3_ptr;
    std::vector<int> *state_d4_ptr;
    std::vector<int> *state_final_ptr;

    uint state_index_d1;
    uint state_index_d2;
    uint state_index_d3;
    uint state_index_d4;
    uint state_index_final;

    std::vector<double> action_d1;
    std::vector<double> action_d2;
    std::vector<double> action_d3;
    std::vector<double> action_d4;
    std::vector<double> action_max;
    std::vector<double> action_final;

    std::vector<double> *action_d1_ptr;
    std::vector<double> *action_d2_ptr;
    std::vector<double> *action_d3_ptr;
    std::vector<double> *action_d4_ptr;
    std::vector<double> *action_max_ptr;
    std::vector<double> *action_final_ptr;

    double delta_Q;
    double reward;
    double alpha;
    double gamma;
    bool step_by_step = false;
    bool training = false;


public:
    Q_Table *T_ptr;
    ludo_player(game *game_ptr, Q_Table *table_ptr);
    game *game_ptr;

    // For data gathering:
    int counter_s1 = 0;
    int counter_s2 = 0;
    int counter_s3 = 0;
    int counter_s4 = 0;
    int counter_s5 = 0;
    int counter_s6 = 0;
    int counter_s7 = 0;
    int counter_s8 = 0;
    int counter_s9 = 0;
    int counter_s10 = 0;


signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
    void state();
    void print_board_state(positions_and_dice relative);
    int get_state_index(std::vector<int> state);
    void state_dot(int dot_index, positions_and_dice relative, std::vector<int> *state);
    void update_Q_action_max(positions_and_dice relative);
    void update_Q_action_final(positions_and_dice relative);
    void player_mode(int mode);
    void reset_state_counters();
    void save_data(std::string filepath, int d1, int d2, int d3, int d4, int d5, int d6, int d7, int d8, int d9, int d10);
};

#endif // LUDO_PLAYER_H
