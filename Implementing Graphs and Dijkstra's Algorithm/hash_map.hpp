#ifndef HASH_MAP_HPP_
#define HASH_MAP_HPP_

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
template<class KEY,class T, int (*thash)(const KEY& a) = undefinedhash<KEY>> class HashMap {
  public:
    typedef ics::pair<KEY,T>   Entry;
    typedef int (*hashfunc) (const KEY& a);

    //Destructor/Constructors
    ~HashMap ();

    HashMap          (double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
    explicit HashMap (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const KEY& k) = undefinedhash<KEY>);
    HashMap          (const HashMap<KEY,T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
    explicit HashMap (const std::initializer_list<Entry>& il, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashMap (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool has_key    (const KEY& key) const;
    bool has_value  (const T& value) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    T    put   (const KEY& key, const T& value);
    T    erase (const KEY& key);
    void clear ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int put_all(const Iterable& i);


    //Operators

    T&       operator [] (const KEY&);
    const T& operator [] (const KEY&) const;
    HashMap<KEY,T,thash>& operator = (const HashMap<KEY,T,thash>& rhs);
    bool operator == (const HashMap<KEY,T,thash>& rhs) const;
    bool operator != (const HashMap<KEY,T,thash>& rhs) const;

    template<class KEY2,class T2, int (*hash2)(const KEY2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY2,T2,hash2>& m);



  private:
    class LN;

  public:
    class Iterator {
      public:
         typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashMap<T>
        ~Iterator();
        Entry       erase();
        std::string str  () const;
        HashMap<KEY,T,thash>::Iterator& operator ++ ();
        HashMap<KEY,T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        bool operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        Entry& operator *  () const;
        Entry* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashMap<KEY,T,thash>::begin () const;
        friend Iterator HashMap<KEY,T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor                current; //Bin Index and Cursor; stops if LN* == nullptr
        HashMap<KEY,T,thash>* ref_map;
        int                   expected_mod_count;
        bool                  can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
    public:
      LN ()                         : next(nullptr){}
      LN (const LN& ln)             : value(ln.value), next(ln.next){}
      LN (Entry v, LN* n = nullptr) : value(v), next(n){}

      Entry value;
      LN*   next;
  };

  int (*hash)(const KEY& k);  //Hashing function used (from template or constructor)
  LN** map      = nullptr;    //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;      //used/bins <= load_threshold
  int bins      = 1;          //# bins in array (should start >= 1 so hash_compress doesn't % 0)
  int used      = 0;          //Cache for number of key->value pairs in the hash table
  int mod_count = 0;          //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const KEY& key)          const;  //hash function ranged to [0,bins-1]
  LN*   find_key             (const KEY& key) const;           //Returns reference to key's node or nullptr
  LN*   copy_list            (LN*   l)                 const;  //Copy the keys/values in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)       const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                   //Reallocate if load_factor > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);             //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};





////////////////////////////////////////////////////////////////////////////////
//
//HashMap class and related definitions

//Destructor/Constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::~HashMap() {
    delete_hash_table(map, bins);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
  if (hash == (hashfunc)undefinedhash<KEY>)
    throw TemplateFunctionError("HashMap::default constructor: neither specified");
  if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
    throw TemplateFunctionError("HashMap::default constructor: both specified and different");

    map = new LN* [bins];
    for (auto i = 0; i < bins; i++) {
        map[i] = new LN();
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(int initial_bins, double the_load_threshold, int (*chash)(const KEY& k))
        : hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::default constructor: both specified and different");

    bins = initial_bins;
    map = new LN*[bins];
    for (auto i = 0; i < bins; i++) {
        map[i] = new LN();
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const HashMap<KEY,T,thash>& to_copy, double the_load_threshold, int (*chash)(const KEY& a))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        hash = to_copy.hash;
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::copy constructor: both specified and different");

    if (hash == to_copy.hash) {
        bins = to_copy.bins;
        used = to_copy.used;
        map = copy_hash_table(to_copy.map, to_copy.bins);
    } else {
        bins = to_copy.bins;
        map = new LN* [bins];
        for (int i = 0; i < bins; i++)
            map[i] = new LN();
        for (int i = 0; i < to_copy.bins; i++) {
            LN* head = to_copy.map[i];
            while (head->next) {
                put(head->value.first, head->value.second);
                head = head->next;
            }
        }
    }

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const std::initializer_list<Entry>& il, double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::initializer_list constructor: both specified and different");

    bins = 1;
    map = new LN* [bins];
    for (int i = 0; i < bins; i++)
        map[i] = new LN();

    for (auto i : il) {
        put(i.first, i.second);
    }

}


template<class KEY,class T, int (*thash)(const KEY& a)>
template <class Iterable>
HashMap<KEY,T,thash>::HashMap(const Iterable& i, double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::Iterable constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::Iterable constructor: both specified and different");

    bins = 1;
    map = new LN* [bins];
    for (int i = 0; i < bins; i++)
        map[i] = new LN();

    for (auto j : i) {
        put(j.first, j.second);
    }
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::empty() const {
    return (used == 0);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::size() const {
    return used;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_key (const KEY& key) const {
    return find_key(key) != nullptr;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_value (const T& value) const {
    for (int i = 0; i < bins; i++) {
        auto head = map[i];
        while (head->next != nullptr) {
            if (head->value.second == value) {
                return true;
            } else {
                head = head->next;
            }
        }
    } return false;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::str() const {
    std::ostringstream answer;
    answer << "HashMap\n";

    if(used) {
        for (int i = 0; i < bins; i++) {
            answer<<"Bin:["<<i<<"] ";
            auto p = map[i];
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


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::put(const KEY& key, const T& value) {
    mod_count++;
    auto p = find_key(key);
    if (p != nullptr) {
        auto v = p->value.second;
        p->value.second = value;
        return v;
    } else {
        used++;
        ensure_load_threshold(used+1);
        int index = hash_compress(key);
        LN* head = map[index];
        while (head->next != nullptr) {
            head = head->next;
        }
        head->next = new LN();
        head->value.first = key;
        head->value.second = value;
        return value;
    }

}


template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::erase(const KEY& key) {
    LN *p = find_key(key);
    if (p == nullptr) {
        std::ostringstream answer;
        answer << "HashMap::erase: key(" << key << ") not in Map";
        throw KeyError(answer.str());
    }

    int index = hash_compress(key);
    LN* head = map[index];

    while (head->next != nullptr) {
        if (head->value.first == key) {
            used--;
            mod_count++;
            auto value = head->value.second;
            head->value.first = head->next->value.first;
            head->value.second = head->next->value.second;
            auto del = head->next;
            head->next = head->next->next;
            delete del;
            return value;
        } else {
            head = head->next;
        }
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::clear() {
    used = 0;
    mod_count++;
    for (int i = 0; i < bins; i++) {
        auto head = map[i];
        while (head->next) {
            auto del = head;
            head = head->next;
            delete del;
        } map[i] = head;
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template<class Iterable>
int HashMap<KEY,T,thash>::put_all(const Iterable& i) {
    int count = 0;
    for (auto j : i) {
        put(j.first, j.second);
        count++;
    } return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class KEY,class T, int (*thash)(const KEY& a)>
T& HashMap<KEY,T,thash>::operator [] (const KEY& key) {
    auto p = find_key(key);
    if (p != nullptr) {
        return p->value.second;
    } else {
        ensure_load_threshold(used+1);
        used++;
        mod_count++;
        int index = hash_compress(key);
        LN* head = map[index];
        while (head->next != nullptr) {
            head = head->next;
        }
        head->next = new LN();
        head->value = Entry(key, T());
        return head->value.second;
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
const T& HashMap<KEY,T,thash>::operator [] (const KEY& key) const {
    LN* p = find_key(key);
    if (p != nullptr)
        return p->value.second;

    std::ostringstream answer;
    answer << "HashMap::operator []: key(" << key << ") not in Map";
    throw KeyError(answer.str());
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>& HashMap<KEY,T,thash>::operator = (const HashMap<KEY,T,thash>& rhs) {
    if (this == &rhs) {
        return *this;
    }

    clear();
    for (auto i = 0; i < rhs.bins; i++) {
        auto head = rhs.map[i];
        while (head->next != nullptr) {
            put(head->value.first, head->value.second);
            head = head->next;
        }
    } mod_count++;

    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator == (const HashMap<KEY,T,thash>& rhs) const {
    if (this == &rhs) {
        return true;
    } else if (used != rhs.used) {
        return false;
    }
    for (int i = 0; i < bins; i++) {
        LN* head = this->map[i];
        while (head->next != nullptr) {
            if (!rhs.has_key(head->value.first) || !rhs.has_value(head->value.second)) {
                return false;
            }else {
                head = head->next;
            }
        }
    } return true;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator != (const HashMap<KEY,T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>& m) {
    outs << "map[";
    for (auto i = 0; i < m.bins; i++) {
        auto head = m.map[i];
        while (head->next != nullptr) {
            outs << head->value.first << "->" << head->value.second;
            head = head->next;
        }
    } outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::begin () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY, T, thash>*>(this),true);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::end () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY, T, thash>*>(this),false);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::hash_compress (const KEY& key) const {
    int index = hash(key);
    return (abs(index) % bins);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::find_key (const KEY& key) const {
    int index = hash_compress(key);
    LN *head = map[index];
    while (head->next != nullptr) {
        if (head->value.first == key) {
            return head;
        } else {
            head = head->next;
        }
    }
    return nullptr;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::copy_list (LN* l) const {
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


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN** HashMap<KEY,T,thash>::copy_hash_table (LN** ht, int bins) const {
    LN** hashMap = new LN* [bins];
    for (int i = 0; i < bins; i++) {
        hashMap[i] = copy_list(ht[i]);
    } return hashMap;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::ensure_load_threshold(int new_used) {
    if(new_used/bins > load_threshold) {
        int b = bins;
        bins*=2;
        LN** new_map = map;
        map = new LN*[bins];
        for(int i = 0; i < bins; i ++) {
            map[i] = new LN();
        }
        for(int i = 0 ; i < b;i++) {
            auto p = new_map[i];
            while (p->next) {
                int index = hash_compress(p->value.first);
                auto head = map[index];
                while (head->next) {
                    head = head->next;
                }
                head->value.first = p->value.first;
                head->value.second = p->value.second;
                head->next = new LN();
                p = p->next;
            }
        }
        delete new_map;
    }
    else return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::delete_hash_table (LN**& ht, int bins) {
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

template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::Iterator::advance_cursors(){
    if (current.second && current.second->next && current.second->next->next) {
        current.second = current.second->next;
    } else {
        int i = current.first+1;
        bool found = false;
        while (i < ref_map->bins) {
            if (ref_map->map[i]->next) {
                found = true;
                current.first = i;
                current.second = ref_map->map[i];
                return;
            } i++;
        }
        if (!found) {
            current.first = -1;
            current.second = nullptr;
        }
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin)
: ref_map(iterate_over), expected_mod_count(ref_map->mod_count) {
    current.first = -1;
    current.second = nullptr;
    if (from_begin) {
        advance_cursors();
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::~Iterator()
{}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::Iterator::erase() -> Entry {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor already erased");
    if (!current.second)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    auto to_return = current.second->value;
    LN* del = current.second->next;
    current.second->value.first = current.second->next->value.first;
    current.second->value.second = current.second->next->value.second;
    current.second->next = current.second->next->next;
    delete del;

    ref_map->mod_count++;
    ref_map->used--;
    expected_mod_count = ref_map->mod_count;

    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::Iterator::str() const {
  std::ostringstream answer;
  answer << current.second << "/expected_mod_count=" << expected_mod_count << "/can_erase=" << can_erase;
  return answer.str();
}

template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ () -> HashMap<KEY,T,thash>::Iterator& {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++");

    if (!current.second)
        return *this;

    if (can_erase || !current.second->next)
        advance_cursors();


    can_erase = true;
    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ (int) -> HashMap<KEY,T,thash>::Iterator {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++(int)");

    if (!current.second)
        return *this;

    Iterator to_return(*this);
    if (can_erase || !current.second->next)
        advance_cursors();


    can_erase = true;
    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashMap::Iterator::operator ==");
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ==");
    if (ref_map != rhsASI->ref_map)
        throw ComparingDifferentIteratorsError("HashMap::Iterator::operator ==");

    return this->current.second == rhsASI->current.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("HashMap::Iterator::operator !=");
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator !=");
  if (ref_map != rhsASI->ref_map)
    throw ComparingDifferentIteratorsError("HashMap::Iterator::operator !=");

  return this->current.second != rhsASI->current.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>& HashMap<KEY,T,thash>::Iterator::operator *() const {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator *");
    if (!can_erase || !current.second)
        throw IteratorPositionIllegal("HashMap::Iterator::operator * Iterator illegal");

    return current.second->value;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>* HashMap<KEY,T,thash>::Iterator::operator ->() const {
  if (expected_mod_count != ref_map->mod_count)
    throw ConcurrentModificationError("HashMap::Iterator::operator ->");
  if (!can_erase || !current.second)
    throw IteratorPositionIllegal("HashMap::Iterator::operator -> Iterator illegal");

  return &current.second->value;
}


}

#endif /* HASH_MAP_HPP_ */
