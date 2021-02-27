#include "chapter_2_extern.h"

void _fibon_elem(int pos, int &elem, vector<int> &cache)
{
    elem = 1;
    if (pos == 1 || pos == 2)
    {
        cache.push_back(elem);
        cache.push_back(elem);
        cache.push_back(elem);
    }
    int i = cache.size() - 1;
    int n_1 = cache[i - 1];
    int n_2 = cache[i];
    for (; i < pos; ++i)
    {
        elem = n_1 + n_2;
        n_1 = n_2;
        n_2 = elem;
        cache.push_back(elem);
    }
}

int fibon_elem(int pos, int &elem)
{
    static vector<int> cache;
    if (pos <= 0 || pos > 1024)
    {
        elem = 0;
        return 0;
    }
    if (pos >= cache.size())
    {
        _fibon_elem(pos, elem, cache);
        return 1;
    }
    elem = cache[pos];
    return 2;
}

int vector_sum(vector<int> &v)
{
    int tot = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        tot += v[i];
    }
    return tot;
}

float vector_sum(vector<float> &v)
{
    float tot = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        tot += v[i];
    }

    return tot;
}

template <typename T>
T template_vector_sum(vector<T> &v)
{
    T tot = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        tot += v[i];
    }
    return tot;
}
