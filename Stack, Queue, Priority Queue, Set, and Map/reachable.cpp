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


typedef ics::ArraySet<std::string>          NodeSet;
typedef ics::pair<std::string,NodeSet>      GraphEntry;

bool graph_entry_gt (const GraphEntry& a, const GraphEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<GraphEntry,graph_entry_gt> GraphPQ;
typedef ics::ArrayMap<std::string,NodeSet>  Graph;


//Read an open file of edges (node names separated by semicolons, with an
//  edge going from the first node name to the second node name) and return a
//  Graph (Map) of each node name associated with the Set of all node names to
//  which there is an edge from the key node name.
Graph read_graph(std::ifstream &file) {
    Graph graph;
    for(std::string line; getline(file, line);) {
        std::vector<std::string> array;
        array = ics::split(line, ";");
        graph[array[0]].insert(array[1]);
    } file.close();
    return graph;

}


//Print a label and all the entries in the Graph in alphabetical order
//  (by source node).
//Use a "->" to separate the source node name from the Set of destination
//  node names to which it has an edge.
void print_graph(const Graph& graph) {
    GraphPQ PQ(graph);
    std::cout << "Graph: source -> set[destination nodes]" << std::endl;
    for (auto i : PQ) {
        std::cout << "  " << i.first << " -> " << i.second << std::endl;
    }

}


//Return the Set of node names reaching in the Graph starting at the
//  specified (start) node.
//Use a local Set and a Queue to respectively store the reachable nodes and
//  the nodes that are being explored.
NodeSet reachable(const Graph& graph, std::string start) {
    NodeSet set;
    std::vector<std::string> searching;
    searching.push_back(start);
    while(!searching.empty()) {
        auto first = searching[0];
        searching.erase(searching.begin() + 0);
        set.insert(first);
        if (graph.has_key(first)) {
            auto value = graph[first];
            for (auto i : value) {
                if (!set.contains(i)) {
                    searching.push_back(i);
                }
            }
        }
    } return set;
}





//Prompt the user for a file, create a graph from its edges, print the graph,
//  and then repeatedly (until the user enters "quit") prompt the user for a
//  starting node name and then either print an error (if that the node name
//  is not a source node in the graph) or print the Set of node names
//  reachable from it by using the edges in the Graph.
int main() {
    try {
        std::ifstream inputFile;
        ics::safe_open(inputFile, "Enter a graph file's name", "graph1.txt");
        std::cout << std::endl;
        Graph map = read_graph(inputFile);
        print_graph(map);
        std::cout << std::endl;
        std::string response;

        while (true) {
            std::cout << "Enter a starting node's name: ";
            std::cin >> response;
            if (response == "quit") {
                break;
            } else if (map.has_key(response)) {
                std::cout << "Reachable from node name " << response << " = " << reachable(map, response);
            } else {
                std::cout << "  " << response << " is not a source node name in the graph";
            } std::cout << std::endl << std::endl;
        }
    } catch (ics::IcsError& e) {
        std::cout << e.what() << std::endl;
    }

  return 0;
}
