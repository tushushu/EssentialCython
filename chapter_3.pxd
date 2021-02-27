"""
Author: tienan.liu
Date: 2020-11-21 22:03:11
"""
from libcpp cimport bool
from libcpp.vector cimport vector


cdef extern from "<functional>" namespace "std" nogil:
    cdef cppclass less[T=*]:
        less() except +
        bool operator()(const T& lhs, const T& rhs) except +

    cdef cppclass greater[T=*]:
        greater() except +
        bool operator()(const T& lhs, const T& rhs) except +

cdef extern from "chapter_3_extern.h":
    int find[T](const T *first, int size, const T &target)
    int find2[IteratorType, elemType](IteratorType first, IteratorType last, const elemType &target)
    OutputIterator filter[InputIterator, OutputIterator, elemType, Cmp](
        InputIterator first, InputIterator last, OutputIterator output, const elemType &value, Cmp pred)
