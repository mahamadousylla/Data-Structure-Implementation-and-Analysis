#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArraySet<std::string>                     States;
typedef ics::ArrayQueue<std::string>                   InputsQueue;
typedef ics::ArrayMap<std::string,States>              InputStatesMap;

typedef ics::ArrayMap<std::string,InputStatesMap>       NDFA;
typedef ics::pair<std::string,InputStatesMap>           NDFAEntry;

bool gt_NDFAEntry (const NDFAEntry& a, const NDFAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<NDFAEntry,gt_NDFAEntry> NDFAPQ;

typedef ics::pair<std::string,States>                   Transitions;
typedef ics::ArrayQueue<Transitions>                    TransitionsQueue;


//Read an open file describing the non-deterministic finite automaton (each
//  line starts with a state name followed by pairs of transitions from that
//  state: (input followed by a new state, all separated by semicolons), and
//  return a Map whose keys are states and whose associated values are another
//  Map with each input in that state (keys) and the resulting set of states it
//  can lead to.
const NDFA read_ndfa(std::ifstream &file) {
    NDFA returnMap;
    InputStatesMap empty;
    for (std::string line; getline(file, line);) {
        States set;
        std::vector<std::string> splitArray;
        splitArray = ics::split(line, ";");
        for (int i = 1; i < splitArray.size(); i += 2) {
            set.insert(splitArray[i + 1]);
            returnMap[splitArray[0]][splitArray[i]] = set;
        } if (splitArray.size() <= 1) {
            returnMap[splitArray[0]] = empty;
        }
    } file.close();
    return returnMap;
}


//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.
void print_ndfa(const NDFA& ndfa) {
    std::cout << "The Non-Deterministic Finite Automaton's Description" << std::endl;
    NDFAPQ sortedPQ(ndfa);
    for (auto i : sortedPQ) {
        std::cout << "  " << i.first << " transitions: " << i.second << std::endl;
    }
}


//Return a queue of the calculated transition pairs, based on the non-deterministic
//  finite automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, set of new states.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to any state in the non-deterministic finite
//  automaton), ignore it.
TransitionsQueue process(const NDFA& ndfa, std::string state, const InputsQueue& inputs) {
    TransitionsQueue processed;
    States set;
    set.insert(state);
    Transitions first("", set);
    processed.enqueue(first);
    States next_state(set);
    for (auto i : inputs) {
        set.clear();
        for (auto j : next_state) {
            if (ndfa[j].has_key(i)) {
                set.insert_all(ndfa[j][i]);
            }
        } Transitions pair(i, set);
        processed.enqueue(pair);
        next_state = set;
    } return processed;
}


//Print a TransitionsQueue (the result of calling process) in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new states indented on subsequent lines; on the last line, print
//  the Stop state.
void interpret(TransitionsQueue& tq) {  //or TransitionsQueue or TransitionsQueue&&
    States end;
    for (auto i : tq) {
        if (i.first != "") {
            std::cout << "  Input = " << i.first << "; new states = " << i.second << std::endl;
        }
        end = i.second;
    } std::cout << "Stop state(s) = " << end << std::endl;
}



//Prompt the user for a file, create a finite automaton Map, and print it.
//Prompt the user for a file containing any number of simulation descriptions
//  for the finite automaton to process, one description per line; each
//  description contains a start state followed by its inputs, all separated by
//  semicolons.
//Repeatedly read a description, print that description, put each input in a
//  Queue, process the Queue and print the results in a nice form.
int main() {
  try {
      std::ifstream inputFile;
      ics::safe_open(inputFile, "Enter a non-deterministic finite automaton's file", "ndfaendin01.txt");
      NDFA ndfa = read_ndfa(inputFile);
      print_ndfa(ndfa);
      std::cout << std::endl;
      std::ifstream secondFile;
      ics::safe_open(secondFile, "Enter the name of a file with the start-states and input", "ndfainputendin01.txt");

      for (std::string line; getline(secondFile, line);) {
          std::vector<std::string> newVector;
          InputsQueue inputs;
          std::string start;
          newVector = ics::split(line, ";");
          start = newVector[0];
          for (int i = 1; i < newVector.size(); i++) {
              inputs.enqueue(newVector[i]);
          }
          std::cout << std::endl << "Starting new simulation with description: " << line << std::endl;
          TransitionsQueue processed = process(ndfa, start, inputs);
          States startStates;
          startStates.insert(start);
          std::cout << "Start state = " << startStates << std::endl;
          interpret(processed);
      }


  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
