# Ludo player using Reinforced Learning
In this project, a Ludo player based on Reinforced learning is implemented. The
algorithm chosen is SARSA since it should be more conservative than Q-learning
and look for avoiding the actions that lead to negative rewards. Although the
solution obtained won’t be the optimal, in a game like ludo, where the bad
rewarded actions are clear, prioritizing safety may be important and influence the
performance of the learning agent. In order to compare the performance of the
player, it will be compared to one whose implemented algorithm is  − Greedy
Q learning.

This project was the semester project for the course Artificial Intelligence II at SDU. 
For a deeper detail about the procedure, the project report can be found [here](https://drive.google.com/file/d/1eMCltVqWKoC67PwFCYdu060NRtnjn1u0/view?usp=sharing).

## Usage
First we need to build the project:
```bash
mkdir build && cd build
mkdir Data
qmake ../Ludo/Ludo.pro 
make
```
When we run the executable, the Ludo board will pop up. The current configuration shows the trained player (red) playing against three random players. The test will run 1000 matches while showing the results (percentage of won games) in the console. In order to modify any of the main parameters, they are found in the **main.cpp** file.

![](Ludo/images/Ludo_player.gif) 

## References
The Ludo Gui and backend is provided by [niive12](https://gitlab.com/niive12/ludo-gui).