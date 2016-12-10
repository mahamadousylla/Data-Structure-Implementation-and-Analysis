#ifndef Q6SOLUTION_HPP_
#define Q6SOLUTION_HPP_


#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>
#include <algorithm>                 // std::swap
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "q6utility.hpp"


////////////////////////////////////////////////////////////////////////////////

//Problem 1

//Write this function
template<class T>
void selection_sort(LN<T>* l) {
    for (LN<T>* p = l; p != nullptr; p = p->next) {
        int index_of_min = 0, smallest_value = p->value, i = 0;
        for (LN<T>* t = p->next; t != nullptr; t = t->next) {
            i++;
            if (t->value < smallest_value) {
                smallest_value = t->value;
                index_of_min = i;
            }
        } LN<T>* j = p;
        for (auto i = 0; i != index_of_min; i++) {
            j = j->next;
        } std::swap(p->value, j->value);
    }
}

////////////////////////////////////////////////////////////////////////////////

//Problem 2

//Write this function
template<class T>
void merge(T a[], int left_low,  int left_high,
                    int right_low, int right_high) {
    int temp_l = left_low, temp_r = right_low, size = right_high - left_low + 1;
    int array[(right_high - left_low + 1)];
    for (auto i = 0; i < size; i++) {
        if (temp_l > left_high) {
            array[i] = a[temp_r];
            temp_r++;
        } else if (temp_r > right_high) {
            array[i] = a[temp_l];
            temp_l++;
        } else if (a[temp_l] <= a[temp_r]) {
            array[i] = a[temp_l];
            temp_l++;
        } else {
            array[i] = a[temp_r];
            temp_r++;
        }
    }

    for (auto i = 0; i < size; i++, left_low++) {
        a[left_low] = array[i];
    }

}


////////////////////////////////////////////////////////////////////////////////

//Problem 3

int select_digit (int number, int place)
{return number/place % 10;}


//Write this function
void radix_sort(int a[], int length) {
    ics::ArrayQueue<int> array[10];
    std::vector<int> placeArray = {1, 10, 100, 1000, 10000, 100000};
    for (auto i = 0; i < placeArray.size(); i++) {
        for (auto j = 0; j < length; j++) {
            auto result = select_digit(a[j], placeArray[i]);
            array[result].enqueue(a[j]);
        }
        for (auto k = 0, index = 0; k < 10; k++) {
            while (array[k].size() > 0) {
                a[index] = array[k].dequeue();
                index++;
            }
        }
    }

}


////////////////////////////////////////////////////////////////////////////////

//Problem 4

//Write this function

//Test how well a partition function (choose_pivot_index: see last and median3) works.
//Test is on an array of the values 0..length-1, randomly shuffled num_tests times
//Returns the average size of the larger of the left and right partitions.
//At best the array is cut in half, yielding the biggest side as 0.5 (50%)
double test_partition(int length, int num_tests, int (*choose_pivot_index) (int a[], int low, int high)) {
    int array[length];
    for (auto i = 0; i < length - 1; i++) {
        array[i] = i;
    }

    double sizeOfBiggerArray;
    for (auto i = 0; i < num_tests; i++) {
        shuffle(array, length);
        auto pivot = choose_pivot_index(array, 0, length-1);
        auto indexOfPivot = partition(array, 0, length -1 , pivot);
        auto halfWayPoint = (length/2);

        if (indexOfPivot < halfWayPoint) {
            indexOfPivot = (length - indexOfPivot) - 1 ;
        } sizeOfBiggerArray += indexOfPivot;
    }
    auto avg =  (sizeOfBiggerArray/num_tests);
    return (avg/length);
}


#endif /* Q6SOLUTION_HPP_ */
