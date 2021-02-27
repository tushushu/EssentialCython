from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool


cdef extern from "chapter_4_extern.h":
    cdef cppclass Stack:
        Stack() except +
        @staticmethod
        int count_instances()
        bool empty()
        bool pop(string &elem)
        bool full()
        bool peek(string &elem)
        bool push(const string &elem)
        const string *next() const
        void reset_next() const
        bool copy_from(const Stack &s)
    
    string test_iterator()

cdef class MyStack:
    cdef vector[string] _stack
