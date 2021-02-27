#include "chapter_4_extern.h"
#include <iostream>
using namespace std;

int Stack::n_instance = 0;

int Stack::count_instances()
{
    return Stack::n_instance;
}

Stack::Stack()
{
    reset_next();
    Stack::n_instance++;
}

Stack::~Stack()
{
    Stack::n_instance--;
}

bool Stack::pop(string &elem)
{
    if (empty())
    {
        return false;
    }
    elem = _elem.back();
    _elem.pop_back();
    return true;
}

bool Stack::peek(string &elem)
{
    if (empty())
    {
        return false;
    }
    elem = _elem.back();
    return true;
}

bool Stack::push(const string &elem)
{
    if (full())
    {
        return false;
    }
    _elem.push_back(elem);
    return true;
}

const string *Stack::next() const
{
    if (++_pos == _elem.size())
    {
        return NULL;
    }
    return &_elem[_pos];
}

void Stack::reset_next() const
{
    _pos = -1;
}

bool Stack::copy_from(const Stack &s)
{
    if (this == &s)
    {
        return false;
    }
    _elem = s._elem;
    _pos = s._pos;
    return true;
}

StackIterator Stack::begin() const
{
    return StackIterator(*this, 0);
}

StackIterator Stack::end() const
{
    int index = _elem.size();
    return StackIterator(*this, index);
}

StackIterator::StackIterator(const Stack &rhs, int index) : _stack(rhs), _index(index)
{
}

string test_iterator(){
    string result = "";
    Stack s;
    s.push("foo");
    s.push("bar");
    s.push("baz");
    StackIterator it = s.begin();
    StackIterator end = s.end();
    while (it != end)
    {
        result += *it;
        result += ", ";
        ++it;
    }
    return result;
}

int main(int argc, char const *argv[])
{
    string res = test_iterator();
    cout << res;
    return 0;
}
