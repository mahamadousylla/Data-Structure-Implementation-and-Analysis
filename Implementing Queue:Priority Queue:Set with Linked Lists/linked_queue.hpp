#ifndef LINKED_QUEUE_HPP_
#define LINKED_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedQueue {
  public:
    //Destructor/Constructors
    ~LinkedQueue();

    LinkedQueue          ();
    LinkedQueue          (const LinkedQueue<T>& to_copy);
    explicit LinkedQueue (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedQueue (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    LinkedQueue<T>& operator = (const LinkedQueue<T>& rhs);
    bool operator == (const LinkedQueue<T>& rhs) const;
    bool operator != (const LinkedQueue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T2>& q);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedQueue<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedQueue<T>::Iterator& operator ++ ();
        LinkedQueue<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedQueue<T>::Iterator& rhs) const;
        bool operator != (const LinkedQueue<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedQueue<T>::begin () const;
        friend Iterator LinkedQueue<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev = nullptr;  //if nullptr, current at front of list
        LN*             current;         //current == prev->next (if prev != nullptr)
        LinkedQueue<T>* ref_queue;
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedQueue<T>* iterate_over, LN* initial);
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
        LN* next = nullptr;
    };


    LN* front     =  nullptr;
    LN* rear      =  nullptr;
    int used      =  0;            //Cache the number of values in linked list
    int mod_count =  0;            //For sensing all concurrent modifications

    //Helper methods
    void delete_list(LN*& front);  //Deallocate all LNs, and set front's argument to nullptr;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedQueue class and related definitions

//Destructor/Constructors

template<class T>
LinkedQueue<T>::~LinkedQueue() {
    delete_list(front);
}


template<class T>
LinkedQueue<T>::LinkedQueue() {
    front = nullptr;
    rear = nullptr;
}


template<class T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue<T>& to_copy) {
    auto i = to_copy.front;
    while (i != nullptr) {
        enqueue(i->value);
        i = i->next;
    }
}


template<class T>
LinkedQueue<T>::LinkedQueue(const std::initializer_list<T>& il) {
    for (const T& i : il) {
        enqueue(i);
    }
}


template<class T>
template<class Iterable>
LinkedQueue<T>::LinkedQueue(const Iterable& i) {
    for (const T& j : i) {
        enqueue(j);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedQueue<T>::empty() const {
    return used == 0;
}


template<class T>
int LinkedQueue<T>::size() const {
    return used;
}


template<class T>
T& LinkedQueue<T>::peek () const {
    if (this->empty()) {
        throw EmptyError("LinkedQueue::peek");
    }
    return front->value;
}


template<class T>
std::string LinkedQueue<T>::str() const {
    std::ostringstream result;
    auto i = front;
    result << "LinkedQueue[";
    if (!this->empty()) {
        while (i->next != nullptr) {
            result << i->value << ":";
            i = i->next;
        }
    } result << "](used=" << used << ",front=" << front << ",rear=" << rear << ",mod_count=" << mod_count << ")";
    return result.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T>
int LinkedQueue<T>::enqueue(const T& element) {
    mod_count++;
    used++;
    if (front == nullptr) {
        front = rear = new LN(element, nullptr);
    } else {
        rear->next = new LN(element, nullptr);
        rear = rear->next;
    } return 1;
}


template<class T>
T LinkedQueue<T>::dequeue() {
    if (this->empty()) {
        throw EmptyError("LinkedQueue::dequeue");
    } else {
        mod_count++;
        used--;
        T node = front->value;
        auto removeNode = front;
        front = front->next;
        delete removeNode;
        return node;
    }
}


template<class T>
void LinkedQueue<T>::clear() {
    delete_list(front);
    used = 0;
    mod_count++;
    front = nullptr;
    rear = nullptr;
}


template<class T>
template<class Iterable>
int LinkedQueue<T>::enqueue_all(const Iterable& i) {
    int counter = 0;
    for (const T& j : i) {
        counter += enqueue(j);
    } return counter;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedQueue<T>& LinkedQueue<T>::operator = (const LinkedQueue<T>& rhs) {
    if (this == &rhs) {
        return *this;
    } else {
        used = rhs.used;
        mod_count++;
        if (front != nullptr) {
            delete_list(front);
            for (const T& i : rhs) {
                enqueue(i);
            }
        }
    } return *this;
}


template<class T>
bool LinkedQueue<T>::operator == (const LinkedQueue<T>& rhs) const {
    if (this == &rhs) {
        return true;
    } else if (this->size() != rhs.size()) {
        return false;
    } else {
        for (auto i = front, r = rhs.front; i != nullptr; i = i->next, r = r->next) {
            if (i->value != r->value) {
                return false;
            }
        } return true;
    }
}


template<class T>
bool LinkedQueue<T>::operator != (const LinkedQueue<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>& q) {
    outs << "queue[";
    if (!q.empty()) {
        outs << q.front->value;
        auto i = q.front->next;
        while (i != nullptr) {
            outs << "," << i->value;
            i = i->next;
        }
    } outs << "]:rear";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedQueue<T>::begin () const -> LinkedQueue<T>::Iterator {
    return Iterator(const_cast<LinkedQueue<T>*>(this), front);
}

template<class T>
auto LinkedQueue<T>::end () const -> LinkedQueue<T>::Iterator {
    return Iterator(const_cast<LinkedQueue<T>*>(this), nullptr);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
void LinkedQueue<T>::delete_list(LN*& front) {
    while (front != nullptr) {
        auto removeNode = front;
        front = front->next;
        delete removeNode;
    } front = rear = nullptr;
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedQueue<T>::Iterator::Iterator(LinkedQueue<T>* iterate_over, LN* initial)
: current(initial), ref_queue(iterate_over), expected_mod_count(ref_queue->mod_count) {
}


template<class T>
LinkedQueue<T>::Iterator::~Iterator()
{}


template<class T>
T LinkedQueue<T>::Iterator::erase() {
    if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::erase");
    } if (!can_erase) {
        throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor already erased");
    } if (current == nullptr) {
        throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor beyond data structure");
    }
    can_erase = false;
    T node = current->value;
    auto count = ref_queue->mod_count;
    expected_mod_count = count;
    --ref_queue->used;
    if (prev != nullptr) {
        prev->next = current->next;
        auto temp = prev->next;
        delete current;
        current = temp;
    } else if (prev == nullptr) { //we are at the beginning node
        ref_queue->front = current->next; //prepare to delete head node
        auto temp = current->next;
        delete current;
        current = temp;
    }
    return node;
}


template<class T>
std::string LinkedQueue<T>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_queue->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ () -> LinkedQueue<T>::Iterator& {
    if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++");
    }
    if (current == nullptr) {
        return *this;
    } if (can_erase) {
//        auto temp = current;
        prev = current;
        current = current->next;
//        prev = temp;
//        temp = nullptr;
//        delete temp;
    } else {
        can_erase = true;
    }
    return *this;
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ (int) -> LinkedQueue<T>::Iterator {
    if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++(int)");
    } if (current == nullptr) {
        return *this;
    } Iterator node(*this);
    if (can_erase) {
//        auto temp = current;
        prev = current;
        current = current->next;
//        prev = temp;
//        temp = nullptr;
//        delete temp;
    } else {
        can_erase = true;
    } return node;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator == (const LinkedQueue<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0) {
        throw IteratorTypeError("LinkedQueue::Iterator::operator ==");
    } if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ==");
    } if (ref_queue != rhsASI->ref_queue) {
        throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator ==");
    } return current == rhsASI->current;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator != (const LinkedQueue<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0) {
        throw IteratorTypeError("LinkedQueue::Iterator::operator !=");
    } if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator !=");
    } if (ref_queue != rhsASI->ref_queue) {
        throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator !=");
    } return current != rhsASI->current;
}


template<class T>
T& LinkedQueue<T>::Iterator::operator *() const {
    if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator *");
    } if (!can_erase || !(current != nullptr)) {
        std::ostringstream where;
        where << current
              << " when front = " << ref_queue->front
              << " and rear = " << ref_queue->rear;
        throw IteratorPositionIllegal("LinkedQueue::Iterator::operator * Iterator illegal: "+where.str());
    } return current->value;
}


template<class T>
T* LinkedQueue<T>::Iterator::operator ->() const {
    if (expected_mod_count != ref_queue->mod_count) {
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ->");
    } if (!can_erase || !(current != nullptr)) {
        std::ostringstream where;
        where << current
              << " when front = " << ref_queue->front
              << " and rear = " << ref_queue->rear;
        throw IteratorPositionIllegal("LinkedQueue::Iterator::operator -> Iterator illegal: "+where.str());
    } return &current->value;
}


}

#endif /* LINKED_QUEUE_HPP_ */
