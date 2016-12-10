#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                           //I used std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>         WordQueue;
typedef ics::ArraySet<std::string>           FollowSet;
typedef ics::pair<WordQueue,FollowSet>       CorpusEntry;
typedef ics::ArrayPriorityQueue<CorpusEntry> CorpusPQ;     //Convenient to supply gt at construction
typedef ics::ArrayMap<WordQueue,FollowSet>   Corpus;


int min_max(Corpus & cc, int m){
    //max == 1
    if(m == 1) {
        int max = 0;
        for (auto i:cc) {
            if (i.second.size() > max) {
                max = i.second.size();
            }
        }
        return max;
    }
    //min == 0
    if(m == 0) {
        int min = 100;
        for (auto i:cc) {
            if (i.second.size() < min) {
                min = i.second.size();
            }
        }
        return min;
    }
}

//Return a random word in the words set (use in produce_text)
std::string random_in_set(const FollowSet& words) {
  int index = ics::rand_range(1, words.size());
  int i = 0;
  for (const std::string& s : words)
    if (++i == index)
      return s;
  return "?";
}


//Read an open file of lines of words (separated by spaces) and return a
//  Corpus (Map) of each sequence (Queue) of os (Order-Statistic) words
//  associated with the Set of all words that follow them somewhere in the
//  file.
Corpus read_corpus(int os, std::ifstream &file) {
    Corpus newCorps;
    std::vector<std::string> tempArray;
    std::string temp;
    std::string line;
    while(getline(file, line)) {
        temp += line;
        temp.push_back(' ');
    }
    tempArray = ics::split(temp, " ");
    for (int i = 0; i < tempArray.size(); i++) {
        WordQueue strQ;
        if (strQ.size() < os){
            for (int q = 0; strQ.size() < os && (i+q) < tempArray.size(); q++){
                strQ.enqueue(tempArray[i + q]);
            }
        }
        if (strQ.size() == os){
            if ((i+os) < tempArray.size()) {
                newCorps[strQ].insert(tempArray[i + os]);
            }
        }
    }
    file.close();
    return newCorps;

}


//Print "Corpus" and all entries in the Corpus, in lexical alphabetical order
//  (with the minimum and maximum set sizes at the end).
//Use a "can be followed by any of" to separate the key word from the Set of words
//  that can follow it.

//One queue is lexically greater than another, if its first value is smaller; or if
//  its first value is the same and its second value is smaller; or if its first
//  and second values are the same and its third value is smaller...
//If any of its values is greater than the corresponding value in the other queue,
//  the first queue is not greater.
//Note that the queues sizes are the same: each stores Order-Statistic words
//Important: Use iterators for examining the queue values: DO NOT CALL DEQUEUE.

bool queue_gt(const CorpusEntry& a, const CorpusEntry& b) {
    for (auto f = a.first.begin(),s = b.first.begin(); f != a.first.end(); ++f,++s) {
        if (*f < *s){
            return true;
        }
        else if (*f > *s){
            return false;
        }
    }
}

void print_corpus(const Corpus& corpus) {
    ics::ArrayPriorityQueue<CorpusEntry, queue_gt> PQ(corpus);

    for (auto v: PQ) {
        std::cout << "  " << v.first << " -> " << v.second << std::endl;
    }
}


//Return a Queue of words, starting with those in start and including count more
//  randomly selected words using corpus to decide which word comes next.
//If there is no word that follows the previous ones, put "None" into the queue
//  and return immediately this list (whose size is <= start.size() + count).
WordQueue produce_text(const Corpus& corpus, const WordQueue& start, int count) {
    WordQueue newQueue(start);
    WordQueue changingKey(start);
    while (count != 0) {
        if (corpus.has_key(changingKey)) {
            std::string value = random_in_set(corpus[changingKey]);
            changingKey.dequeue();
            changingKey.enqueue(value);
            newQueue.enqueue(value);
        }
        else if (!corpus.has_key(changingKey)) {
            newQueue.enqueue("None");
            return newQueue;
        } count --;
    }
    return newQueue;
}



//Prompt the user for (a) the order statistic and (b) the file storing the text.
//Read the text as a Corpus and print it appropriately.
//Prompt the user for order statistic words from the text.
//Prompt the user for number of random words to generate
//Call the above functions to solve the problem, and print the appropriate information
int main() {
  try {

      std::ifstream inputFile;
      int orderS = 2;
      ics::prompt_int("Enter an order statistic", orderS);

      ics::safe_open(inputFile, "Enter a file to process", "wginput1.txt");
      std::cout << std::endl;

      auto corp = read_corpus(orderS,inputFile);
      std::cout << "Corpus has " << corp.size() << " Entries"<< std::endl;
      print_corpus(corp);
      std::cout << "Corpus has " << corp.size() << " Entries"<< std::endl;
      std::cout << "min/max = " << min_max(corp,0) << "/" << min_max(corp,1);

      std::cout << std::endl;
      std::cout << std::endl << "Enter " << orderS << " words to start with" << std::endl;
      WordQueue queUp;
      for (int i = 1; i <= orderS;i++){
          std::string current;
          std::cout << "Enter word "<< i << ": ";
          std::cin >> current;
          queUp.enqueue(current);
      }
      int number_of_words;
      std::cout << "Enter # of words to generate: ";
      std::cin >> number_of_words;
      std::cout << "Random text = " << produce_text(corp,queUp,number_of_words) << std::endl;




  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
