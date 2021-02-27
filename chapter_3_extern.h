#include <iostream>
#include <algorithm>
#include <functional>
#include <list>
#include <vector>
using namespace std;

template <typename T>
int find(const T *first, int size, const T &target);

template <typename IteratorType, typename elemType>
int find2(IteratorType first, IteratorType last, const elemType &target);

template <typename InputIterator, typename OutputIterator,
          typename elemType, typename Cmp>
OutputIterator filter(InputIterator first, InputIterator last,
                      OutputIterator output, const elemType &value, Cmp pred);
