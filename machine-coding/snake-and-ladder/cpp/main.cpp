#include <string>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace std;

enum CellType {
    NORMAL,
    SNAKE_START,
    SNAKE_END,
    LADDER_START,
    LADDER_END
};

class Player {
    string name;
    int id;
public:
    Player() = default;
    int getID() {}
};

class Teleporter {
    int startPoint;
    int endPoint;
public:
    Teleporter(int start, int end): startPoint(start), endPoint(end) {}
    virtual int teleport() {} ;
    int getStart() {}
};

class Snake: public Teleporter {
public:
    Snake(int start, int end): Teleporter(start, end) {
        if(end>=start) throw logic_error("Invalid snake");
    }
    virtual int teleport() {
        cout<<"Snake bite\n";
        Teleporter::teleport();
    } ;
};

class Ladder {
public:
    Ladder() = default;
};

class Dice {
public:
    int Roll() {}
};

class Board {
    static int ids;
    int id;
    vector<Player*> players;
    map<int, int> playerPositions;
    Dice* dice;
    int turn;
    vector<CellType> board;
    map<int, Teleporter*> teleporters;
public:
    Board(vector<Player*>& p): id(ids++), players(p), turn(0) {
        board.resize(100, CellType::NORMAL);
        Snake* snake1 = new Snake(27, 13);
        teleporters[snake1->getStart()] = snake1;
        //Add snakes

        dice = new Dice();
        for(int i=0;i<p.size();i++){
            players[p[i]->getID()] = 0;
        }
    }

    int getID() {}

    void NextTurn() {
        turn = (turn+1)%players.size();
    }

    int isGameOver() {
        for(auto& [id, position]: playerPositions) {
            if(position == 99) return id;
        }
        return -1;
    }

    int Play() {
        while(isGameOver() == -1) {
            int steps = dice->Roll();
            int playerID = players[turn]->getID();
            int position = playerPositions[playerID] + steps;
            if(teleporters.find(position) != teleporters.end()) {
                position = teleporters[position]->teleport();
            }

            playerPositions[playerID] = position;
            NextTurn();
        }

        return isGameOver();
    }
};

int Board::ids = 1;

class GameManager {
    GameManager() = default;
    map<int, Board*> games;
public:
    static GameManager& getInstance() {
        static GameManager instance;
        return instance;
    }

    Board* startGame(vector<Player*>& p) {
        if(p.size() > 6) throw invalid_argument("Players should be less than 7");
        auto newGame = new Board(p);
        games[newGame->getID()] = newGame;
        return newGame;
    }
};

int main() {

}