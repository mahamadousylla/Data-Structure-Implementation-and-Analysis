#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>              CandidateQueue;
typedef ics::ArraySet<std::string>                CandidateSet;
typedef ics::ArrayMap<std::string,int>            CandidateTally;

bool entry_gt(const ics::pair<std::string, CandidateQueue> & f, const ics::pair<std::string, CandidateQueue> & s) {
    return f.first < s.first;
}

bool gt_remaining(const ics::pair<std::string, int> & f, const ics::pair<std::string, int> & s){
    return f.second < s.second;
}

bool gt_alpha(const ics::pair<std::string, int> & f, const ics::pair<std::string, int> & s) {
    return f.first < s.first;
}

bool num_alpha(const ics::pair<std::string, int> & f, const ics::pair<std::string, int> & s) {
    if (f.second == s.second) {
        return f.first < s.first;
    } else {
        return f.second > s.second;
    }
}
typedef ics::ArrayMap<std::string,CandidateQueue> Preferences;
typedef ics::pair<std::string,CandidateQueue>     PreferencesEntry;


typedef ics::ArrayPriorityQueue<PreferencesEntry, entry_gt> PreferencesEntryPQ; //Must supply gt at construction

typedef ics::pair<std::string,int>                TallyEntry;
typedef ics::ArrayPriorityQueue<TallyEntry>       TallyEntryPQ;



//Read an open file stating voter preferences (each line is (a) a voter
//  followed by (b) all the candidates the voter would vote for, in
//  preference order (from most to least preferred candidate, separated
//  by semicolons), and return a Map of preferences: a Map whose keys are
//  voter names and whose values are a queue of candidate preferences.
Preferences read_voter_preferences(std::ifstream &file) {
    Preferences newMap;
    for (std::string line; getline(file, line);) {
        auto tempArray = ics::split(line, ";");
        for (int i = 1; i < tempArray.size(); i++) {
            newMap[tempArray[0]].enqueue(tempArray[i]);
        }
    } file.close();
    return newMap;
}


//Print a label and all the entries in the preferences Map, in alphabetical
//  order according to the voter.
//Use a "->" to separate the voter name from the Queue of candidates.
void print_voter_preferences(const Preferences& preferences) {
    std::cout << std::endl << "Voter -> queue[Preferences]" << std::endl;
    PreferencesEntryPQ p(preferences);
    for (auto i : p) {
        std::cout << "  " << i.first << " -> " << i.second << std::endl;
    }
}


//Print the message followed by all the entries in the CandidateTally, in
//  the order specified by has_higher_priority: i is printed before j, if
//  has_higher_priority(i,j) returns true: sometimes alphabetically by candidate,
//  other times by decreasing votes for the candidate.
//Use a "->" to separate the candidat name from the number of votes they
//  received.
void print_tally(std::string message, const CandidateTally& tally, bool (*has_higher_priority)(const TallyEntry& i,const TallyEntry& j)) {
    std::cout << message << std::endl;
    ics::ArrayPriorityQueue<ics::pair<std::string, int>> alpha(tally, has_higher_priority);
    for (auto i : alpha) {
        std::cout << "  " << i.first << " -> " << i.second << std::endl;
    }
}


//Return the CandidateTally: a Map of candidates (as keys) and the number of
//  votes they received, based on the unchanging Preferences (read from the
//  file) and the candidates who are currently still in the election (which changes).
//Every possible candidate should appear as a key in the resulting tally.
//Each voter should tally one vote: for their highest-ranked candidate who is
//  still in the the election.
CandidateTally evaluate_ballot(const Preferences& preferences, const CandidateSet& candidates) {
    CandidateTally tally;
    for (auto j : preferences) {
        for (auto k : j.second) {
            if (candidates.contains(k)) {
                tally[k] += 1;
                break;
            }
        }
    } return tally;
}


//Return the Set of candidates who are still in the election, based on the
//  tally of votes: compute the minimum number of votes and return a Set of
//  all candidates receiving more than that minimum; if all candidates
//  receive the same number of votes (that would be the minimum), the empty
//  Set is returned.
CandidateSet remaining_candidates(const CandidateTally& tally) {
    ics::ArrayPriorityQueue<ics::pair<std::string, int>, gt_remaining> cand(tally);
    CandidateSet return_set;
    auto least = cand.peek().second;
    for (auto i: tally){
        if (i.second != least){
            return_set.insert(i.first);
        }
    } return return_set;
}


//Prompt the user for a file, create a voter preference Map, and print it.
//Determine the Set of all the candidates in the election, from this Map.
//Repeatedly evaluate the ballot based on the candidates (still) in the
//  election, printing the vote count (tally) two ways: with the candidates
//  (a) shown alphabetically increasing and (b) shown with the vote count
//  decreasing (candidates with equal vote counts are shown alphabetically
//  increasing); from this tally, compute which candidates remain in the
//  election: all candidates receiving more than the minimum number of votes;
//  continue this process until there are less than 2 candidates.
//Print the final result: there may 1 candidate left (the winner) or 0 left
//   (no winner).
int main() {
  try {

      std::ifstream inputFile;
      ics::safe_open(inputFile, "Enter a voter preferences file's name", "votepref1.txt");
      Preferences prefMap = read_voter_preferences(inputFile);
      CandidateSet initialSet;
      for (auto i : prefMap) {
          initialSet.insert_all(i.second);
      }
      print_voter_preferences(prefMap);
      int cycles = 1;
      while (initialSet.size() > 1) {


          CandidateTally evaluated = evaluate_ballot(prefMap, initialSet);
          std::string theSet = "set[";
          for (auto i = initialSet.begin(); i != initialSet.end(); ++i) {
              theSet += *i + ",";
          }

          theSet.pop_back();
          theSet += "]";
          auto alphaMessage = "Vote count on ballot #" + std::to_string(cycles) + " with candidates (alphabetically ordered); remaining candidates = " + theSet;
          std::cout << std::endl;
          print_tally(alphaMessage, evaluated, gt_alpha);

          auto numMessage = "Vote count on ballot #" + std::to_string(cycles) + " with candidates (numerically ordered); remaining candidates = " + theSet;
          std::cout << std::endl;
          print_tally(numMessage, evaluated, num_alpha);

          initialSet = remaining_candidates(evaluated);
          cycles++;
          if (initialSet.size() == 1) {
              for (auto i : initialSet) {
                  std::cout << std::endl << "Winner is " << i << std::endl;
              }
          } else if (initialSet.size() == 0) {
              std::cout << std::endl << "There is no winner" << std::endl;
          }
      }

  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
