#ifndef HASH_SET_HPP_
#define HASH_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"


namespace ics {


#ifndef undefinedhashdefined
#define undefinedhashdefined
template<class T>
int undefinedhash (const T& a) {return 0;}
#endif /* undefinedhashdefined */

//Instantiate the templated class supplying thash(a): produces a hash value for a.
//If thash is defaulted to undefinedhash in the template, then a constructor must supply chash.
//If both thash and chash are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedhash value supplied by thash/chash is stored in the instance variable hash.
template<class T, int (*thash)(const T& a) = undefinedhash<T>> class HashSet {
  public:
    typedef int (*hashfunc) (const T& a);

    //Destructor/Constructors
    ~HashSet ();

    HashSet (double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const T& k) = undefinedhash<T>);
    HashSet (const HashSet<T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (const std::initializer_list<T>& il, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashSet (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);


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
    HashSet<T,thash>& operator = (const HashSet<T,thash>& rhs);
    bool operator == (const HashSet<T,thash>& rhs) const;
    bool operator != (const HashSet<T,thash>& rhs) const;
    bool operator <= (const HashSet<T,thash>& rhs) const;
    bool operator <  (const HashSet<T,thash>& rhs) const;
    bool operator >= (const HashSet<T,thash>& rhs) const;
    bool operator >  (const HashSet<T,thash>& rhs) const;

    template<class T2, int (*hash2)(const T2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashSet<T2,hash2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashSet<T,thash>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HashSet<T,thash>::Iterator& operator ++ ();
        HashSet<T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashSet<T,thash>::Iterator& rhs) const;
        bool operator != (const HashSet<T,thash>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashSet<T,thash>::begin () const;
        friend Iterator HashSet<T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor              current; //Bin Index and Cursor; stops if LN* == nullptr
        HashSet<T,thash>*   ref_set;
        int                 expected_mod_count;
        bool                can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashSet<T,thash>* iterate_over, bool from_begin);
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

public:
  int (*hash)(const T& k);   //Hashing function used (from template or constructor)
private:
  LN** set      = nullptr;   //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;     //used/bins <= load_threshold
  int bins      = 1;         //# bins in array (should start >= 1 so hash_compress doesn't % 0)
  int used      = 0;         //Cache for number of key->value pairs in the hash table
  int mod_count = 0;         //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const T& key)              const;  //hash function ranged to [0,bins-1]
  LN*   find_element         (const T& element)          const;  //Returns reference to element's node or nullptr
  LN*   copy_list            (LN*   l)                   const;  //Copy the elements in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)         const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                     //Reallocate if load_threshold > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);               //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};





//HashSet class and related definitions

////////////////////////////////////////////////////////////////////////////////
//
//Destructor/Constructors

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::~HashSet() {
    delete_hash_table(set, bins);
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::default constructor: both specified and different");

    set = new LN* [bins];
    for (auto i = 0; i < bins; i++) {
        set[i] = new LN();
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(int initial_bins, double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::default constructor: both specified and different");

    bins = initial_bins;
    set = new LN*[bins];
    for (auto i = 0; i < bins; i++) {
        set[i] = new LN();
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const HashSet<T,thash>& to_copy, double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash) {
    if (hash == (hashfunc)undefinedhash<T>)
        hash = to_copy.hash;
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::copy constructor: both specified and different");

    if (hash == to_copy.hash) {
        bins = to_copy.bins;
        used = to_copy.used;
        set = copy_hash_table(to_copy.set, to_copy.bins);
    } else {
        bins = to_copy.bins;
        set = new LN* [bins];
        for (int i = 0; i < bins; i++)
            set[i] = new LN();
        for (int i = 0; i < to_copy.bins; i++) {
            LN* head = to_copy.set[i];
            while (head->next) {
                insert(head->value);
                head = head->next;
            }
        }
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const std::initializer_list<T>& il, double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::initializer_list constructor: both specified and different");

    bins = 1;
    set = new LN* [bins];
    for (int i = 0; i < bins; i++)
        set[i] = new LN();

    for (auto i : il) {
        insert(i);
    }
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
HashSet<T,thash>::HashSet(const Iterable& i, double the_load_threshold, int (*chash)(const T& a))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::Iterable constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::Iterable constructor: both specified and different");

    bins = 1;
    set = new LN* [bins];
    for (int i = 0; i < bins; i++)
        set[i] = new LN();

    for (auto j : i) {
        insert(j);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::empty() const {
    return (used == 0);
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::size() const {
    return used;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::contains (const T& element) const {
    return (find_element(element) != nullptr);
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::str() const {
    std::ostringstream answer;
    answer << "HashSet\n";

    if(used) {
        for (int i = 0; i < bins; i++) {
            answer<<"Bin:["<<i<<"] ";
            auto p = set[i];
            while(p->next) {
                answer<<p->value<<"->";
                p=p->next;
            }
            if(i != bins -1)
                answer<<"TRAILER\n";
        }
    }
    answer << "TRAILER](used=" << used << ",bins= "<<bins<<",mod_count=" << mod_count << ")";
    return answer.str();
}


template<class T, int (*thash)(const T& a)>
template <class Iterable>
bool HashSet<T,thash>::contains_all(const Iterable& i) const {
    for (auto j : i) {
        if (!contains(j)) {
            return false;
        }
    } return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::insert(const T& element) {
    if (contains(element)) {
        return 0;
    } else {
        ensure_load_threshold(used+1);
        mod_count++;
        used++;
        int index = hash_compress(element);
        LN* head = set[index];
        while (head->next != nullptr) {
            head = head->next;
        }
        head->next = new LN();
        head->value = element;
        return 1;
    }
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::erase(const T& element) {
    LN *p = find_element(element);
    if (p == nullptr) {
        return 0;
    }

    int index = hash_compress(element);
    LN* head = set[index];

    while (head->next != nullptr) {
        if (head->value == element) {
            used--;
            mod_count++;
            auto value = head->value;
            head->value = head->next->value;
            auto del = head->next;
            head->next = head->next->next;
            delete del;
            return 1;
        } else {
            head = head->next;
        }
    }
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::clear() {
    used = 0;
    mod_count++;
    for (int i = 0; i < bins; i++) {
        auto head = set[i];
        while (head->next) {
            auto del = head;
            head = head->next;
            delete del;
        } set[i] = head;
    }
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::insert_all(const Iterable& i) {
    int count = 0;
    for (auto j : i) {
        count += insert(j);
    } return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::erase_all(const Iterable& i) {
    int count = 0;
    for (auto j : i) {
        count += erase(j);
    } return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::retain_all(const Iterable& i) {
    HashSet<T,thash> newSet(i);
    int counter = 0;
    for (int i = 0; i < bins; i++) {
        LN* head = set[i];
        while (head->next != nullptr) {
            if (!newSet.contains(head->value)) {
                LN* del = head->next;
                head->value = head->next->value;
                head->next = head->next->next;
                delete del;
                counter++;
                used--;
            } else {
                head = head->next;
            }
        }
    } return counter;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>& HashSet<T,thash>::operator = (const HashSet<T,thash>& rhs) {
    if (this == &rhs) {
        return *this;
    }
    clear();
    for (auto i = 0; i < rhs.bins; i++) {
        auto head = rhs.set[i];
        while (head->next != nullptr) {
            insert(head->value);
            head = head->next;
        }
    } return *this;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator == (const HashSet<T,thash>& rhs) const {
    if (this == &rhs) {
        return true;
    } else if (used != rhs.used) {
        return false;
    }

    for (int i = 0; i < bins; i++) {
        LN *head = this->set[i];
        while (head->next != nullptr) {
            if (!rhs.find_element(head->value)) {
                return false;
            } else {
                head = head->next;
            }
        }
    } return true;


}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator != (const HashSet<T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator <= (const HashSet<T,thash>& rhs) const {
    if (this == &rhs) {
        return false;
    }

    if (used > rhs.size()) {
        return false;
    }

    for (int i =0; i < bins; i++) {
        LN* head = set[i];
        while (head->next != nullptr) {
            if (!rhs.contains(head->value)) {
                return false;
            } else {
                head= head->next;
            }
        }
    } return true;
}

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator < (const HashSet<T,thash>& rhs) const {
    if (this == &rhs) {
        return false;
    }

    if (used >= rhs.size()) {
        return false;
    }

    for (int i = 0; i < bins; i++) {
        LN* head = set[i];
        while (head->next != nullptr) {
            if (!rhs.contains(head->value)) {
                return false;
            } else {
                head = head->next;
            }
        }
    } return true;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator >= (const HashSet<T,thash>& rhs) const {
    return rhs <= *this;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator > (const HashSet<T,thash>& rhs) const {
    return rhs < *this;
}


template<class T, int (*thash)(const T& a)>
std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>& s) {
    outs << "set[";
    for (auto i = 0; i < s.bins; i++) {
        auto head = s.set[i];
        while (head->next != nullptr) {
            outs << head->value;
            head = head->next;
        }
    } outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::begin () const -> HashSet<T,thash>::Iterator {
    return Iterator(const_cast<HashSet<T, thash>*>(this),true);
}


template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::end () const -> HashSet<T,thash>::Iterator {
    return Iterator(const_cast<HashSet<T, thash>*>(this),false);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::hash_compress (const T& element) const {
    int index = hash(element);
    return (abs(index) % bins);
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::find_element (const T& element) const {
    for (int i = 0; i < bins; i++) {
        auto head = set[i];
        while (head->next != nullptr) {
            if (head->value == element) {
                return head;
            } else {
                head = head->next;
            }
        }
    } return nullptr;
}

template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::copy_list (LN* l) const {
    LN* head = new LN(l->value);
    LN* runner = head;
    l = l->next;
    while (l) {
        runner->next = new LN(l->value);
        runner = runner->next;
        l = l->next;
    }

    return head;
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN** HashSet<T,thash>::copy_hash_table (LN** ht, int bins) const {
    LN** hashSet = new LN* [bins];
    for (int i = 0; i < bins; i++) {
        hashSet[i] = copy_list(ht[i]);
    } return hashSet;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::ensure_load_threshold(int new_used) {
    if(new_used/bins > load_threshold) {
        int b = bins;
        bins*=2;
        LN** new_set = set;
        set = new LN*[bins];
        for(int i = 0; i < bins; i ++) {
            set[i] = new LN();
        }
        for(int i = 0 ; i < b; i++) {
            auto p = new_set[i];
            while (p->next) {
                int index = hash_compress(p->value);
                auto head = set[index];
                while (head->next) {
                    head = head->next;
                }
                head->value = p->value;
                head->next = new LN();
                p = p->next;
            }
        }
        delete new_set;
    }
    else return;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::delete_hash_table (LN**& ht, int bins) {
    for (int i = 0; i < bins; i++) {
        LN* head = ht[i];
        while (head->next) {
            auto del = head;
            head = head->next;
            delete head;
        }
    } delete ht;
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::Iterator::advance_cursors() {
    if (current.second && current.second->next && current.second->next->next) {
        current.second = current.second->next;
    } else {
        int i = current.first+1;
        bool found = false;
        while (i < ref_set->bins) {
            if (ref_set->set[i]->next) {
                found = true;
                current.first = i;
                current.second = ref_set->set[i];
                return;
            } i++;
        }
        if (!found) {
            current.first = -1;
            current.second = nullptr;
        }
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::Iterator(HashSet<T,thash>* iterate_over, bool begin)
: ref_set(iterate_over), expected_mod_count(ref_set->mod_count) {
    current.first = -1;
    current.second = nullptr;
    if (begin) {
        advance_cursors();
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::~Iterator()
{}


template<class T, int (*thash)(const T& a)>
T HashSet<T,thash>::Iterator::erase() {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashSet::Iterator::erase Iterator cursor already erased");
    if (!current.second)
        throw CannotEraseError("HashSet::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    auto to_return = current.second->value;
    LN* del = current.second->next;
    current.second->value = current.second->next->value;
    current.second->next = current.second->next->next;
    delete del;

    ref_set->mod_count++;
    ref_set->used--;
    expected_mod_count = ref_set->mod_count;

    return to_return;
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::Iterator::str() const {
  std::ostringstream answer;
  answer << current.second << "/expected_mod_count=" << expected_mod_count << "/can_erase=" << can_erase;
  return answer.str();
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ () -> HashSet<T,thash>::Iterator& {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ++");

    if (!current.second)
        return *this;

    if (can_erase || !current.second->next)
        advance_cursors();


    can_erase = true;
    return *this;
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ (int) -> HashSet<T,thash>::Iterator {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ++(int)");

    if (!current.second)
        return *this;

    Iterator to_return(*this);
    if (can_erase || !current.second->next)
        advance_cursors();


    can_erase = true;
    return to_return;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator == (const HashSet<T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashSet::Iterator::operator ==");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ==");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("HashSet::Iterator::operator ==");

    return this->current.second == rhsASI->current.second;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator != (const HashSet<T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashSet::Iterator::operator !=");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator !=");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("HashSet::Iterator::operator !=");

    return this->current.second != rhsASI->current.second;
}

template<class T, int (*thash)(const T& a)>
T& HashSet<T,thash>::Iterator::operator *() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator *");
    if (!can_erase || !current.second)
        throw IteratorPositionIllegal("HashSet::Iterator::operator * Iterator illegal");

    return current.second->value;
}

template<class T, int (*thash)(const T& a)>
T* HashSet<T,thash>::Iterator::operator ->() const {
  if (expected_mod_count != ref_set->mod_count)
    throw ConcurrentModificationError("HashSet::Iterator::operator ->");
  if (!can_erase || !current.second)
    throw IteratorPositionIllegal("HashSet::Iterator::operator -> Iterator illegal");

  return &current.second->value;
}

}

#endif /* HASH_SET_HPP_ */
