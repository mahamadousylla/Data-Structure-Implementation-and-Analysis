#include <string>
#include <iostream>
#include <fstream>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "hash_graph.hpp"
#include "dijkstra.hpp"



std::string get_node_in_graph(const ics::DistGraph& g, std::string prompt, bool allow_QUIT) {
  std::string node;
  for(;;) {
    node = ics::prompt_string(prompt + " (must be in graph" + (allow_QUIT ? " or QUIT" : "") + ")");
    if ((allow_QUIT && node == "QUIT") || g.has_node(node))
      break;
  }
  return node;
}


int main() {
  try {
     std::ifstream inputFile;
     ics::DistGraph hashGraph;

     ics::safe_open(inputFile, "Enter graph file name", "flightcost.txt");

     hashGraph.load(inputFile, ";");
     std::cout << hashGraph;

     std::string start_node = get_node_in_graph(hashGraph, "Enter start node", false);

     ics::CostMap shortest_path_map = extended_dijkstra(hashGraph, start_node);
     std::cout << shortest_path_map << std::endl << std::endl;

     bool keepgoing = true;
     while (keepgoing) {
        std::string stop_node = get_node_in_graph(hashGraph, "Enter stop node", true);
        if (stop_node == "QUIT") {
           break;
        } else {
           ics::ArrayQueue<std::string> queue = recover_path(shortest_path_map, stop_node);
           int cost = shortest_path_map[stop_node].cost;
           std::cout << "Cost is " << cost << "; path is " << queue << std::endl << std::endl;
        }
     }


  } catch (ics::IcsError& e) {
      std::cout << e.what() << std::endl;
  }

  return 0;
}
