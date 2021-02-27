#include "chapter_3_extern.h"

template <typename T>
int find(const T *first, int size, const T &target)
{
    if (!first || !size)
    {
        return -1;
    }
    for (int i = 0; i < size; ++i)
    {
        if (first[i] == target)
        {
            return i;
        }
    }
    return -1;
}

template <typename IteratorType, typename elemType>
int find2(IteratorType first, IteratorType last, const elemType &target)
{
    int i = 0;
    for (; first != last; first++)
    {
        if (*first == target)
        {
            return i;
        }
        ++i;
    }
    return -1;
}

template <typename InputIterator, typename OutputIterator, typename elemType, typename Cmp>
OutputIterator filter(InputIterator first, InputIterator last, OutputIterator output, const elemType &value, Cmp pred)
{

    while ((first = find_if(first, last, bind2nd(pred, value))) != last)
    {
        cout << "Found value: " << *first << endl;
        *output++ = *first++;
    }
    return output;
}

int main(int argc, char const *argv[])
{
    /* code */
    int ia[4] = {1, 2, 3, 4};
    int ix = find(ia, 4, ia[2]);
    cout << "运算结果是:" << ix << endl;

    float fa[4] = {1.0, 2.0, 3.0, 4.0};
    ix = find(fa, 4, fa[3]);
    cout << "运算结果是:" << ix << endl;

    list<int> ilist(ia, ia + 4);
    list<int>::iterator first = ilist.begin();
    list<int>::iterator last = ilist.end();
    int target = 3;
    ix = find2(first, last, target);
    cout << "运算结果是:" << ix << endl;

    int ia[5] = {1, 2, 3, 4, 5};
    vector<int> v(ia, ia + 5);
    vector<int> output(5);
    int target = 3;
    filter(v.begin(), v.end(), output.begin(), target, less<int>());
    return 0;
}
