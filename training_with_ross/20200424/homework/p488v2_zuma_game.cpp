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
        // Fill the hand.
        for (char& c: hand) {
            ++_hand[charToColor(c)];
        }
        printHand(_hand);

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
        printBoard(_board);

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

                // Match the rightmost group with all the groups in the current
                // subsequence with the same color.
                Color& currColor = hiGroup.c;
                vector<int>& currColorIndices = colorIndices[currColor];
                for (int cIdx : currColorIndices) {
#ifdef DEBUG
                    cout << "  cIdx: " << cIdx;
#endif
                    if (cIdx < lo) {
                        // This group is outside the subsequence.
#ifdef DEBUG
                        cout << " -> smaller than lo=" << lo << endl;
#endif
                        continue;
                    }

                    if (cIdx > hi) {
                        // Since indices are in order, if we go beyond the index of the
                        // righmost group, all the indices from now on will be out of
                        // the current subsequence.
#ifdef DEBUG
                        cout << " -> larger than hi=" << hi << endl;
#endif
                        break;
                    }

                    if (cIdx == hi) {
                        // Case "erase subsequence before last group + erase last group".
#ifdef DEBUG
                        cout << " -> equals hi=" << hi << endl;
                        cout << "   Solutions" << endl;
                        cout << "    R Y B G W" << endl;
#endif
                        for (vector<int> sol : mat[lo][hi - 1]) {
                            // For each solution of the subsequence before the last group...
                            sol[hiGroup.c] += 3 - hiGroup.n;
#ifdef DEBUG
                            cout << "    ";
                            for (auto& v : sol) {
                                cout << v << " ";
                            }
#endif
                            if (validateSolution(sol, _hand)) {
                                mat[hi][lo].insert(sol);
#ifdef DEBUG
                                cout << " -> V";
#endif
                            }
#ifdef DEBUG
                            cout << endl;
#endif
                        }
                    } else {
                        // Case "erase the last group using a group of the same color in
                        // the middle of the current subsequence".
#ifdef DEBUG
                        cout << " -> mid-match" << endl;
                        cout << "   Solutions" << endl;
                        cout << "    R Y B G W" << endl;
#endif
                        set<vector<int>>& solutionsBeforeLeftMatch = mat[lo][cIdx - 1];
                        
                        // Two groups with the same color cannot be consecutive, otherwise
                        // they would already be the same group.
                        set<vector<int>>& solutionsBetweenMatches = mat[cIdx + 1][hi - 1];
                        
                        // After removing the sequence in-between, how many balls do I need
                        // to erase the extremes of the same color?
                        int numCurrColorToAdd = 3 - min(3, _board[cIdx].n + hiGroup.n); 
                        
                        // Put together each "between" solution with each "before" solution.
                        // The "between" solution is guaranteed to exist, but if we are matching
                        // the first group of the subsequence (at lo), there will be nothing
                        // to the left of the match.
                        // To solve this, we merge "before" solutions to "between" and not viceversa.
                        for (vector<int> solBetween : solutionsBetweenMatches) {
                            for (vector<int> solBefore : solutionsBeforeLeftMatch) {
                                for (int k = 0; k < solBetween.size(); ++k) {
                                    solBetween[k] += solBefore[k];
                                }
                            }

                            solBetween[hiGroup.c] += numCurrColorToAdd;
#ifdef DEBUG
                            cout << "    ";
                            for (auto& v : solBetween) {
                                cout << v << " ";
                            }
#endif

                            if (validateSolution(solBetween, _hand)) {
                                mat[hi][lo].insert(solBetween);
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

