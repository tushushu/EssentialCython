#include <vector>
#include <string>
using namespace std;

//Forward declaration.
class StackIterator;

class Stack
{
    friend class StackIterator;

private:
    vector<string> _elem;
    mutable int _pos;

public:
    static int n_instance;
    Stack();
    ~Stack();
    static int count_instances();
    bool pop(string &elem);
    bool peek(string &elem);
    bool push(const string &elem);
    const string *next() const;
    void reset_next() const;
    bool copy_from(const Stack &s);

    bool empty()
    {
        return _elem.empty();
    }

    bool full()
    {
        return _elem.size() == _elem.max_size();
    }
    StackIterator begin() const;
    StackIterator end() const;
};

class StackIterator
{
public:
    StackIterator(const Stack &rhs, int index);
    // StackIterator(const Stack &rhs, int index);
    bool operator==(const StackIterator &rhs) const
    {
        return _index == rhs._index;
    };
    bool operator!=(const StackIterator &rhs) const
    {
        return !(*this == rhs);
    };
    string operator*() const
    {
        check_integrity();
        return _stack._elem[_index];
    };
    StackIterator &operator++()
    {
        ++_index;
        check_integrity();
        return *this;
    }; //prefix
    StackIterator operator++(int)
    {
        StackIterator tmp = *this;
        ++_index;
        check_integrity();
        return tmp;
    }; //postfix, needs an unchanged object after operation.
private:
    const Stack &_stack;
    int _index;
    void check_integrity() const
    {
        if (_index > _stack._elem.size())
        {
            throw std::out_of_range("Iterator of out range!");
        }
    };
};
