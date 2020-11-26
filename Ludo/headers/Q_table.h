#ifndef Q_TABLE_H
#define Q_TABLE_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <iomanip>

using namespace std;

// Structure for the rows of the Q table:
struct Q_state
{
    std::vector<double> val;

    Q_state()
    {
        val.resize(1);
        val.at(0) = 0.0001;
    }
};

class Q_Table
{

private:
    // Creating the Q table:
    std::vector<Q_state> Q_table;
    // Default size for the table:
    int Q_table_size = 0;
    // Name of the file containing the Qtable
    string doc_name = "Qtable.txt";
    // Data statistics variables:
    int explored_states_count = 0;

public:
    Q_Table();
    void initialize_Q_table(int Q_table_size);
    void print_Q_state(int state_index, int style);
    void get_max_Q_value(int state_index, std::vector<double> *result);
    void set_Q_value(int state_index, double Q_value);
    void print_Q_table();
    void save_Q_table();
    void load_Q_table();
    void save_data(string filepath, double d1,double d2,double d3,double d4,double d5,double d6);
};

#endif // Q_table_H
