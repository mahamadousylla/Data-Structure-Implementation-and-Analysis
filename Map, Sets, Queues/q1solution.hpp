#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <math.h>            /* for atan2 and sqrt */
#include "ics46goody.hpp"
#include "ics_exceptions.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_map.hpp"


class Point {
public:
  Point() : x(0), y(0) {} // Needed for pair
  Point(int x_val, int y_val) : x(x_val), y(y_val) {}
  friend bool operator == (const Point& p1, const Point& p2) {
    return p1.x == p2.x && p1.y == p2.y;
  }
  friend std::ostream& operator << (std::ostream& outs, const Point& p) {
    outs << "(" << p.x << "," << p.y << ")";
    return outs;
  }

  int x;
  int y;
};


//Helper Functions (you decide what is useful)
//Hint: I used helpers for sort_yo, sort_angle, points, and first_quad

bool sort_ascending(const ics::pair<int, Point>& f, const ics::pair<int, Point>& s) {
    if (f.second.y == s.second.y) {
        return f.first > s.first;
    } else {
        return f.second.y < s.second.y;
    }
}

bool sort_descending(const ics::pair<int, Point>& f, const ics::pair<int, Point> & s) {
    auto first = sqrt( pow((f.second.x - 0), 2) + pow((f.second.y - 0), 2) );
    auto second = sqrt( pow((s.second.x - 0), 2) + pow((s.second.y - 0), 2) );
    return first > second;
}

bool sortasc(const ics::pair<int, Point> &f, const ics::pair<int, Point> & s) {
    return f.first < s.first;
}

bool sortatan(const ics::pair<int, Point> &f, const ics::pair<int, Point> & s) {
    auto result1 = atan2(f.second.y, f.second.x);
    auto result2 = atan2(s.second.y, s.second.x);
    return result1 < result2;
}

//Problem #1a and #1b
template<class KEY,class T>
void swap (ics::ArrayMap<KEY,T>& m, KEY key1, KEY key2) {
    m[key1] = m.put(key2, m[key1]);
//    auto temp = m[key1];
//    m[key1] = m[key2];
//    m[key2] = temp;
}


template<class KEY,class T>
void values_set_to_queue (const ics::ArrayMap<KEY,ics::ArraySet<T>>& m1,
                          ics::ArrayMap<KEY,ics::ArrayQueue<T>>&     m2) {
    for (auto i : m1) {
        m2[i.first].enqueue_all(i.second);
    }
}


//Problem #2a, #2b, #2c, and #2d
ics::ArrayQueue<ics::pair<int,Point>> sort_yo (const ics::ArrayMap<int,Point>& m) {
    ics::ArrayPriorityQueue<ics::pair<int, Point>,sort_ascending> PQ(m);
    ics::ArrayQueue<ics::pair<int,Point>> newQueue;
//    std::cout << PQ;
    for (auto i : PQ) {
        newQueue.enqueue(i);
    } return newQueue;
}


ics::ArrayQueue<Point> sort_distance (const ics::ArrayMap<int,Point>& m) {
    ics::ArrayPriorityQueue<ics::pair<int, Point>, sort_descending> PQ(m);
    ics::ArrayQueue<Point> newQueue;
    for (auto i : PQ) {
        newQueue.enqueue(i.second);
    } return newQueue;
}


ics::ArrayQueue<Point> points (const ics::ArrayMap<int,Point>& m) {
    ics::ArrayPriorityQueue<ics::pair<int, Point>, sortasc> PQ(m);
    ics::ArrayQueue<Point> newQueue;
    for (auto i : PQ) {
        newQueue.enqueue(i.second);
    } return newQueue;
}


ics::ArrayQueue<ics::pair<int,double>> angles (const ics::ArrayMap<int,Point>& m) {
    ics::ArrayPriorityQueue<ics::pair<int, Point>, sortatan> PQ(m);
    ics::ArrayQueue<ics::pair<int, double>> newQueue;
    for (auto i : PQ) {
        auto result1 = atan2(i.second.y, i.second.x);
        ics::pair<int, double> value(i.first, result1);
        newQueue.enqueue(value);
    } return newQueue;
}


//Problem #3
ics::ArrayMap<char,ics::ArraySet<char>> follows(const std::string word) {
    ics::ArrayMap<char,ics::ArraySet<char>> newMap;
    for (int i = 0; i < word.size() -1; i++) {
        newMap[word[i]].insert(word[i+1]);
    } return newMap;
}


//Problem #4a and #4b
ics::ArrayMap<std::string,int> got_called(const  ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>>& calls) {
    ics::ArrayMap<std::string,int> newMap;
    for (auto i : calls) {
        for (auto j : i.second) {
            newMap[j.first] = j.second + newMap[j.first];
        }
    } return newMap;
}


ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>> invert (const ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>>& calls) {
    ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>> newMap;
    ics::ArrayMap<std::string,int> innerMap;
    for (auto i : calls) {
        for (auto j : i.second) {
            newMap[j.first][i.first] = j.second;
        }
    } return newMap;
}
#endif /* SOLUTION_HPP_ */


