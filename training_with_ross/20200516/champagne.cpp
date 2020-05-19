#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <utility>
#include <string>
#include <iostream>
#include <tuple>

using namespace std;

struct Glass {
  // Glass() : row(-1), pos(-1), flow(0), level(0.0) {}
  Glass(int row_, int pos_) : row(row_), pos(pos_), flow(0.0), level(0.0) {}
  
  int row;
  int pos;
  double flow;
  double level;
  
  // bool operator<(Glass const & otherGlass) const {
  //   return level < otherGlass.level;
  // }

  string toString() {
    string s = "[r" + to_string(row) + ", ";
    s += "p" + to_string(pos) + ", ";
    s += "f" + to_string(flow) + ", ";
    s += to_string(static_cast<int>(level * 100))  + "% full]";
    return s;
  }
};

class Solution {
public:
  double champagneTower(int poured, int query_row, int query_glass) {
    if (poured == 0) {
      return 0.0;
    }
    
    Glass* firstGlass = new Glass(0, 0);
    firstGlass->flow = 1.0;
    
    map<pair<int, int>, Glass*> allGlasses;
    allGlasses[make_pair(0, 0)] = firstGlass;

    auto cmp = [](Glass* g1, Glass* g2) {
        // The glass with top priority is the next one to fill up.
        // If the glass is empty up to "level" and receives champagne with
        // flow "flow", it will take level/flow seconds to fill up.
        // The glass with lowest time has the top priority. Since priority
        // queues assign the highest priority to high values, we invert the
        // relationship.
        return (1.0 - g1->level) / g1->flow > (1.0 - g2->level) / g2->flow;
    };
    priority_queue<Glass*, vector<Glass*>, decltype(cmp)> pq1(cmp);
    priority_queue<Glass*, vector<Glass*>, decltype(cmp)> pq2(cmp);
    vector<priority_queue<Glass*, vector<Glass*>, decltype(cmp)>> pqs{pq1, pq2};
    pqs[0].push(firstGlass);

    // int numFlows = 1;
    int fromPq = 0;
    int toPq = 1;
    
    for (int afterPoured = 1; afterPoured <= poured; ++afterPoured) {
      // Assume we pour 1 glass in 1 second.
      double remainingTime = 1.0;
      
      // DEBUG
      cout << "Start to pour glass " << afterPoured << endl;

      while (remainingTime > 0.0) {
        priority_queue<Glass*, vector<Glass*>, decltype(cmp)>& pq = pqs[fromPq];
        
        // The "front" glass is the one that will fill up first.
        double timeToNextFull = (1.0 - pq.top()->level) / pq.top()->flow;
        double pouringTime = min(remainingTime, timeToNextFull);
        remainingTime -= pouringTime;
        
        // DEBUG
        cout << "Next to fill: " << pq.top()->toString() << endl;
        cout << " timeToNextFull: " << timeToNextFull << endl;
        cout << " remainingTime:  " << remainingTime << endl;
        cout << " pouringTime:    " << pouringTime << endl;
        
        // List of glasses that will have more flows at the next iteration.
        list<tuple<int, int, double>> addFlowTo;

        // Update all the glasses transferring them from one queue to the other.
        // Note that no glass can overflow, for how we chose the flow to pour.
        while (!pq.empty()) {
          Glass* glass = pq.top();
          pq.pop();
          
          // DEBUG
          cout << " -> pour into " << glass->toString() << endl;
          
          // Pour into the glass.
          glass->level += pouringTime * glass->flow;
          if (glass->level < 1.0) {
            // Not full yet: just insert it into the other queue.
            pqs[toPq].push(glass);
            
            // DEBUG
            cout << "    not full" << endl;
          } else {
            // This glass is full (set to 1 to overcome precision issues).
            glass->level = 1.0;

            if (glass->row == 99 && glass->pos == 99) {
                cout << "=====================================" << endl;
                cout << afterPoured << endl;
                return 0.0;
            }
            
            // What children will this glass pour into?
            int childrenRow = glass->row + 1;
            int leftChildPos = glass->pos;
            int rightChildPos = glass->pos + 1;
            
            addFlowTo.push_back(make_tuple(childrenRow, leftChildPos, glass->flow / 2));
            addFlowTo.push_back(make_tuple(childrenRow, rightChildPos, glass->flow / 2));
            
            // Update the total amount of flow:
            // numFlows += (- glass->flows + 2);
            
            // DEBUG
            cout << "    full" << endl;
            cout << "    left child:  (" << childrenRow << ", " << leftChildPos << ") will receive " << glass->flow / 2 << endl;
            cout << "    right child: (" << childrenRow << ", " << rightChildPos << ") will receive " << glass->flow / 2 << endl;
            // cout << "    numFlows: " << numFlows << endl;
          }
        } // empty fromPQ
                  
        // DEBUG
        cout << " Add children" << endl;

        // Add the flows to the glasses that will start receiving from next round.
        for (list<tuple<int, int, double>>::iterator aftIt = addFlowTo.begin();
             aftIt != addFlowTo.end();
             ++aftIt) {
          int r = get<0>(*aftIt);
          int p = get<1>(*aftIt);
          double f = get<2>(*aftIt);

          // DEBUG
          cout << " - child (" << r << ", " << p << ", " << f << ")";

          Glass* child;
          map<pair<int, int>, Glass*>::iterator mIt = allGlasses.find(make_pair(r, p));
          if (mIt != allGlasses.end()) {
            // This glass already exists: update it.
            child = mIt->second;

            // DEBUG
            cout << " exists";
          } else {
            // New glass: it is empty and is not in the queue.
            child = new Glass(r, p);
            allGlasses[make_pair(r, p)] = child;
            pqs[toPq].push(child);

            // DEBUG
            cout << " new";
          }
          child->flow += f;

          // DEBUG
          cout << endl;
        }
        
        // DEBUG
        cout << "pqs[0].size() = " << pqs[0].size() << endl;
        cout << "pqs[1].size() = " << pqs[1].size() << endl;


        // Swap queues.
        fromPq = 1 - fromPq;
        toPq = 1 - toPq;
        
        // DEBUG
        cout << endl;
      } // while pouringTime
    }
    
    map<pair<int, int>, Glass*>::iterator iit = allGlasses.find(make_pair(query_row, query_glass));
    if (iit != allGlasses.end()) {
      return iit->second->level;
    }
    return 0.0;
  }
};

int main() {
    Solution* sol = new Solution(); 
    double res = sol->champagneTower(1000000000, 3, 0);
    cout << "res " << res << endl;
    return 0;
}

