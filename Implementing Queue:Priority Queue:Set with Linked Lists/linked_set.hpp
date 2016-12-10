#ifndef LINKED_SET_HPP_
#define LINKED_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedSet {
  public:
    //Destructor/Constructors
    ~LinkedSet();

    LinkedSet          ();
    explicit LinkedSet (int initialLength);
    LinkedSet          (const LinkedSet<T>& to_copy);
    explicit LinkedSet (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedSet (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool contains   (const T& element) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    bool contains_all (const Iterable& i) const;


    //Commands
    int  insert (const T& element);
    int  erase  (const T& element);
    void clear  ();

    //Iterable class must support "for" loop: .begin()/.end() and prefix ++ on returned result

    template <class Iterable>
    int insert_all(const Iterable& i);

    template <class Iterable>
    int erase_all(const Iterable& i);

    template<class Iterable>
    int retain_all(const Iterable& i);


    //Operators
    LinkedSet<T>& operator = (const LinkedSet<T>& rhs);
    bool operator == (const LinkedSet<T>& rhs) const;
    bool operator != (const LinkedSet<T>& rhs) const;
    bool operator <= (const LinkedSet<T>& rhs) const;
    bool operator <  (const LinkedSet<T>& rhs) const;
    bool operator >= (const LinkedSet<T>& rhs) const;
    bool operator >  (const LinkedSet<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedSet<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedSet<T>::Iterator& operator ++ ();
        LinkedSet<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedSet<T>::Iterator& rhs) const;
        bool operator != (const LinkedSet<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedSet<T>::begin () const;
        friend Iterator LinkedSet<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*           current;  //if can_erase is false, this value is unusable
        LinkedSet<T>* ref_set;
        int           expected_mod_count;
        bool          can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedSet<T>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next   = nullptr;
    };


    LN* front     = new LN();
    LN* trailer   = front;         //Always point to special trailer LN
    int used      =  0;            //Cache the number of values in linked list
    int mod_count = 0;             //For sensing concurrent modification

    //Helper methods
    int  erase_at   (LN* p);
    void delete_list(LN*& front);  //Deallocate all LNs (but trailer), and set front's argument to trailer;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedSet class and related definitions

//Destructor/Constructors

template<class T>
LinkedSet<T>::~LinkedSet() {
    delete_list(front);
    delete trailer;
}


template<class T>
LinkedSet<T>::LinkedSet() {

}


template<class T>
LinkedSet<T>::LinkedSet(const LinkedSet<T>& to_copy) : used(to_copy.used) {
    auto i = to_copy.front;
    while (i != to_copy.trailer) {
        front = new LN(i->value, front);
        i = i->next;
    }
}


template<class T>
LinkedSet<T>::LinkedSet(const std::initializer_list<T>& il) {
    for (const T& i : il) {
        insert(i);
    }
}


template<class T>
template<class Iterable>
LinkedSet<T>::LinkedSet(const Iterable& i) {
    for (const T& j: i) {
        insert(j);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedSet<T>::empty() const {
    return used == 0;
}


template<class T>
int LinkedSet<T>::size() const {
    return used;
}


template<class T>
bool LinkedSet<T>::contains (const T& element) const {
    auto find = front;
    while (find != trailer) {
        if (find->value == element) {
            return true;
        } else {
            find = find->next;
        }
    } return false;
}


template<class T>
std::string LinkedSet<T>::str() const {
    auto i = front;
    std::ostringstream result;
    result << "LinkedSet[";
    if (!this->empty()) {
        result << "0:" << i->value;
        int k = 1;
        for (i = front->next; i != trailer; i = i->next) {
            result << "," << k << ":" << i->value;
            k++;
        }
    }   result << "](used=" << used << ",front=" << front << ",trailer=" << trailer << ",mod_count=" << mod_count << ")";
    return result.str();
}


template<class T>
template<class Iterable>
bool LinkedSet<T>::contains_all (const Iterable& i) const {
    for (const T& j : i) {
        if (!contains(j)) {
            return false;
        }
    } return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands


template<class T>
int LinkedSet<T>::insert(const T& element) {
    for (auto i = front; i != trailer; i = i->next) {
        if (i->value == element) {
            return 0;
        }
    }

    used++;
    mod_count++;
    front = new LN(element, front);
    return 1;

}


template<class T>
int LinkedSet<T>::erase(const T& element) {
    auto i = front;
    while (i != trailer) {
        if (i->value == element) {
            return erase_at(i);
        } i = i->next;
    } return 0;
}


template<class T>
void LinkedSet<T>::clear() {
    used = 0;
    delete_list(front);
    mod_count++;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::insert_all(const Iterable& i) {
    int counter = 0;
    for (const T& j : i) {
        counter += insert(j);
    } return counter;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::erase_all(const Iterable& i) {
    int counter = 0;
    for (const T& v : i) {
        counter += erase(v);
    } return counter;

}


template<class T>
template<class Iterable>
int LinkedSet<T>::retain_all(const Iterable& i) {
    LinkedSet s(i);
    int count = 0;
    auto j = front;
    while (j != trailer) {
        if (!s.contains(j->value)) {
            erase_at(j);
            ++count;
        }  else {
            j = j->next;
        }
    } return count;

}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedSet<T>& LinkedSet<T>::operator = (const LinkedSet<T>& rhs) {
    if (this == &rhs) {
        return *this;
    }
    used = rhs.used;
    mod_count++;
    this->clear();
    auto i = rhs.front;
    while (i != rhs.trailer) {
        front->next = new LN(i->value);
        i = i->next;
    } for (auto j : rhs) {
        insert(j);
    }
//    trailer = rhs.trailer;
    return *this;
}


template<class T>
bool LinkedSet<T>::operator == (const LinkedSet<T>& rhs) const {
    if (this == &rhs) {
        return true;
    }
    if (used != rhs.size()) {
        return false;
    }
    for (auto i = front; i != trailer; i = i->next) {
        if (!rhs.contains(i->value)) {
            return false;
        }
    }
    return true;
}


template<class T>
bool LinkedSet<T>::operator != (const LinkedSet<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
bool LinkedSet<T>::operator <= (const LinkedSet<T>& rhs) const {
    if (this == &rhs) {
        return true;
    }
    if (used > rhs.size()) {
        return false;
    }
    for (auto i = front; i != trailer; i = i->next) {
        if (!rhs.contains(i->value)) {
            return false;
        }
    } return true;
}


template<class T>
bool LinkedSet<T>::operator < (const LinkedSet<T>& rhs) const {
    if (this == &rhs) {
        return false;
    }
    if (used >= rhs.size()) {
        return false;
    }
    for (auto i = front; i != trailer; i = i->next) {
        if (!rhs.contains(i->value)) {
            return false;
        }
    } return true;
}


template<class T>
bool LinkedSet<T>::operator >= (const LinkedSet<T>& rhs) const {
    return rhs <= *this;
}


template<class T>
bool LinkedSet<T>::operator > (const LinkedSet<T>& rhs) const {
    return rhs < *this;
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedSet<T>& s) {
    auto i = s.front;
    outs << "set[";
    if (!s.empty()) {
        outs << i->value;
        for (i = i->next; i != s.trailer; i = i->next) {
            outs << "," << i->value;
        }
    }

    outs << "]";
    return outs;

}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedSet<T>::begin () const -> LinkedSet<T>::Iterator {
    return Iterator(const_cast<LinkedSet<T>*>(this), front);
}


template<class T>
auto LinkedSet<T>::end () const -> LinkedSet<T>::Iterator {
    return Iterator(const_cast<LinkedSet<T>*>(this), trailer);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
int LinkedSet<T>::erase_at(LN* p) {
    used--;
    mod_count++;
    if (p->next != trailer) {
        auto remove = p->next;
        *p = *p->next;
        delete remove;
        return 1;
    } else {
        auto remove = p->next;
        trailer = p;
        *p = *p->next;
        delete remove;
        return 1;
    }
}


template<class T>
void LinkedSet<T>::delete_list(LN*& front) {
    while (front != trailer) {
        auto removeNode = front;
        front = front->next;
        delete removeNode;
    } front = trailer;
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedSet<T>::Iterator::Iterator(LinkedSet<T>* iterate_over, LN* initial)
: current(initial), ref_set(iterate_over), expected_mod_count(ref_set->mod_count){
}


template<class T>
LinkedSet<T>::Iterator::~Iterator()
{}


template<class T>
T LinkedSet<T>::Iterator::erase() {
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::erase");}
    if (!can_erase) {
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor already erased");
    }
    if (current == ref_set->trailer) {
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor beyond data structure");
    }

    can_erase = false;
    T to_return = current->value;
    ref_set->erase_at(current);
    expected_mod_count = ref_set->mod_count;
    return to_return;
}


template<class T>
std::string LinkedSet<T>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_set->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ () -> LinkedSet<T>::Iterator& {
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++");
    }
    if (current == ref_set->trailer) {
        return *this;
    }
    if (can_erase) {
        current = current->next;
    }
    else {
        can_erase = true;  //current already indexes "one beyond" erased value
    }

    return *this;
}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ (int) -> LinkedSet<T>::Iterator {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++(int)");

    if (current == ref_set->trailer)
        return *this;

    Iterator to_return(*this);
    if (can_erase) {
        current = current->next;
    }
    else {
        can_erase = true;  //current already indexes "one beyond" erased value
    }

    return to_return;

}


template<class T>
bool LinkedSet<T>::Iterator::operator == (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0) {
        throw IteratorTypeError("LinkedSet::Iterator::operator ==");
    }
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ==");
    }
    if (ref_set != rhsASI->ref_set) {
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator ==");
    }
    return current == rhsASI->current;
}


template<class T>
bool LinkedSet<T>::Iterator::operator != (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0) {
        throw IteratorTypeError("LinkedSet::Iterator::operator !=");
    }
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::operator !=");
    }
    if (ref_set != rhsASI->ref_set) {
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator !=");
    }
    return current != rhsASI->current;
}


template<class T>
T& LinkedSet<T>::Iterator::operator *() const {
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::operator *");
    }
    if (!can_erase || current == ref_set->trailer) {
        std::ostringstream where;
        where << current << " when size = " << ref_set->size();
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator * Iterator illegal: "+where.str());
    }

    return current->value;
}


template<class T>
T* LinkedSet<T>::Iterator::operator ->() const {
    if (expected_mod_count != ref_set->mod_count) {
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ->");
    }
    if (!can_erase || current == ref_set->trailer) {
        std::ostringstream where;
        where << current << " when size = " << ref_set->size();
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator -> Iterator illegal: "+where.str());
    }

    return &current->value;
}


}

#endif /* LINKED_SET_HPP_ */
