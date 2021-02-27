#include <vector>
using namespace std;

void _fibon_elem(int pos, int &elem, vector<int> &cache);

int fibon_elem(int pos, int &elem);

int vector_sum(vector<int> &v);

float vector_sum(vector<float> &v);

template <typename T>
T template_vector_sum(vector<T> &v);
