#ifndef DIJKSTRA_HPP_
#define DIJKSTRA_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "array_queue.hpp"
#include "array_stack.hpp"
#include "heap_priority_queue.hpp"
#include "hash_graph.hpp"


namespace ics {


class Info {
  public:
    Info() { }

    Info(std::string a_node) : node(a_node) { }

    bool operator==(const Info &rhs) const { return cost == rhs.cost && from == rhs.from; }

    bool operator!=(const Info &rhs) const { return !(*this == rhs); }

    friend std::ostream &operator<<(std::ostream &outs, const Info &i) {
      outs << "Info[" << i.node << "," << i.cost << "," << i.from << "]";
      return outs;
    }

    //Public instance variable definitions
    std::string node = "?";
    int cost = std::numeric_limits<int>::max();
    std::string from = "?";
  };


  bool gt_info(const Info &a, const Info &b) { return a.cost < b.cost; }

  typedef ics::HashGraph<int> DistGraph;
  typedef ics::HeapPriorityQueue<Info, gt_info> CostPQ;
  typedef ics::HashMap<std::string, Info, DistGraph::hash_str> CostMap;
  typedef ics::pair<std::string, Info> CostMapEntry;


//Return the final_map as specified in the lecture-node description of
//  extended Dijkstra algorithm
CostMap extended_dijkstra(const DistGraph &g, std::string start_node) {
       CostMap answer_map;
       CostMap info_map;
       auto set = g.all_nodes();
       for (auto i : set) {
          info_map.put(i.first, Info(i.first));
       }

       info_map[start_node].cost = 0;
       CostPQ info_pq;
       for (auto i : info_map) {
          info_pq.enqueue(i.second);
       }


       while (!info_map.empty()) {
            auto next = info_pq.dequeue();
            if (next.cost == 2147483647) {
               return answer_map;
            } else if (!answer_map.has_key(next.node)) {

               auto min_node = next.node;
               auto min_cost = next.cost;
               auto smallest = info_map.erase(next.node);
               answer_map.put(min_node, smallest);

               for (auto destination : set[min_node].out_nodes) {
                  if (!answer_map.has_key(destination)) {

                     auto in_edge_cost = g.edge_value(min_node, destination);
                     auto c = min_cost + in_edge_cost;
                     if (c < info_map[destination].cost || c == 2147483647) {
                        info_map[destination].cost = c;
                        info_map[destination].from = min_node;
                        info_pq.enqueue(info_map[destination]);
                     }
                  }
               }
            }
       }
       return answer_map;

}


//Return a queue whose front is the start node (implicit in answer_map) and whose
//  rear is the end node
ArrayQueue <std::string> recover_path(const CostMap &answer_map, std::string end_node) {
       ics::ArrayStack<std::string> stack;
       ics::ArrayQueue<std::string> queue;
       std::string previous = end_node;

       bool found = true;
       while (found) {
          stack.push(previous);
          previous = answer_map[previous].from;
          if (previous == "?") {
             break;
          }
       }

       while (!stack.empty()) {
          queue.enqueue(stack.pop());
       }

       return queue;
}


}

#endif /* DIJKSTRA_HPP_ */
