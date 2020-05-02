#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
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
    cout << "[";
    cout << "R=" << hand[0] << ", ";
    cout << "Y=" << hand[1] << ", ";
    cout << "B=" << hand[2] << ", ";
    cout << "G=" << hand[3] << ", ";
    cout << "W=" << hand[4] << "]" << endl;
}

string boardToString(vector<Group>& board, int lo, int hi) {
    string row = "[ ";
    Group g;
    for (int i = lo; i <= hi; ++i) {
        g = board[i];
        row += to_string(g.n);
        row.push_back(colorToChar(g.c));
        row += " ";
    }
    row += "]";
    return row;
}

string solutionToString(vector<int>& solution) {
    string row = to_string(solution[0]);
    for (int i = 1; i < 5; ++i) {
        row += " " + to_string(solution[i]);
    }
    return row;
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
        // Fill the hand.
        for (char& c: hand) {
            ++_hand[charToColor(c)];
        }

#ifdef DEBUG
        cout << "Hand" << endl;
        printHand(_hand);
#endif

        // Fill the board.
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

#ifdef DEBUG
        cout << "Board: " << boardToString(_board, 0, _board.size() - 1) << endl;
#endif

        // Fill the color map: for each color, stores the indices of the 
        // relative groups.
        map<Color, vector<int>> colorIndices;
        for (int groupIdx = 0; groupIdx < _board.size(); ++groupIdx) {
            colorIndices[_board[groupIdx].c].push_back(groupIdx);
        }

#ifdef DEBUG
        cout << "Color indices" << endl;
        for (pair<Color const, vector<int>>& p : colorIndices) {
            cout << colorToChar(p.first) << ":";
            for (int v : p.second) {
                cout << " " << v;
            }
            cout << endl;
        }
#endif

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
        //
        // At each iteration, consider the subsequence of groups, in particular
        // the rightmost group. How can we erase this substring? To erase the
        // sequence we must erase the rightmost group, at some point. We can
        // do it in 2 ways:
        //  1. erase the sequence up to the group before the last one (solved
        //     in a previous iteration) and then add as many balls as needed
        //     to remove the last character.
        //  2. Match the last group with some groups of the same color in the
        //     middle and erase the sequence before it.
        //     Example: RWWYYW can be solved as R-WW(YY)W.
        //     This case also involves grouping more than two groups together.
        //     For example RWYWRRW can be solved as R-W(Y)W(RR)W.
        //
        //     Case 2 can be solved in different ways, depending on how we match
        //     the group of the same color in the right sequence.
        //     a. Combine 2 groups: X(...)X
        //        We solve the sequence in the middle using a previous subcase
        //        and we possibly add one X to complete the combination.
        //     b. Combine 3 groups: X(...)X(...)X
        //        Solve two subsequences between groups of X, so that the X's
        //        end up next to each other and elide. No ball addition needed.
        //        Each group contains either 1 or 2 balls, therefore there are
        //        8 possible dispositions of X groups. However, only 3 can be
        //        canceled by grouping:
        //        1. X(yyy)X(zzz)X    -> order of erase does not matter
        //        2. XX(yyy)X(zzz)X   -> erase (zzz) first
        //        3. X(yyy)X(zzz)XX   -> erase (yyy) first
        //        In all the other 5 cases, if we elide (yyy) or (zzz) two
        //        groups will cancel immediately, before combining the remaining
        //        third group and we fall back in case 2a.
        //        Example:
        //          X(yyy)XX(zzz)X -> X()XX(zzz)X -> (zzz)X
        //                         -> X(yyy)XX()X -> X(yyy)
        vector<set<vector<int>>> emptyRow(_board.size());
        vector<vector<set<vector<int>>>> mat(_board.size(), emptyRow);

#ifdef DEBUG
        cout << "Diagonal" << endl;
#endif

        // The diagonal stores the values to solve each single group in the board.
        // This is the base case.
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
                hiGroup = _board[hi];
#ifdef DEBUG
                cout << boardToString(_board, lo, hi) << endl;
#endif

                // Match the rightmost group with all the groups in the current
                // subsequence with the same color.
                Color& currColor = hiGroup.c;
                vector<int>& currColorIndices = colorIndices[currColor];
                for (int idx1 : currColorIndices) {
                    if (idx1 < lo) {
                        continue;
                    }

                    if (idx1 > hi) {
                        break;
                    }

                    if (idx1 == hi) {
                        // Case 1: erase subsequence before last group + erase last group.
                        //  (www)-X

#ifdef DEBUG
                        cout << " Case 1: ";
                        cout << boardToString(_board, lo, idx1 - 1) << " + ";
                        cout << boardToString(_board, idx1, idx1) << endl;
#endif

                        set<vector<int>> solutionsBeforeMatch1 = mat[lo][hi - 1];
                        if (solutionsBeforeMatch1.empty()) {
                            // Cannot solve this subsequence.
                            // Since index 1 is at the highest possible position, it makes no sense
                            // to continue. We break here.
#ifdef DEBUG
                            cout << "  " << boardToString(_board, lo, idx1 - 1) << " has no solution" << endl;
#endif
                            break;
                        }

                        for (vector<int> sol : solutionsBeforeMatch1) {
                            sol[hiGroup.c] += 3 - hiGroup.n;
                            if (validateSolution(sol, _hand)) {
                                mat[lo][hi].insert(sol);
                            }
                        }

#ifdef DEBUG
                        cout << "  Valid solutions:" << endl;
                        cout << "   R Y B G W" << endl;
                        for (vector<int> sol : mat[lo][hi]) {
                            cout << "   " << solutionToString(sol) << endl;
                        }
#endif
                    } else {
                        // lo <= idx1 < hi
                        // Case 2: erase the last group using a group of the same color in
                        // the middle of the current subsequence.
                    
                        set<vector<int>> solutionsBeforeMatch1;
                    
                        // Early check that the sequence before index 1 can be solved. If it cannot, neither
                        // will the entire sequence in [lo, hi].
                        if (idx1 == lo) {
                            // Nothing to solve before the math at index 1.
                            solutionsBeforeMatch1.insert(vector<int>(5, 0));
                        } else {
                            solutionsBeforeMatch1 = mat[lo][idx1 - 1];
                            if (solutionsBeforeMatch1.empty()) {
#ifdef DEBUG
                                cout << " Case 2: " << boardToString(_board, lo, idx1 - 1) << " + ";
                                cout << boardToString(_board, idx1, hi) << endl;
                                cout << "  No solution for " << boardToString(_board, lo, idx1 - 1) << endl;
#endif
                                continue;
                            }        
                        }

                        // TODO: Use iterators instead of idx1 and idx2!
                        for (int idx2 : currColorIndices) {
                            if (idx2 <= idx1) {
                                continue;
                            }

                            if (idx2 > hi) {
                                break;
                            }

#ifdef DEBUG
                            cout << " idx1=" << idx1 << " idx2=" << idx2 << endl;
#endif

                            set<vector<int>> solutionsMidLeft;
                            set<vector<int>> solutionsMidRight;
                            int numColorsToAdd = 0;     // amount of extra balls to erase the matching groups at the edge

                            if (idx2 == hi) {
                                // lo <= idx1 < idx2==hi
                                // Case 2a: combine two groups
                                //  (www)-X(yyy)X
                                                        
                                solutionsMidRight.insert(vector<int>(5, 0));
                                solutionsMidLeft = mat[idx1 + 1][hi - 1];
                                if (solutionsMidLeft.empty()) {
                                    // If a subsequence is not elidable, neither will the entire lo-hi sequence.
                                    // Go to next index 2.
#ifdef DEBUG
                                    cout << " Case 2a: " << boardToString(_board, lo, idx1 - 1) << " + ";
                                    cout << boardToString(_board, idx1, idx1) << " + ";
                                    cout << boardToString(_board, idx1 + 1, hi - 1) + " + ";
                                    cout << boardToString(_board, hi, hi) << endl;
                                    cout << "  No solution for " << boardToString(_board, idx1 + 1, hi - 1) << endl;
#endif
                                    continue;
                                }
                        
                                // After removing the sequence in-between, how many balls do I need
                                // to erase the extremes of the same color?
                                numColorsToAdd = 3 - min(3, _board[idx1].n + hiGroup.n); 
                            } else {
                                // lo < idx1 < idx2 < hi
                                // Case 2b: combine 3 groups
                                //  (www)-X(yyy)X(zzz)X
                                
                                Group g1 = _board[idx1];
                                Group g2 = _board[idx2];
                                Group g3 = _board[hi];
                                bool isCase2b1 = g1.n == 1 && g2.n == 1 && g3.n == 1; // case 2b1: (www)-1X(yyy)1X(zzz)1X
                                bool isCase2b2 = g1.n == 2 && g2.n == 1 && g3.n == 1; // case 2b2: (www)-2X(yyy)1X(zzz)1X
                                bool isCase2b3 = g1.n == 1 && g2.n == 1 && g3.n == 2; // case 2b3: (www)-1X(yyy)1X(zzz)2X    
                                bool canSolve = isCase2b1 || isCase2b2 || isCase2b3;
                                if (!canSolve) {
                                    // Any of the remaining 5 dispositions: case already covered in 2a. Go to next index 2.
                                    continue;
                                }

                                solutionsMidLeft = solutionsMidLeft = mat[idx1 + 1][idx2 - 1];
                                solutionsMidRight = solutionsMidRight = mat[idx2 + 1][hi - 1];

                                canSolve = !(solutionsMidLeft.empty() || solutionsMidRight.empty());
                                if (!canSolve) {
                                    // Go to next index 2.
#ifdef DEBUG
                                    cout << " Case 2b: ";
                                    cout << boardToString(_board, lo, idx1 - 1) << " + ";
                                    cout << boardToString(_board, idx1, idx1) << " + ";
                                    cout << boardToString(_board, idx1 + 1, idx2 - 1) + " + ";
                                    cout << boardToString(_board, idx2, idx2) + " + ";
                                    cout << boardToString(_board, idx2 + 1, hi - 1) + " + ";
                                    cout << boardToString(_board, hi, hi) << endl;
                                    cout << "  No solution for some of the middle sequences" << endl;
#endif
                                    continue;
                                }
                            }

                            for (vector<int> solBefore : solutionsBeforeMatch1) {
                                for (vector<int> solMidLeft : solutionsMidLeft) {
                                    for (vector<int> solMidRight : solutionsMidRight) {
                                        vector<int> sol(5, 0);
                                        for (int k = 0; k < 5; ++k) {
                                            sol[k] += solMidRight[k];
                                            sol[k] += solBefore[k];
                                            sol[k] += solMidLeft[k];
                                        }
                                        sol[hiGroup.c] += numColorsToAdd;
                                        if (validateSolution(sol, _hand)) {
                                            mat[lo][hi].insert(sol);
                                        }
                                    }
                                }
                            }

#ifdef DEBUG
                            cout << " Case 2b: idx1=" << idx1 << ", idx2=" << idx2 << endl;
                            cout << "  Valid solutions:" << endl;
                            cout << "   R Y B G W" << endl;
                            for (vector<int> sol : mat[lo][hi]) {
                                cout << "   " << solutionToString(sol) << endl;
                            }
#endif
                        }
                    }
                }
            }
        }

#ifdef DEBUG
        cout << endl;
#endif

        // After filling the matrix, the top-right cell contains the solution
        // for the entire sequence.
        if (mat[0][_board.size() - 1].empty()) {
#ifdef DEBUG
            cout << "No solutions" << endl;
#endif
            return -1;
        }

#ifdef DEBUG
        cout << "--- Solutions ---" << endl;
        cout << " R Y B G W   tot" << endl;
#endif
        int res = numeric_limits<int>::max();
        for (vector<int> sol : mat[0][_board.size() - 1]) {
            // Count the value of each solution;
            int solValue = accumulate(sol.begin(), sol.end(), 0);
      
#ifdef DEBUG
            cout << " " << solutionToString(sol) << "   " << solValue << endl;
#endif

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

