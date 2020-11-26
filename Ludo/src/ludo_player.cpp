#include "./headers/ludo_player.h"

ludo_player::ludo_player(game *Game_ptr, Q_Table *table_ptr)
{
    this->game_ptr = Game_ptr;
    this->T_ptr = table_ptr;
    this->T_ptr->initialize_Q_table(Table_size);

    // Initialize Dot variables:
    state_d1.resize(num_of_rules);
    state_d2.resize(num_of_rules);
    state_d3.resize(num_of_rules);
    state_d4.resize(num_of_rules);
    state_final.resize(num_of_rules);

    state_d3_ptr = &state_d3;
    state_d2_ptr = &state_d2;
    state_d1_ptr = &state_d1;
    state_d4_ptr = &state_d4;
    state_final_ptr = &state_final;

    state_index_d1 = 0;
    state_index_d2 = 0;
    state_index_d3 = 0;
    state_index_d4 = 0;
    state_index_final = 0;

    action_d1.resize(1);
    action_d2.resize(1);
    action_d3.resize(1);
    action_d4.resize(1);
    action_max.resize(4);

    action_d1_ptr = &action_d1;
    action_d2_ptr = &action_d2;
    action_d3_ptr = &action_d3;
    action_d4_ptr = &action_d4;
    action_max_ptr = &action_max;
    action_final_ptr = &action_final;

    // Learning variables:
    delta_Q = 0.0;
    reward = 0.0;
    alpha = 0.9;
    gamma = 0.01;
}

int ludo_player::make_decision()
{
    // When a  6 is rolled, the player needs to check if a dot is at home position. If this is the case, the player has to put it out.
    if (dice_roll == 6)
    {
        // Check all the dots for pieces at home.
        for (int i = 0; i < 4; ++i)
        {
            // home_position = -1.
            if (pos_start_of_turn[i] < 0)
            {
                // This dot is at home and since the player rolled a 6, this dot has to go out.
                return i;
            }
        }
    }

    return action_max_ptr->at(1);
}

void ludo_player::start_turn(positions_and_dice relative)
{
    pos_start_of_turn = relative.pos;
    dice_roll = relative.dice;

    //print_board_state(relative);
    update_Q_action_max(relative);
    int decision = make_decision();
    //state();

    if (step_by_step == true)
    {
        string dummy;
        cout << " Press any key to continue..." << endl;
        cin >> dummy;
    }

    emit select_piece(decision);
}

void ludo_player::post_game_analysis(std::vector<int> relative_pos)
{
    //system("clear");
    pos_end_of_turn = relative_pos;
    bool game_complete = true;
    for (int i = 0; i < 4; ++i)
    {
        if (pos_end_of_turn[i] < 99)
        {
            game_complete = false;
        }
    }

    if (game_complete)
    {
        T_ptr->save_Q_table();
    }

    positions_and_dice relative;
    relative.pos = relative_pos;
    relative.dice = 0;
    dice_roll = 0;
    //print_board_state(relative);
    if (action_max_ptr->at(2) != 0)
    {
        update_Q_action_final(relative);
    }

    //state();
    if (step_by_step == true)
    {
        string dummy;
        cout << " Press any key to continue..." << endl;
        cin >> dummy;
    }

    //T_ptr->print_Q_table();

    emit turn_complete(game_complete);
}

int ludo_player::get_state_index(std::vector<int> state)
{
    uint it = 0;
    int base = 1;
    int decimal_val = 0;

    while (it < (state.size()))
    {

        decimal_val += state.at(it) * base;
        base = base * 2;
        it++;
    }

    return decimal_val;
}

void ludo_player::state_dot(int dot_index, positions_and_dice relative, std::vector<int> *state)
{
    // For debugging:
    bool verbose_R1 = false;
    bool verbose_R2 = false;
    bool verbose_R3 = false;
    bool verbose_R4 = false;
    bool verbose_R5 = false;
    bool verbose_R6 = false;
    bool verbose_R7 = false;
    bool verbose_R8 = false;
    bool verbose_R9 = false;
    bool verbose_R10 = false;

    // initialize reward:
    reward = 0.0;

    // Varible declarations:
    int P_dot_pos = 0;
    int O_dot_pos = 0;

    int dis_P_to_O = 0;
    int dis_O_to_P = 0;

    int dis_O_to_P_moved = 0;
    int dis_P_to_O_moved = 0;
    int P_dot_pos_moved = 0;

    int oponent_dot_id = 0;

    // Rewards:
    double reward_R1 = 0.3;  // Dot movable
    double reward_R2 = 0.5;  // Dot in danger
    double reward_R3 = 0.5;  // Dot can kill
    double reward_R4 = -0.5; // After moved, in danger
    double reward_R5 = 0.1;  // After moved, can kill an oponent
    double reward_R6 = 0.3;  // After moved, on a star
    double reward_R7 = -5.0; // After moved, on a globe
    double reward_R8 = -0.5; // on winners road
    double reward_R9 = 0.1;  // Closest one to goal
    double reward_R10 = 3.0; // Dot can finish

    // Initialize state to false in all cases:
    for (uint i = 0; i < this->num_of_rules; i++)
    {
        state->at(i) = 0;
    }

    // Player dot position.
    P_dot_pos = relative.pos[dot_index];

    if (verbose_R1 || verbose_R2 || verbose_R3 || verbose_R4 || verbose_R5 || verbose_R6 || verbose_R7 || verbose_R8 || verbose_R9 || verbose_R10)
    {
        cout << "Computing state of Dot N: " << (dot_index + 1) << endl;
        cout << "   |-> Actual Position: " << P_dot_pos << endl;
        cout << "   |-> R1: Checking if dot can be moved:" << endl;
    }

    // Check if the dot is out of home and hasn't finished.
    if (!(P_dot_pos == -1) && !(P_dot_pos == 99))
    {
        // Piece movable
        state->at(0) = 1;
        counter_s1++;

        if (verbose_R1)
        {
            cout << "   |   |------> RESULT: Dot Can be moved" << endl;
        }

        // After moving the dot the distance obtained from the dice.
        // Player dot position.
        P_dot_pos_moved = P_dot_pos + dice_roll;

        // If after moved, the dot folls on a star, it wil bounce to the next one:
        if (game_ptr->isStar(P_dot_pos_moved) == 7)
        {
            P_dot_pos_moved += 7;

            // In case the star puts you in the goal:
            if (P_dot_pos_moved == 57)
            {
                P_dot_pos_moved = 56;
            }
        }
        else if (game_ptr->isStar(P_dot_pos_moved) == 6)
        {
            P_dot_pos_moved += 6;

            // In case the star puts you in the goal:
            if (P_dot_pos_moved == 57)
            {
                P_dot_pos_moved = 56;
            }
        }

        // Check the relation between the player dot and the oponent ones.
        for (int j = 4; j < 16; j++)
        {
            // For debugging, keep track of which dot we are checking:
            if (oponent_dot_id == 4)
            {
                oponent_dot_id = 0;
            }
            oponent_dot_id++;

            // Before moving:
            // Oponent dot position.
            O_dot_pos = relative.pos[j];

            // Check if the oponent dot is out of home and hasn't finished.
            if (!(O_dot_pos == -1) && !(O_dot_pos == 99))
            {
                // Distance from the oponent's dot to the player's dot.
                dis_O_to_P = P_dot_pos - O_dot_pos;

                // Check if the dot is in range of an oponent piece.
                if (verbose_R2)
                {
                    cout << "   |-> Check player [ " << (j / 4) + 1 << " ] Dot [ " << oponent_dot_id << " ]" << endl;
                    cout << "   |   |-> R2: Check if the dot is in range of an oponent's dot:" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 1:" << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P <= 6)                    [ " << (dis_O_to_P <= 6) << " ] , dis_O_to_P [ " << dis_O_to_P << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P  > 0)                    [ " << (dis_O_to_P > 0) << " ] , dis_O_to_P [ " << dis_O_to_P << " ] " << endl;
                    cout << "   |   |   |   |-> Not in winners road                  [ " << (P_dot_pos < 51) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a standar Globe           [ " << !(game_ptr->isGlobeCustom(P_dot_pos) == 2) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a star (7)                [ " << !(game_ptr->isStar(P_dot_pos) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a star (6)                [ " << !(game_ptr->isStar(P_dot_pos) == 6) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 2:" << endl;
                    cout << "   |   |   |   |-> Dot on other player's home position  [ " << (game_ptr->isGlobeCustom(P_dot_pos) == 1) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos != 0)                     [ " << (P_dot_pos != 0) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 3:" << endl;
                    cout << "   |   |   |   |-> Dot on a star (7)                    [ " << (game_ptr->isStar(P_dot_pos) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P <= 12)                   [ " << (dis_O_to_P <= 12) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P > 7)                     [ " << (dis_O_to_P > 7) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 4:" << endl;
                    cout << "   |   |   |   |-> Dot on a star (6)                    [ " << (game_ptr->isStar(P_dot_pos) == 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P <= 13)                   [ " << (dis_O_to_P <= 13) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P > 8)                     [ " << (dis_O_to_P > 8) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 5:" << endl;
                    cout << "   |   |   |   |-> (P_dot_pos != 0)                     [ " << (P_dot_pos != 0) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos < 6)                      [ " << (P_dot_pos < 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos - ( O_dot_pos -50)) < 6)  [ " << ((P_dot_pos - (O_dot_pos - 50)) < 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (O_dot_pos <= 50)                    [ " << (O_dot_pos <= 50) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> State not active yet                     [ " << (state->at(1) != 1) << " ] " << endl;
                }

                // State not active yet
                if (state->at(1) != 1)
                {
                    //----------- In Range of Rolling -------- Not in winners road ----- Dot not on a standar Globe  ---------------------- Dot not on a star ----------------------- Don not on a star ----------------
                    if ((dis_O_to_P <= 6) && (dis_O_to_P > 0) && (P_dot_pos < 51) && !(game_ptr->isGlobeCustom(P_dot_pos) == 2) && !(game_ptr->isStar(P_dot_pos) == 7) && !(game_ptr->isStar(P_dot_pos) == 6))
                    {
                        // The dot can be killed by an oponent.
                        state->at(1) = 1;
                        counter_s2++;
                        reward += reward_R2;
                        if (verbose_R2)
                        {
                            cout << "   |   |------> RESULT: (in game) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_O_to_P << " ]" << endl;
                        }
                    }
                    // ---- Dot on other player's home position ---
                    else if ((game_ptr->isGlobeCustom(P_dot_pos) == 1) && (P_dot_pos != 0))
                    {
                        // The dot can be killed by an oponent.
                        state->at(1) = 1;
                        counter_s2++;
                        reward += reward_R2;
                        if (verbose_R2)
                        {
                            cout << "   |   |------> RESULT: (in game) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Dot is standing on an oponent's home globe" << endl;
                        }
                    }
                    // ---- Dot on a star and an oponent can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(P_dot_pos) == 7) && (dis_O_to_P <= 12) && (dis_O_to_P > 7))
                    {
                        // The dot can be killed by an oponent.
                        state->at(1) = 1;
                        counter_s2++;
                        reward += reward_R2;
                        if (verbose_R2)
                        {
                            cout << "   |   |------> RESULT: (in game) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 7), Distance [ " << dis_O_to_P << " ]" << endl;
                        }
                    }
                    // ---- Dot on a star and an oponent can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(P_dot_pos) == 6) && (dis_O_to_P <= 13) && (dis_O_to_P > 8))
                    {
                        // The dot can be killed by an oponent.
                        state->at(1) = 1;
                        counter_s2++;
                        reward += reward_R2;
                        if (verbose_R2)
                        {
                            cout << "   |   |------> RESULT: (in game) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 6), Distance [ " << dis_O_to_P << " ]" << endl;
                        }
                    }
                    // The dot can be killed by a player on position > than 46:
                    //------- Not at home -- No further than  pos 6 ---------- In range of an oponent -----
                    else if (!(P_dot_pos == 0) && (P_dot_pos < 6) && ((P_dot_pos - (O_dot_pos - 50)) < 6) && (O_dot_pos <= 50))
                    {
                        // The dot can be killed by an oponent.
                        state->at(1) = 1;
                        counter_s2++;
                        reward += reward_R2;
                        if (verbose_R2)
                        {
                            cout << "   |   |------> RESULT: (in game) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Who is in position over 46, Position [ " << O_dot_pos << " ] , Diatance [ " << (P_dot_pos - (O_dot_pos - 50)) << " ]" << endl;
                        }
                    }
                }

                // Distance from the player's dot to the oponent's dot.
                dis_P_to_O = O_dot_pos - P_dot_pos;

                // Check if the piece is in range to kill an oponent piece.
                if (verbose_R3)
                {
                    cout << "   |   |-> R3: Check if the dot can kill an oponent's dot in the next move:" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 1:" << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O == dice_roll)            [ " << (dis_P_to_O == dice_roll) << " ] , dis_P_to_O [ " << dis_P_to_O << " ] " << endl;
                    cout << "   |   |   |   |-> The oponent is not in a globe        [ " << game_ptr->isGlobe(O_dot_pos) << " ] " << endl;
                    cout << "   |   |   |   |-> (O_dot_pos < 50)                     [ " << (O_dot_pos < 50) << " ] , O_dot_pos [ " << O_dot_pos << " ]" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 3:" << endl;
                    cout << "   |   |   |   |-> Oponent dot on a star (7)            [ " << (game_ptr->isStar(O_dot_pos) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O <= 12)                   [ " << (dis_P_to_O <= 12) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 4:" << endl;
                    cout << "   |   |   |   |-> Oponent dot on a star (6)            [ " << (game_ptr->isStar(O_dot_pos) == 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O <= 13)                   [ " << (dis_P_to_O <= 13) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> State not active yet                     [ " << (state->at(2) != 1) << " ] " << endl;
                }
                //---  Distance to oponet   --
                //--- equal to rolled value ---- The oponent is not in a globe ------- Oponent is not unreachable -- State not active yet -
                /*if (state->at(2) != 1)
                {
                    if ((dis_P_to_O == dice_roll) && (game_ptr->isGlobe(O_dot_pos) == false) && (O_dot_pos < 50))
                    {
                        // The dot can kill an oponent.
                        state->at(2) = 1;
                        counter_s3++;
                        reward += reward_R3;
                        if (verbose_R3)
                        {
                            cout << "   |   |------> RESULT: (in game) Can kill player [ " << (j / 4) + 1 << " ] . Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_P_to_O << " ]" << endl;
                        }
                    }
                    // ---- oponent dot on a star and an player's dot can jump on the previous one and kill it ---
                    else if ((game_ptr->isStar(O_dot_pos) == 7) && (dis_P_to_O == 12))
                    {
                        // The dot can be kill oponent.
                        state->at(2) = 1;
                        counter_s3++;
                        reward += reward_R3;
                        if (verbose_R3)
                        {
                            cout << "   |   |------> RESULT: (in game) Can kill player [ " << (j / 4) + 1 << " ] By bouncing on a star. Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_P_to_O << " ]" << endl;
                        }
                    }
                    // ---- oponent dot on a star and an player's dot can jump on the previous one and kill it ---
                    else if ((game_ptr->isStar(O_dot_pos) == 6) && (dis_P_to_O == 13))
                    {
                        // The dot can be killed by an oponent.
                        state->at(2) = 1;
                        counter_s3++;
                        reward += reward_R3;
                        if (verbose_R3)
                        {
                            cout << "   |   |------> RESULT: (in game) Can kill player [ " << (j / 4) + 1 << " ] By bouncing on a star. Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_P_to_O << " ]" << endl;
                        }
                    }
                }*/
                // Distance from the oponent's dot to the player's dot after.
                dis_O_to_P_moved = P_dot_pos_moved - O_dot_pos;

                // Check if the dot is in range of an oponent piece (after moved).
                if (verbose_R4)
                {
                    cout << "   |-> Check player [ " << (j / 4) + 1 << " ] Dot [ " << oponent_dot_id << " ]" << endl;
                    cout << "   |   |-> R4: Check if the dot is in range of an oponent's dot after moving:" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 1:" << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved <= 6)              [ " << (dis_O_to_P_moved <= 6) << " ] , dis_O_to_P_moved [ " << dis_O_to_P_moved << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved  > 0)              [ " << (dis_O_to_P_moved > 0) << " ] , dis_O_to_P_moved [ " << dis_O_to_P_moved << " ] " << endl;
                    cout << "   |   |   |   |-> Not in winners road                  [ " << (P_dot_pos_moved < 51) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a standar Globe           [ " << !(game_ptr->isGlobeCustom(P_dot_pos_moved) == 2) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a star (7)                [ " << !(game_ptr->isStar(P_dot_pos_moved) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> Dot not on a star (6)                [ " << !(game_ptr->isStar(P_dot_pos_moved) == 6) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 2:" << endl;
                    cout << "   |   |   |   |-> Dot on other player's home position  [ " << (game_ptr->isGlobeCustom(P_dot_pos_moved) == 1) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos_moved != 0)               [ " << (P_dot_pos_moved != 0) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 3:" << endl;
                    cout << "   |   |   |   |-> Dot on a star (7)                    [ " << !(game_ptr->isStar(P_dot_pos_moved) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved <= 12)             [ " << (dis_O_to_P_moved <= 12) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved > 7)               [ " << (dis_O_to_P_moved > 7) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 4:" << endl;
                    cout << "   |   |   |   |-> Dot on a star (6)                    [ " << !(game_ptr->isStar(P_dot_pos_moved) == 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved <= 13)             [ " << (dis_O_to_P_moved <= 13) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_O_to_P_moved > 8)               [ " << (dis_O_to_P_moved > 8) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 5:" << endl;
                    cout << "   |   |   |   |-> (P_dot_pos_moved != 0)               [ " << (P_dot_pos_moved != 0) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos_moved < 6)                [ " << (P_dot_pos_moved < 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (P_dot_pos_moved-(O_dot_pos-50))< 6) [ " << ((P_dot_pos_moved - (O_dot_pos - 50)) < 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (O_dot_pos <= 50)                    [ " << (O_dot_pos <= 50) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> State not active yet                     [ " << (state->at(3) != 1) << " ] " << endl;
                }
                //- State not active yet -
                if ((state->at(3) != 1))
                {
                    //----------- In Range of Rolling -------- Not in winners road ----- Dot not on a standar Globe  ---------------------- Dot not on a star ----------------------- Don not on a star ----------------
                    if ((dis_O_to_P_moved <= 6) && (dis_O_to_P_moved > 0) && (P_dot_pos_moved < 51) && !(game_ptr->isGlobeCustom(P_dot_pos_moved) == 2) && !(game_ptr->isStar(P_dot_pos_moved) == 7) && !(game_ptr->isStar(P_dot_pos_moved) == 6))
                    {
                        // The dot can be killed by an oponent.
                        state->at(3) = 1;
                        counter_s4++;
                        reward += reward_R4;
                        if (verbose_R4)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_O_to_P_moved << " ]" << endl;
                        }
                    }
                    // ---- Dot on other player's home position ---
                    else if ((game_ptr->isGlobeCustom(P_dot_pos_moved) == 1) && (P_dot_pos_moved != 0))
                    {
                        // The dot can be killed by an oponent.
                        state->at(3) = 1;
                        counter_s4++;
                        reward += reward_R4;
                        if (verbose_R4)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Dot is standing on an oponent's home globe" << endl;
                        }
                    }
                    // ---- Dot on a star and an oponent can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(P_dot_pos_moved) == 7) && (dis_O_to_P_moved <= 12) && (dis_O_to_P_moved > 7))
                    {
                        // The dot can be killed by an oponent.
                        state->at(3) = 1;
                        counter_s4++;
                        reward += reward_R4;
                        if (verbose_R4)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 7), Distance [ " << dis_O_to_P_moved << " ]" << endl;
                        }
                    }
                    // ---- Dot on a star and an oponent can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(P_dot_pos_moved) == 6) && (dis_O_to_P_moved <= 13) && (dis_O_to_P_moved > 8))
                    {
                        // The dot can be killed by an oponent.
                        state->at(3) = 1;
                        counter_s4++;
                        reward += reward_R4;
                        if (verbose_R4)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 6), Distance [ " << dis_O_to_P_moved << " ]" << endl;
                        }
                    }
                    // The dot can be killed by a player on position > than 46:
                    //------- Not at home -- No further than  pos 6 ---------- In range of an oponent -----
                    else if (!(P_dot_pos_moved == 0) && (P_dot_pos_moved < 6) && ((P_dot_pos_moved - (O_dot_pos - 50)) < 6) && (O_dot_pos <= 50))
                    {
                        // The dot can be killed by an oponent.
                        state->at(3) = 1;
                        counter_s4++;
                        reward += reward_R4;
                        if (verbose_R4)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . Who is in position over 46, Position [ " << O_dot_pos << " ] , Diatance [ " << (P_dot_pos_moved - (O_dot_pos - 50)) << " ]" << endl;
                        }
                    }
                }

                // Distance from the player's dot to the oponent's dot.
                dis_P_to_O_moved = O_dot_pos - P_dot_pos_moved;

                // Check if the piece is in range to kill an oponent's dot.
                if (verbose_R5)
                {
                    cout << "   |   |-> R5: Check if the piece is in range to kill an oponent's dot after moving:" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 1:" << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved <= 6)              [ " << (dis_P_to_O_moved <= 6) << " ] , dis_P_to_O_moved [ " << dis_P_to_O_moved << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved  > 0)              [ " << (dis_P_to_O_moved > 0) << " ] , dis_P_to_O_moved [ " << dis_P_to_O_moved << " ] " << endl;
                    cout << "   |   |   |   |-> The oponent is not in a globe        [ " << (game_ptr->isGlobe(O_dot_pos)) << " ] " << endl;
                    cout << "   |   |   |   |-> Oponent is not unreachable           [ " << (O_dot_pos < 51) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 2:" << endl;
                    cout << "   |   |   |   |-> Oponent dot on a star (7)            [ " << (game_ptr->isStar(O_dot_pos) == 7) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved <= 12)             [ " << (dis_P_to_O_moved <= 12) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved > 7)               [ " << (dis_P_to_O_moved > 7) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> Case 3:" << endl;
                    cout << "   |   |   |   |-> Oponent dot on a star (6)            [ " << (game_ptr->isStar(O_dot_pos) == 6) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved <= 13)             [ " << (dis_P_to_O_moved <= 13) << " ] " << endl;
                    cout << "   |   |   |   |-> (dis_P_to_O_moved > 8)               [ " << (dis_P_to_O_moved > 8) << " ] " << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> State not active yet                     [ " << (state->at(4) != 1) << " ] " << endl;
                }

                //-- State not active yet -
                if ((state->at(4) != 1))
                {
                    //---------Distance to oponet In Range of Rolling ------ The oponent is not in a globe ---- Oponent is not unreachable-
                    if ((dis_P_to_O_moved <= 6) && (dis_P_to_O_moved > 0) && (game_ptr->isGlobe(O_dot_pos) == false) && (O_dot_pos < 50))
                    {
                        // The dot can kill an oponent.
                        state->at(4) = 1;
                        counter_s5++;
                        reward += reward_R5;
                        if (verbose_R5)
                        {
                            cout << "   |   |------> RESULT: (After moved) Can kill player [ " << (j / 4) + 1 << " ] . Oponent's Dot at [ " << O_dot_pos << " ] , Distance [ " << dis_P_to_O_moved << " ]" << endl;
                        }
                    }
                    // ---- oponent on a star and an dot can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(O_dot_pos) == 7) && (dis_P_to_O_moved <= 12) && (dis_P_to_O_moved > 7))
                    {
                        // The dot can be killed by an oponent.
                        state->at(4) = 1;
                        counter_s5++;
                        reward += reward_R5;
                        if (verbose_R5)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 7), Distance [ " << dis_P_to_O_moved << " ]" << endl;
                        }
                    }
                    // ---- oponent on a star and an dot can jump on the previous one and kill the dot ---
                    else if ((game_ptr->isStar(O_dot_pos) == 6) && (dis_P_to_O_moved <= 13) && (dis_P_to_O_moved > 8))
                    {
                        // The dot can be killed by an oponent.
                        state->at(4) = 1;
                        counter_s5++;
                        reward += reward_R5;
                        if (verbose_R5)
                        {
                            cout << "   |   |------> RESULT: (After moved) Dot Can be killed by player [ " << (j / 4) + 1 << " ] . By bouncing from previous star (case 6), Distance [ " << dis_P_to_O_moved << " ]" << endl;
                        }
                    }
                }
            }
        }

        // Check if the dot can jump onto a star:
        if (verbose_R6)
        {
            cout << "   |   |-> R6: Check if the dot can jump onto a star:" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 1:" << endl;
            cout << "   |   |   |   |-> isStar(P_dot_pos_moved) == 7         [ " << (game_ptr->isStar(P_dot_pos_moved) == 7) << " ] " << endl;
            cout << "   |   |   |   |-> isStar(P_dot_pos_moved) == 6         [ " << (game_ptr->isStar(P_dot_pos_moved) == 6) << " ] " << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> State not active yet                     [ " << (state->at(5) != 1) << " ] " << endl;
        }

        //- State not active yet -
        if (state->at(5) != 1)
        {
            //--------------------------------- It can fall in a star ------------------------------
            if ((game_ptr->isStar(P_dot_pos_moved) == 7) || (game_ptr->isStar(P_dot_pos_moved) == 6))
            {
                // The dot will fall in a jumping star.
                state->at(5) = 1;
                counter_s6++;
                reward += reward_R6;
                if (verbose_R6)
                {
                    cout << "   |   |------> RESULT: (After moved)  Dot on a star" << endl;
                }
            }
        }

        // Check if the dot is in a globe after being moved and theres an oponent dot in there.
        if (verbose_R7)
        {
            cout << "   |   |-> R7: Check if the dot is on a bad globe after moving:" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 1:" << endl;
            cout << "   |   |   |   |-> On a board globe                 [ " << ((game_ptr->isGlobeCustom(P_dot_pos_moved)) == 1) << " ] " << endl;
            cout << "   |   |   |   |-> Is it occupied                   [ " << (game_ptr->isOccupied(P_dot_pos_moved) > 0) << " ] " << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 2:" << endl;
            cout << "   |   |   |   |-> On a home globe                  [ " << ((game_ptr->isGlobeCustom(P_dot_pos_moved)) == 2) << " ] " << endl;
            cout << "   |   |   |   |-> Is it occupied                   [ " << (game_ptr->isOccupied(P_dot_pos_moved) > 0) << " ] " << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 3:" << endl;
            cout << "   |   |   |   |-> On a double dot globe            [ " << ((game_ptr->isGlobeCustom(P_dot_pos_moved)) == 3) << " ] " << endl;
            cout << "   |   |   |   |-> Is it occupied                   [ " << (game_ptr->isOccupied(P_dot_pos_moved) > 0) << " ] " << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> State not active yet                 [ " << (state->at(6) != 1) << " ] " << endl;
        }

        //- State not active yet -
        if ((state->at(6) != 1))
        {
            switch (game_ptr->isGlobeCustom(P_dot_pos_moved))
            {
            case 0:
                /* After moved there isn't any globe */
                break;
            case 1:
                /* After moved there is a Globe (Board Globe) */
                if ((game_ptr->isOccupied(P_dot_pos_moved) > 0))
                {
                    state->at(6) = 1;
                    counter_s7++;
                    reward += reward_R7;
                    if (verbose_R7)
                    {
                        cout << "   |   |------> RESULT: (After moved) In a Board Globe with an oponent (Dot killed)" << endl;
                    }
                }
                break;
            case 2:
                /* After moved there is a Globe (Home Globe) */
                if ((game_ptr->isOccupied(P_dot_pos_moved) > 0))
                {
                    state->at(6) = 1;
                    counter_s7++;
                    reward += reward_R7;
                    if (verbose_R7)
                    {
                        cout << "   |   |------> RESULT: (After moved)  In a Home Globe with an oponent (Dot killed)" << endl;
                    }
                }
                break;
            case 3:
                /* After moved there is a Globe (Double occupied Globe) */
                state->at(6) = 1;
                counter_s7++;
                reward += reward_R7;
                if (verbose_R7)
                {
                    cout << "   |   |------> RESULT: (After moved)  In a DoubleDot Globe (Dot killed)" << endl;
                }
                break;
            default:
                break;
            }
        }

        // Check if dot is on the winers road, should be expensive to move here
        if (verbose_R8)
        {
            cout << "   |   |-> R8: Check if dot is on the winers road:" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 1:" << endl;
            cout << "   |   |   |   |-> ((50) < (P_dot_pos))                 [ " << ((50) < (P_dot_pos)) << " ] ,P_dot_pos [ " << P_dot_pos << " ]" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> State not active yet                 [ " << (state->at(7) != 1) << " ] " << endl;
        }

        if (((50) < (P_dot_pos)) && (state->at(7) != 1))
        {
            state->at(7) = 1;
            counter_s8++;
            reward += reward_R8;
            if (verbose_R8)
            {
                cout << "   |   |------> RESULT: (In game) Dot in winners road" << endl;
            }
        }

        // Check if the dot is the closest one to goal, but is not in the winners road.
        if (verbose_R9)
        {
            cout << "   |   |-> R9 : Check if the dot is the closest one to goal, but is not in the winners road.:" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 1:" << endl;
            cout << "   |   |   |   |-> (P_dot_pos < 51)                 [ " << (P_dot_pos < 51) << " ] ,P_dot_pos [ " << P_dot_pos << " ]" << endl;
        }
        int count = 0;
        int max_pos = -1;
        int P_other_dot_pos = 0;

        // State not active
        if ((state->at(8) != 1))
        {
            // Dot is not in the winners road
            if (P_dot_pos < 51)
            {
                for (uint i = 0; i < 4; i++)
                {

                    P_other_dot_pos = relative.pos[i];
                    // The other dot hasn finished and is not in the winners road
                    if (!(P_other_dot_pos == 99) && (P_other_dot_pos < 50))
                    {
                        if (P_other_dot_pos >= max_pos)
                        {
                            max_pos = P_other_dot_pos;
                        }
                    }
                    else
                    {
                        count++;
                    }
                }

                if (verbose_R9)
                {
                    cout << "   |   |   |   |-> (max_pos == P_dot_pos)           [ " << (max_pos == P_dot_pos) << " ] ,P_dot_pos [ " << P_dot_pos << " ] , max_pos [ " << max_pos << " ]" << endl;
                    cout << "   |   |   |   |-> count                            [ " << count << " ]" << endl;
                    cout << "   |   |   |" << endl;
                    cout << "   |   |   |-> State not active yet                 [ " << (state->at(8) != 1) << " ] " << endl;
                }

                if ((max_pos == P_dot_pos) && (state->at(8) != 1))
                {
                    state->at(8) = 1;
                    counter_s9++;
                    reward += reward_R9;

                    if (verbose_R9)
                    {
                        cout << "   |   |------> RESULT: Dot is the closest one to the goal, Dot pos: " << P_dot_pos << " , max_pos: " << max_pos << endl;
                    }
                }

                if (count == 3 && (state->at(8) != 1))
                {
                    state->at(8) = 1;
                    counter_s9++;
                    reward += reward_R9;
                    if (verbose_R9)
                    {
                        cout << "   |   |------> RESULT: Dot is the closest one to the goal, all the other dots have already finished" << endl;
                    }
                }
            }
        }
        // Check if the dot can finish in this movement:
        if (verbose_R10)
        {
            cout << "   |   |-> R10: Check if the dot can finish in this movement:" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> Case 1:" << endl;
            cout << "   |   |   |   |-> (P_dot_pos_moved == 56)          [ " << (P_dot_pos_moved == 56) << " ] ,P_dot_pos_moved [ " << P_dot_pos_moved << " ]" << endl;
            cout << "   |   |   |" << endl;
            cout << "   |   |   |-> State not active yet                 [ " << (state->at(9) != 1) << " ] " << endl;
        }

        if ((P_dot_pos_moved == 56) && (state->at(9) != 1))
        {
            state->at(9) = 1;
            counter_s10++;
            reward += reward_R10;
            if (verbose_R10)
            {
                cout << "   |   |------> RESULT: Dot can finish with this movement!" << endl;
            }
        }
    }
    else
    {
        if (verbose_R1)
        {
            if (P_dot_pos == -1)
            {
                cout << "   |   |------> RESULT: Dot Can't be moved (Dot in Home)" << endl;
            }
            else if (P_dot_pos == 99)
            {
                cout << "   |   |------> RESULT: Dot Can't be moved (Dot in Goal)" << endl;
            }
        }
    }

    int state_index = get_state_index(*state);
    if (state_index == 1)
    {
        reward += reward_R1;
    }
}

void ludo_player::update_Q_action_final(positions_and_dice relative)
{
    bool verbose = false;

    state_dot(action_max_ptr->at(1), relative, state_final_ptr);
    state_index_final = get_state_index(state_final);
    T_ptr->get_max_Q_value(state_index_final, action_final_ptr);

    double Q_previous_state = action_max_ptr->at(0);
    double reward_previous = action_max_ptr->at(3);
    double Q_new_state = action_final_ptr->at(0);

    if (verbose)
    {
        cout << "The reward is:  " << reward_previous << endl;
        cout << "The Q_new_state is:  " << Q_new_state << endl;
        cout << "The Q_previous_state is:  " << Q_previous_state << endl;
        cout << "The delta_Q is:  " << delta_Q << endl;
    }

    delta_Q = alpha * (reward_previous + gamma * (Q_new_state)-Q_previous_state);

    double Q_final = Q_previous_state + delta_Q;
    T_ptr->set_Q_value(action_max_ptr->at(2), Q_final);
}

void ludo_player::update_Q_action_max(positions_and_dice relative)
{
    // Update the state of all the dots:
    state_dot(0, relative, state_d1_ptr);
    double reward_d1 = reward;
    state_dot(1, relative, state_d2_ptr);
    double reward_d2 = reward;
    state_dot(2, relative, state_d3_ptr);
    double reward_d3 = reward;
    state_dot(3, relative, state_d4_ptr);
    double reward_d4 = reward;

    // Obtain the index of the state of each dot:
    state_index_d1 = get_state_index(state_d1);
    state_index_d2 = get_state_index(state_d2);
    state_index_d3 = get_state_index(state_d3);
    state_index_d4 = get_state_index(state_d4);

    // Obtain the max Q(s,a) value for each dot:
    T_ptr->get_max_Q_value(state_index_d1, action_d1_ptr);
    T_ptr->get_max_Q_value(state_index_d2, action_d2_ptr);
    T_ptr->get_max_Q_value(state_index_d3, action_d3_ptr);
    T_ptr->get_max_Q_value(state_index_d4, action_d4_ptr);

    // Initialize container:
    action_max_ptr->at(0) = -100.0;
    action_max_ptr->at(1) = 0.0;
    action_max_ptr->at(2) = 0.0;

    // When one of the dots is movable:
    if (state_index_d1 != 0 || state_index_d2 != 0 || state_index_d3 != 0 || state_index_d4 != 0)
    {

        double exploration_activation = 0.0;
        double EXPLORE_RATE = 0.9;

        if (training)
        {
            // Select between exploring and exploiting:
            exploration_activation = (rand() % 1000) / 1000.0;
        }
        else
        {
            exploration_activation = EXPLORE_RATE;
        }

        if (exploration_activation <= EXPLORE_RATE)
        {
            // Obtain the max Q(s,a) from all the dots:
            //cout << "---------------------Exploiting step!" << endl;
            if (action_d1_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d1_ptr->at(0);
                action_max_ptr->at(1) = 0;
                action_max_ptr->at(2) = state_index_d1;
                action_max_ptr->at(3) = reward_d1;
            }
            if (action_d2_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d2_ptr->at(0);
                action_max_ptr->at(1) = 1;
                action_max_ptr->at(2) = state_index_d2;
                action_max_ptr->at(3) = reward_d2;
            }
            if (action_d3_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d3_ptr->at(0);
                action_max_ptr->at(1) = 2;
                action_max_ptr->at(2) = state_index_d3;
                action_max_ptr->at(3) = reward_d3;
            }
            if (action_d4_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d4_ptr->at(0);
                action_max_ptr->at(1) = 3;
                action_max_ptr->at(2) = state_index_d4;
                action_max_ptr->at(3) = reward_d4;
            }
        }
        else
        {
            //cout << "Exploring step!" << endl;
            // QLEARNING:
            if (action_d1_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d1_ptr->at(0);
                action_max_ptr->at(2) = state_index_d1;
                action_max_ptr->at(3) = reward_d1;
            }
            if (action_d2_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d2_ptr->at(0);
                action_max_ptr->at(2) = state_index_d2;
                action_max_ptr->at(3) = reward_d2;
            }
            if (action_d3_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d3_ptr->at(0);
                action_max_ptr->at(2) = state_index_d3;
                action_max_ptr->at(3) = reward_d3;
            }
            if (action_d4_ptr->at(0) > action_max_ptr->at(0))
            {
                action_max_ptr->at(0) = action_d4_ptr->at(0);
                action_max_ptr->at(2) = state_index_d4;
                action_max_ptr->at(3) = reward_d4;
            }
            double action_Q = 0.0;
            int random_selection;

            while (action_Q == 0.0)
            {

                random_selection = (4.0 * rand() / (RAND_MAX + 1.0));

                /*// Check that we aren't picking a dot that can't be moved:            
                while (action_max_ptr->at(2) == 0.0)
                {
                int random_selection = (4.0 * rand() / (RAND_MAX + 1.0));

                switch (random_selection)
                {
                case 0:
                    action_max_ptr->at(0) = action_d1_ptr->at(0);
                    action_max_ptr->at(1) = 0;
                    action_max_ptr->at(2) = state_index_d1;
                    action_max_ptr->at(3) = reward_d1;
                    break;
                case 1:
                    action_max_ptr->at(0) = action_d2_ptr->at(0);
                    action_max_ptr->at(1) = 1;
                    action_max_ptr->at(2) = state_index_d2;
                    action_max_ptr->at(3) = reward_d2;
                    break;
                case 2:
                    action_max_ptr->at(0) = action_d3_ptr->at(0);
                    action_max_ptr->at(1) = 2;
                    action_max_ptr->at(2) = state_index_d3;
                    action_max_ptr->at(3) = reward_d3;
                    break;
                case 3:
                    action_max_ptr->at(0) = action_d4_ptr->at(0);
                    action_max_ptr->at(1) = 3;
                    action_max_ptr->at(2) = state_index_d4;
                    action_max_ptr->at(3) = reward_d4;
                    break;

                default:
                    break;
                }
                }*/

                // QLEARNING:
                switch (random_selection)
                {
                case 0:
                    action_Q = state_index_d1;
                    action_max_ptr->at(1) = random_selection;
                    break;
                case 1:
                    action_Q = state_index_d2;
                    action_max_ptr->at(1) = random_selection;
                    break;
                case 2:
                    action_Q = state_index_d3;
                    action_max_ptr->at(1) = random_selection;
                    break;
                case 3:
                    action_Q = state_index_d4;
                    action_max_ptr->at(1) = random_selection;
                    break;
                default:
                    break;
                }
            }
        }
    }
    else
    {
        // In case any dot can be moved:
        action_max_ptr->at(0) = 0.0;
        action_max_ptr->at(1) = -1;
        action_max_ptr->at(2) = 0;
        action_max_ptr->at(3) = 0.0;
    }
}

void ludo_player::state()
{
    std::cout << std::fixed;
    std::cout << std::setprecision(4);

    cout << "  ------------------------------------------------------------------------------------" << endl;
    cout << "  |                                      Player State                                |" << endl;
    cout << "  ------------------------------------------------------------------------------------" << endl;
    cout << setw(5) << "  |        |" << setw(5) << "R1" << setw(5) << "R2" << setw(5) << "R3" << setw(5) << "R4" << setw(5) << "R5" << setw(5) << "R6" << setw(5) << "R7" << setw(5) << "R8" << setw(5) << "R9" << setw(5) << "R10" << setw(5) << " |   s_i  | " << setw(9) << "Q(s,M)" << setw(4) << " | " << endl;
    cout << "  ------------------------------------------------------------------------------------" << endl;
    cout << setw(5) << "  | Dot  1 |" << setw(5) << state_d1.at(0) << setw(5) << state_d1.at(1) << setw(5) << state_d1.at(2) << setw(5) << state_d1.at(3) << setw(5) << state_d1.at(4) << setw(5) << state_d1.at(5) << setw(5) << state_d1.at(6) << setw(5) << state_d1.at(7) << setw(5) << state_d1.at(8) << setw(5) << state_d1.at(9) << " |";
    (T_ptr->print_Q_state(state_index_d1, 2));
    cout << setw(2) << "  |" << endl;
    cout << setw(5) << "  | Dot  2 |" << setw(5) << state_d2.at(0) << setw(5) << state_d2.at(1) << setw(5) << state_d2.at(2) << setw(5) << state_d2.at(3) << setw(5) << state_d2.at(4) << setw(5) << state_d2.at(5) << setw(5) << state_d2.at(6) << setw(5) << state_d2.at(7) << setw(5) << state_d2.at(8) << setw(5) << state_d2.at(9) << " |";
    (T_ptr->print_Q_state(state_index_d2, 2));
    cout << setw(2) << "  |" << endl;
    cout << setw(5) << "  | Dot  3 |" << setw(5) << state_d3.at(0) << setw(5) << state_d3.at(1) << setw(5) << state_d3.at(2) << setw(5) << state_d3.at(3) << setw(5) << state_d3.at(4) << setw(5) << state_d3.at(5) << setw(5) << state_d3.at(6) << setw(5) << state_d3.at(7) << setw(5) << state_d3.at(8) << setw(5) << state_d3.at(9) << " |";
    (T_ptr->print_Q_state(state_index_d3, 2));
    cout << setw(2) << "  |" << endl;
    cout << setw(5) << "  | Dot  4 |" << setw(5) << state_d4.at(0) << setw(5) << state_d4.at(1) << setw(5) << state_d4.at(2) << setw(5) << state_d4.at(3) << setw(5) << state_d4.at(4) << setw(5) << state_d4.at(5) << setw(5) << state_d4.at(6) << setw(5) << state_d4.at(7) << setw(5) << state_d4.at(8) << setw(5) << state_d4.at(9) << " |";
    (T_ptr->print_Q_state(state_index_d4, 2));
    cout << setw(2) << "  |" << endl;
    cout << "  ------------------------------------------------------------------------------------" << endl;
    cout << "  | Moving Dot:" << setw(5) << (int)(action_max_ptr->at(1) + 1) << "                                      MAX: |" << setw(7) << (int)action_max_ptr->at(2) << " |   " << setw(7) << action_max_ptr->at(0) << setw(2) << "  |" << endl;
    cout << "  ------------------------------------------------------------------------------------" << endl;
}

void ludo_player::print_board_state(positions_and_dice relative)
{

    cout << endl;
    cout << "  --------------------------------" << endl;
    cout << "  |         Board State          |" << endl;
    cout << "  --------------------------------" << endl;
    cout << "  | Dice: " << dice_roll << "|" << setw(5) << "1" << setw(5) << "2" << setw(5) << "3" << setw(7) << "4 |" << endl;
    cout << "  --------------------------------" << endl;
    cout << setw(5) << "  |Player 1|" << setw(5) << relative.pos[0] << setw(5) << relative.pos[1] << setw(5) << relative.pos[2] << setw(5) << relative.pos[3] << " |" << endl;
    cout << setw(5) << "  |Player 2|" << setw(5) << relative.pos[4] << setw(5) << relative.pos[5] << setw(5) << relative.pos[6] << setw(5) << relative.pos[7] << " |" << endl;
    cout << setw(5) << "  |Player 3|" << setw(5) << relative.pos[8] << setw(5) << relative.pos[9] << setw(5) << relative.pos[10] << setw(5) << relative.pos[11] << " |" << endl;
    cout << setw(5) << "  |Player 4|" << setw(5) << relative.pos[12] << setw(5) << relative.pos[13] << setw(5) << relative.pos[14] << setw(5) << relative.pos[15] << " |" << endl;
    cout << "  --------------------------------" << endl;
}

void ludo_player::player_mode(int mode)
{

    switch (mode)
    {
    case 1:
        step_by_step = true;
        break;

    case 2:
        training = false;
        break;

    case 3:
        training = true;
        break;

    default:
        break;
    }
}

void ludo_player::save_data(std::string filepath, int d1, int d2, int d3, int d4, int d5, int d6, int d7, int d8, int d9, int d10)
{
    std::ofstream file;
    //can't enable exception now because of gcc bug that raises ios_base::failure with useless message
    //file.exceptions(file.exceptions() | std::ios::failbit);
    file.open(filepath, std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file.seekp(0, ios::end);
    size_t size = file.tellp();
    if (size == 0)
    {

        cout << "File: " << filepath << " wasn't initialized, initializing now." << endl;
        file << " Data =[";
    }

    file << d1 << "," << d2 << "," << d3 << "," << d4 << "," << d5 << "," << d6 << "," << d7 << "," << d8 << "," << d9 << "," << d10 << ";"
         << "\n";
}

void ludo_player::reset_state_counters()
{
    // Save current values:
    save_data("Data/DATA_STATE_COUNTERS.txt", counter_s1, counter_s2, counter_s3, counter_s4, counter_s5, counter_s6, counter_s7, counter_s8, counter_s9, counter_s10);
    // For data gathering:
    counter_s1 = 0;
    counter_s2 = 0;
    counter_s3 = 0;
    counter_s4 = 0;
    counter_s5 = 0;
    counter_s6 = 0;
    counter_s7 = 0;
    counter_s8 = 0;
    counter_s9 = 0;
    counter_s10 = 0;
}
