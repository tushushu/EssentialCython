# distutils: language = c++
# distutils: sources = chapter_4_extern.cpp

from chapter_4 cimport Stack

def test_count_instances():
    cdef Stack* s1 = new Stack()
    cdef Stack* s2 = new Stack()
    cdef Stack* s3 = new Stack()
    cdef Stack s4
    s4.empty()  # If we do not call s4's member, s4 will not call constructor.
    cdef int n = Stack.count_instances()
    print(f"There are {n} instances of Stack!")
    del s1
    del s2
    del s3

