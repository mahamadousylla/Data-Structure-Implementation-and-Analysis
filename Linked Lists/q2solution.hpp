#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <string>
#include <iostream>
#include <fstream>


template<class T>
class LN {
  public:
    LN ()                        : next(nullptr){}
    LN (const LN<T>& ln)         : value(ln.value), next(ln.next){}
    LN (T v, LN<T>* n = nullptr) : value(v), next(n){}
    T      value;
    LN<T>* next;
};


//Simple way to print linked lists (useful for driver and debugging)
template<class T>
std::ostream& operator << (std::ostream& outs, LN<T>* l) {
  for (LN<T>* p = l; p!=nullptr; p=p->next)
    outs << p->value << "->";
  outs << "nullptr";
  return outs;
}


//Simple way to print linked lists given first node
//  (useful for driver and debugging)
template<class T>
std::ostream& operator << (std::ostream& outs, LN<T> l) {
  outs << l.value << "->";
  for (LN<T>* p = l.next; p!=nullptr; p=p->next)
    outs << p->value << "->";
  outs << "nullptr";
  return outs;
}


char relation (const std::string& s1, const std::string& s2) {
    if (s1.empty() && !s2.empty()) {
        return '<';
    } else if (!s1.empty() && s2.empty()) {
        return '>';
    } else if (s1.empty() && s2.empty()) {
        return '=';
    } else if (s1[0] < s2[0]) {
        return '<';
    } else if (s1[0] > s2[0]) {
        return '>';
    } else if (s1[0] == s2[0]) {
        return relation(s1.substr(1, s1.size()), s2.substr(1, s2.size()));
    }
}


template<class T>
void remove_ascending_i (LN<T>*& l) {
    while (l != nullptr && l->next != nullptr && l->value < l->next->value) {
        l = l->next;
    } if (l == nullptr || l->next == nullptr) {
        return;
    }
    auto back = l;

    while (back->next->next != nullptr) {
        if (back->next->value < back->next->next->value) {
            back->next = back->next->next;
        } else {
            back = back->next;
        }
    }
}

template<class T>
void remove_ascending_r (LN<T>*& l) {
    if (l == nullptr || l->next == nullptr) {
        return;
    } else if (l->value < l->next->value) {
        auto to_delete = l;
        l = l->next;
        delete to_delete;
        return remove_ascending_r(l);
    } return remove_ascending_r(l->next);
}



#endif /* SOLUTION_HPP_ */
