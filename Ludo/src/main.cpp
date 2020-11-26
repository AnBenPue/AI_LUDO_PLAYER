
#include <QApplication>
#include "./headers/dialog.h"
#include "./headers/game.h"
#include "./headers/ludo_player.h"
#include "./headers/ludo_player_random.h"
#include "./headers/positions_and_dice.h"
#include "./headers/Q_table.h"
#include <iostream>
#include <vector>

Q_DECLARE_METATYPE(positions_and_dice)

int main(int argc, char *argv[])
{
    // Game and Q_table object initialization:
    game Game;
    game *Game_ptr;
    Game_ptr = &Game;

    Q_Table Table;
    Q_Table *Table_ptr;
    Table_ptr = &Table;

    // QT app initialization:
    QApplication a(argc, argv);
    qRegisterMetaType<positions_and_dice>();

    // Control variables:
    bool loop = true;
    int number_of_games = 100000;
    // loop mode:
    bool training = false;
    bool test = true;
    // For one game run:
    bool step_by_step = true;
    int delay = 10;

    // Instanciate the players here:

    ludo_player p4(Game_ptr, Table_ptr);    
    ludo_player_random p2,p3,p1;

    /*ludo_player p1(Game_ptr, Table_ptr);
    ludo_player p4(Game_ptr, Table_ptr);
    ludo_player p3(Game_ptr, Table_ptr);
    ludo_player_random p2;*/

    // Load the Q_table:
    Table.load_Q_table();

    // Game app connections:
    QObject::connect(&Game, SIGNAL(close()), &a, SLOT(quit())); //ADDED

    //set up for each player
    QObject::connect(&Game, SIGNAL(player1_start(positions_and_dice)), &p1, SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p1, SIGNAL(select_piece(int)), &Game, SLOT(movePiece(int)));
    QObject::connect(&Game, SIGNAL(player1_end(std::vector<int>)), &p1, SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p1, SIGNAL(turn_complete(bool)), &Game, SLOT(turnComplete(bool)));

    QObject::connect(&Game, SIGNAL(player2_start(positions_and_dice)), &p2, SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p2, SIGNAL(select_piece(int)), &Game, SLOT(movePiece(int)));
    QObject::connect(&Game, SIGNAL(player2_end(std::vector<int>)), &p2, SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p2, SIGNAL(turn_complete(bool)), &Game, SLOT(turnComplete(bool)));

    QObject::connect(&Game, SIGNAL(player3_start(positions_and_dice)), &p3, SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p3, SIGNAL(select_piece(int)), &Game, SLOT(movePiece(int)));
    QObject::connect(&Game, SIGNAL(player3_end(std::vector<int>)), &p3, SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p3, SIGNAL(turn_complete(bool)), &Game, SLOT(turnComplete(bool)));

    QObject::connect(&Game, SIGNAL(player4_start(positions_and_dice)), &p4, SLOT(start_turn(positions_and_dice)));
    QObject::connect(&p4, SIGNAL(select_piece(int)), &Game, SLOT(movePiece(int)));
    QObject::connect(&Game, SIGNAL(player4_end(std::vector<int>)), &p4, SLOT(post_game_analysis(std::vector<int>)));
    QObject::connect(&p4, SIGNAL(turn_complete(bool)), &Game, SLOT(turnComplete(bool)));

    // Add a GUI
    Dialog w;
    QObject::connect(&Game, SIGNAL(update_graphics(std::vector<int>)), &w, SLOT(update_graphics(std::vector<int>)));
    QObject::connect(&Game, SIGNAL(set_color(int)), &w, SLOT(get_color(int)));
    QObject::connect(&Game, SIGNAL(set_dice_result(int)), &w, SLOT(get_dice_result(int)));
    QObject::connect(&Game, SIGNAL(declare_winner(int)), &w, SLOT(get_winner(int)));
    w.show();
    //

    if (loop)
    {
        if (training)
        {
            // Activate exploring:
            //p1.player_mode(3);
            //p3.player_mode(3);
            p4.player_mode(3);
        }
        else if (test)
        {
            // Deactivate exploring:
            p4.player_mode(2);
        }

        Game.setGameDelay(0); //if you want to see the game, set a delay

        for (int i = 0; i < number_of_games; ++i)
        {
            std::cout << " NEW GAME! " << std::endl;
            //Table.print_Q_table();
            Game.start();
            a.exec();
            Game.reset();

            //p1.reset_state_counters();
            //p3.reset_state_counters();
            p4.reset_state_counters();
        }

        return 0;
    }
    else
    {
        if (step_by_step)
        {
            // Activate steps:
            p4.player_mode(1);
        }

        Game.setGameDelay(delay); //if you want to see the game, set a delay
        Game.start();
        return a.exec();
    }
}
