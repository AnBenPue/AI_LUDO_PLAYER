#include "./headers/Q_table.h"

using namespace std;

Q_Table::Q_Table()
{
}

void Q_Table::initialize_Q_table(int Q_table_size)
{

    this->Q_table.resize(Q_table_size);
    this->Q_table_size = Q_table_size;
}

void Q_Table::print_Q_state(int state_index, int style)
{
    if (style == 1)
    {
        if (state_index <= this->Q_table_size)
        {
            cout << "-------------------------------------" << endl;
            cout << "|   Q_state   |" << setw(5) << "1" << setw(5) << "2" << setw(5) << "3" << setw(7) << "4 |" << endl;
            cout << "-------------------------------------" << endl;
            cout << "|" << setw(7) << state_index << setw(7) << "|" << setw(5) << (Q_table.at(state_index)).val.at(0) << " |" << endl;
            cout << "-------------------------------------" << endl;
        }
        else
        {
            cout << "-------------------------------------" << endl;
            cerr << "| ERROR: The state you want to acces doesn exist." << endl;
            cerr << "|       Q_table_size = " << this->Q_table_size << " , Requested state: " << state_index << endl;
            cout << "-------------------------------------" << endl;
        }
    }
    else if (style == 2)
    {
        cout << setw(7) << state_index << " |" << setw(10) << (Q_table.at(state_index)).val.at(0);
    }
}

void Q_Table::get_max_Q_value(int state_index, std::vector<double> *result)
{
    // I know it doesn make any sense when we only have one possible option...
    result->resize(1);
    result->at(0) = (Q_table.at(state_index)).val.at(0);
}

void Q_Table::set_Q_value(int state_index, double Q_value)
{
    (Q_table.at(state_index)).val.at(0) = Q_value;
}

void Q_Table::print_Q_table()
{
    std::cout << std::fixed;
    std::cout << std::setprecision(2);

    cout << "  ------------------------" << endl;
    cout << "  |        Q Table       |" << endl;
    cout << "  ------------------------" << endl;
    cout << "  |   s_i   | " << setw(10) << "Q(s,M)"
         << " |" << endl;
    cout << "  ------------------------" << endl;

    // Initialize counter:
    explored_states_count = 0;

    for (uint i; i < Q_table_size; i++)
    {
        // Only print the states that have been explored:
        if (((Q_table.at(i)).val.at(0) != 0.0001))
        {
            explored_states_count++;
            cout << "  | ";
            print_Q_state(i, 2);
            cout << "  | " << endl;
        }
    }
    cout << "  ------------------------" << endl;
    cout << "  | Nº explored: " << setw(6) << explored_states_count << "  |" << endl;
    cout << "  ------------------------" << endl;

}

void Q_Table::save_Q_table()
{
    std::ofstream file;
    std::string filename("Qtable.txt");
    file.open(filename);

    if (file.is_open())
    {
        file.seekp(0, ios::end);
        size_t size = file.tellp();
        if (size == 0)
        {
            //cout << " File: " << filename << " wasn't initialized, size [ " << size << " ], initializing now." << endl;
        }
        else
        {
            //cout << " File: " << filename << " opened, size [ " << size << " ]" << endl;
        }
    }
    else
    {
        cout << " ERROR: Coldn't open file: " << filename << endl;
    }

    // Initialize counter:
    explored_states_count = 0;

    for (uint i; i < Q_table_size; i++)
    {

        file << (Q_table.at(i)).val.at(0) << "\n";
        
        // Only print the states that have been explored:
        if (((Q_table.at(i)).val.at(0) != 0.0001))
        {
            explored_states_count++;
        }
    }

    file.close();
    // Saving data for the statistics:
    save_data("Data/DATA_Q_TABLE.txt", explored_states_count,(Q_table.at(641)).val.at(0),(Q_table.at(291)).val.at(0),(Q_table.at(65)).val.at(0),(Q_table.at(67)).val.at(0),(Q_table.at(5)).val.at(0));

}
void Q_Table::load_Q_table()
{
    std::ifstream file;
    std::string filename("Qtable.txt");
    file.open(filename, std::ios::in);

    string line;

    if (file.is_open())
    {
        int i = 0;
        while (!file.eof() && i < Q_table_size)
        {

            getline(file, line);
            (Q_table.at(i)).val.at(0) = atof(line.c_str());
            i++;
        }
    }
    else
    {
        cout << " ERROR: Coldn't open file: " << filename << endl;
    }

    // Checking that table was loaded correctly:
    cout << "INFO: Q table file:" << endl;
    system("head Qtable.txt");

    cout << endl
         << "INFO: Q table after loading:" << endl;
    print_Q_table();
    cout << endl;

    file.close();
}
void Q_Table::save_data(string filepath, double d1,double d2,double d3,double d4,double d5,double d6)
{
    std::ofstream file;
    //can't enable exception now because of gcc bug that raises ios_base::failure with useless message
    //file.exceptions(file.exceptions() | std::ios::failbit);
    file.open(filepath, std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    //make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file.seekp(0,ios::end);
    size_t size = file.tellp();
    if(size==0){

      cout<< "File: " << filepath << " wasn't initialized, initializing now." << endl;
      file  << " Data =[" ;
    }

    file  << d1 << "," <<  d2 << "," <<  d3 << "," <<  d4 << "," <<  d5 << "," << d6 << ";"  << "\n";
}