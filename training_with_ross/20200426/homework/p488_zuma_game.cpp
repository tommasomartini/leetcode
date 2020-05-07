#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

using namespace std;

#define DEBUG 0

enum Color {R=0, Y, B, G, W};

struct Group {
    Color c;
    int n;
};

Color charToColor(char c) {
    switch (c) {
        case 'R':
            return R;
        case 'Y':
            return Y;
        case 'B':
            return B;
        case 'G':
            return G;
        case 'W':
            return W;
        default:
            throw runtime_error("Unknown character " + c);
    }
}

char colorToChar(Color c) {
    switch (c) {
        case R:
            return 'R';
        case Y:
            return 'Y';
        case B:
            return 'B';
        case G:
            return 'G';
        case W:
            return 'W';
        default:
            throw runtime_error("Unknown color " + c);
    }
}

void printHand(vector<int>& hand) {
    cout << "hand=[";
    cout << "R=" << hand[0] << ", ";
    cout << "Y=" << hand[1] << ", ";
    cout << "B=" << hand[2] << ", ";
    cout << "G=" << hand[3] << ", ";
    cout << "W=" << hand[4] << "]" << endl;
}

void printBoard(vector<Group>& board) {
    string row1 = "board [ ";
    string row2 = "      [ ";
    for (auto& g: board) {
        row1.push_back(colorToChar(g.c));
        row1 += " ";
        row2 += to_string(g.n) + " ";
    }
    row1 += "]";
    row2 += "]";
    cout << row1 << endl;
    cout << row2 << endl;
}

bool validateSolution(vector<int>& solution, vector<int>& hand) {
    for (int i = 0; i < solution.size(); ++i) {
        if (solution[i] > hand[i]) {
            return false;
        }
    }
    return true;
}

class Solution {
public:
    Solution() : _hand(5, 0) {};

    int findMinStep(string board, string hand) {
        for (char& c: hand) {
            ++_hand[charToColor(c)];
        }

        printHand(_hand);

        char currColor;
        int i = 0;
        int j;
        while (i < board.length()) {
            currColor = board[i];
            j = i + 1;
            for (; board[j] == currColor && j < board.length(); ++j) {}
            Group group{charToColor(currColor), j - i};
            _board.push_back(group);
            i = j; 
        }
        printBoard(_board);

        // Here comes the Dynamic Programming part.
        
        // We fill a matrix with dimensions the size of the board (N x N).
        // Each cell in the matrix represents a subsequence of the original
        // board and contains all the possible sets of colors to solve that
        // subsequence.
        // For instance, if the subsequence is "WWBBW", we could solve it
        // by using a single "1-B" (erase the central B-group and the W at
        // the sides will vanish) or "3-W, 1-B" (erase the side W first and
        // then the central Bs).
        // Each subsequence solution is represented by a 5-elements vector
        // with the amount of colors needed: R-Y-B-G-W.
        vector<set<vector<int>>> emptyRow(_board.size());
        vector<vector<set<vector<int>>>> mat(_board.size(), emptyRow);

#ifdef DEBUG
        cout << "Diagonal" << endl;
#endif

        // The diagonal stores the values to solve each single group in the board.
        for (int i = 0; i < _board.size(); ++i) {
            vector<int> solution(5, 0);
            Group gr = _board[i];
            solution[gr.c] = 3 - gr.n;
            if (validateSolution(solution, _hand)) {
                mat[i][i].insert(solution);
#ifdef DEBUG
            cout << solution[gr.c] << colorToChar(gr.c) << " ";
#endif
            } else {
#ifdef DEBUG
                cout << "-- ";
#endif
            }
        }

#ifdef DEBUG
        cout << endl;
#endif

        // Iterate the board considering different intervals.
        for (int diff = 1; diff < _board.size(); ++diff) {
#ifdef DEBUG
        cout << "\nDiff: " << diff << endl;
#endif
            int hi;
            Group loGroup, hiGroup;
            for (int lo = 0; lo < _board.size() - diff; ++lo) {
                hi = lo + diff;
                loGroup = _board[lo];
#ifdef DEBUG
                string row1 = " [ ";
                string row2 = " [ ";
                for (int k = lo; k <= hi; ++k) {
                    row1.push_back(colorToChar(_board[k].c));
                    row1 += " ";
                    row2 += to_string(_board[k].n) + " ";
                }
                cout << row1 << "]" << endl;
                cout << row2 << "]" << endl;
#endif
                hiGroup = _board[hi];

                if (loGroup.c == hiGroup.c) {
                    // First and last colors of this subsequence match.
                    // I can solve this subsequence by erasing what is in the middle
                    // and possibly adding only one extra ball (in case there is only
                    // a single ball per side).
                    // Note that, since we already grouped together sequences with
                    // the same color, if the left and right groups are the same,
                    // they must have a subsequence between them.
#ifdef DEBUG
                    cout << "  Same extremes" << endl;
                    cout << "   Solutions" << endl;
                    cout << "    R Y B G W" << endl;
#endif
                    set<vector<int>> midSequenceSolutions = mat[lo + 1][hi - 1];
                    for (vector<int> midSeqSol : midSequenceSolutions) {
                        // Copy-construct a new mid-subsequence solution.
                        vector<int> sol(midSeqSol);
                        
                        // Add the extra color to solve the edge balls.
                        sol[loGroup.c] += 3 - min(3, loGroup.n + hiGroup.n);

#ifdef DEBUG
                        cout << "    ";
                        for (auto& v : sol) {
                            cout << v << " ";
                        }
#endif
                        if (validateSolution(sol, _hand)) {
                            // If this solution is feasible, insert it.
                            mat[lo][hi].insert(sol);
#ifdef DEBUG
                            cout << " -> V";
#endif
                        }
#ifdef DEBUG
                            cout << endl;
#endif
                    }
                } else {
                    // First and last colors of this subsequence do not match.
                    // The possible solutions are the union of the two sets of solutions:
                    // * match left color + solutions [lo+1, hi]
                    // * match right color + solution [lo, hi-1]

#ifdef DEBUG
                    cout << "  Different extremes" << endl;
                    cout << "   Solutions" << endl;
                    cout << "    R Y B G W" << endl;
#endif
                    // Left color + right subsequence.
                    for (vector<int> rightSol : mat[lo + 1][hi]) {
                        // Copy-construct a new solution.
                        vector<int> sol(rightSol);
                        
                        // Add the left color.
                        sol[loGroup.c] += 3 - loGroup.n;

#ifdef DEBUG
                        cout << "    ";
                        for (auto& v : sol) {
                            cout << v << " ";
                        }
#endif
                        if (validateSolution(sol, _hand)) {
                            // If this solution is feasible, insert it.
                            mat[lo][hi].insert(sol);
#ifdef DEBUG
                            cout << " -> V";
#endif
                        }
#ifdef DEBUG
                        cout << endl;
#endif
                    }
                    
                    // Right color + left subsequence.
                    for (vector<int> leftSol : mat[lo][hi - 1]) {
                        // Copy-construct a new solution.
                        vector<int> sol(leftSol);
                        
                        // Add the right color.
                        sol[hiGroup.c] += 3 - hiGroup.n;

#ifdef DEBUG
                        cout << "    ";
                        for (auto& v : sol) {
                            cout << v << " ";
                        }
#endif
                        if (validateSolution(sol, _hand)) {
                            // If this solution is feasible, insert it.
                            mat[lo][hi].insert(sol);
#ifdef DEBUG
                            cout << " -> V";
#endif
                        }
#ifdef DEBUG
                        cout << endl;
#endif
                    }
                }
            }
        }

        // After filling the matrix, the top-right cell contains the solution
        // for the entire sequence.
        if (mat[0][_board.size() - 1].empty()) {
            return -1;
        }

        int res = numeric_limits<int>::max();
        for (vector<int> sol : mat[0][_board.size() - 1]) {
            // Count the value of each solution;
            int solValue = accumulate(sol.begin(), sol.end(), 0);
            res = min(res, solValue);
        }

        return res;
    }

private:
    vector<Group> _board;
    vector<int> _hand;
};

int main() {
    string board;
    string hand;
    cout << "Board: ";
    getline(cin, board);
    cout << "Hand: ";
    getline(cin, hand);

    Solution sol;
    int res = sol.findMinStep(board, hand);
    cout << "Result: " << res << endl;
}

