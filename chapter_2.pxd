from libcpp cimport bool
from libcpp.vector cimport vector

cdef extern from "chapter_2_extern.h":
    void _fibon_elem(int pos, int &elem, vector[int] &cache)
    int fibon_elem(int pos, int &elem)
    int vector_sum(vector[int] &v)
    float vector_sum(vector[float] &v)
    T template_vector_sum[T](vector[T] &v)

cdef inline int fmax(int a, int b):
    return a if a > b else b
