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


typedef ics::ArrayQueue<std::string>                InputsQueue;
typedef ics::ArrayMap<std::string,std::string>      InputStateMap;

typedef ics::ArrayMap<std::string,InputStateMap>    FA;
typedef ics::pair<std::string,InputStateMap>        FAEntry;

bool gt_FAEntry (const FAEntry& a, const FAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<FAEntry,gt_FAEntry> FAPQ;

typedef ics::pair<std::string,std::string>          Transition;
typedef ics::ArrayQueue<Transition>                 TransitionQueue;


//Read an open file describing the finite automaton (each line starts with
//  a state name followed by pairs of transitions from that state: (input
//  followed by new state, all separated by semicolons), and return a Map
//  whose keys are states and whose associated values are another Map with
//  each input in that state (keys) and the resulting state it leads to.
const FA read_fa(std::ifstream &file) {
    FA returnMap;
    InputStateMap innerMap;
    for (std::string line; getline(file, line);) {
        std::vector<std::string> splitArray;
        splitArray = ics::split(line, ";");
        for (int i = 1; i < splitArray.size(); i += 2) {
            returnMap[splitArray[0]][splitArray[i]] = splitArray[i + 1];
        }
    } file.close();
     return returnMap;
}


//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.
void print_fa(const FA& fa) {
    std::cout << "The Finite Automaton's Description" << std::endl;
    for (auto i : fa) {
        std::cout << "  " << i.first << " transitions: " << i.second << std::endl;
    }
}


//Return a queue of the calculated transition pairs, based on the finite
//  automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, new state.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to a state in the finite
//  automaton), then the last pair in the returned queue is i,"None".
TransitionQueue process(const FA& fa, std::string state, const InputsQueue& inputs) {
    TransitionQueue returnQueue;
    Transition add("", state);
    returnQueue.enqueue(add);
    std::string nextState = state;
    for (auto i : inputs) {
        if (!fa[nextState].has_key(i)) {
            Transition add(i, "None");
            returnQueue.enqueue(add);
            return returnQueue;
        } else {
            Transition add(i, fa[nextState][i]);
            nextState = fa[nextState][i];
            returnQueue.enqueue(add);
        }

    } return returnQueue;
}


//Print a TransitionQueue (the result of calling the process function above)
// in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new state (or "illegal input: terminated", if the state is
//  "None") indented on subsequent lines; on the last line, print the Stop
//  state (which may be "None").
void interpret(TransitionQueue& tq) {  //or TransitionQueue or TransitionQueue&&
    std::string end;
    for (auto i : tq) {
        if (i.first != "" && i.second != "None") {
            std::cout << "  Input = " << i.first << "; new state = " << i.second << std::endl;
        } else if (i.second == "None") {
            std::cout << "  Input = " << i.first << "; illegal input: terminated" << std::endl;
            std::cout << "Stop state = None" << std::endl;
            return;
        }

        end = i.second;
    } std::cout << "Stop state = " << end << std::endl;
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
      ics::safe_open(inputFile, "Enter a finite automaton's file", "faparity.txt");
      FA faMap = read_fa(inputFile);
      print_fa(faMap);

      std::cout << std::endl;
      std::ifstream secondFile;
      ics::safe_open(secondFile, "Enter a start-state and input file", "fainputparity.txt");

      for (std::string line; getline(secondFile, line);) {
          std::vector<std::string> newVector;
          std::string start;
          InputsQueue inputs;
          newVector = ics::split(line, ";");
          start = newVector[0];
          for (int i = 1; i < newVector.size(); i++) {
              inputs.enqueue(newVector[i]);
          }
          std::cout << std::endl << "Starting a new simulation with description: " << line << std::endl;
          std::cout << "Start state = " << start << std::endl;
          TransitionQueue p = process(faMap, start, inputs);
          interpret(p);
      }

    }

    catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
