## 4. 基于对象的编程风格
作者: tushushu  
项目地址: https://github.com/tushushu/EssentialCython   
参考书籍: 《Essential C++ 中文版》  
编程环境: MacOS + Jupyter Notebook + Python 3.6  
参考文章: https://phonchi.gitbooks.io/cython_note/content/cython-classextension-type.html  
https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html


```python
%load_ext Cython
```

### 4.0 必要的Cython知识
基于Python的“内置类型”，Cython支持类的扩展类型，有时称为“cdef类”。与Python类相比，它们在某种程度上受到限制，但通常比通用Python类具有更高的内存效率和更快的内存效率。主要区别在于它们使用C结构而不是Python的dict来存储其字段和方法。这样一来，它们就可以将任意C类型存储在其字段中，而无需使用Python的wrapper，并且可以直接在C级别访问字段和方法，而无需通过Python字典查找。  
注意：通过cdef class生成的类的实例依然是一个Python对象。

### 4.1 如何实现一个Class
Class的声明以关键字cdef class开始，其后接一个class名称。Class定义由两部分组成：class的声明，以及紧接在声明之后的主体。  
我们首先创建一个Stack类，也就是数据结构里的堆栈。然后添加一个类成员_stack，用于存储堆栈的元素。接下来创建push方法进行压栈操作，pop方法进行出栈操作，peek方法返回栈顶元素，empty/full方法判断堆栈是空的还是满的。  

然后我们试着创建一个Stack类的实例，并压入三个字符串b'foo', b'bar', b'baz'，打印测试结果和我们预期的一致。

创建类的实例的时候，通过`cdef Stack s`语句声明变量，然后用`s = Stack()`语句创建实例。如果只用后一个语句创建实例的话，会生成一个普通的Python对象，没办法访问通过cdef定义的类成员函数。  
注意：使用cdef定义类成员函数会让我们的程序运行得更快，Cython Classes使用 def, cdef and cpdef的性能表现请参考：  
https://notes-on-cython.readthedocs.io/en/latest/classes.html  


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool
from copy import copy

cdef class Stack:
    cdef vector[string] _stack

    cdef bool push(self, const string &elem):
        if self.full():
            return False
        self._stack.push_back(elem)
        return True
    
    cdef bool pop(self, string *ptr):
        if self.empty():
            return False
        ptr[0] = self._stack.back()
        self._stack.pop_back()
        return True
    
    cdef bool peek(self, string *ptr):
        if self.empty():
            return False
        ptr[0] = self._stack.back()
        return True

    cdef bool empty(self):
        return self._stack.empty()
    
    
    cdef bool full(self):
        return self._stack.size() == self._stack.max_size()
    
    def __reduce__(self):
        return (Stack, ())


# 创建类的实例
cdef Stack s
s = Stack()
cdef string elem
cdef string *ptr
ptr = &elem

# 给栈压入元素
s.push(b"foo")
s.push(b"bar")
s.push(b"baz")
print("栈元素包含:", s._stack)

# 让指针ptr指向栈顶元素
s.peek(ptr)
print("栈顶元素为:", elem)
```

    栈元素包含: [b'foo', b'bar', b'baz']
    栈顶元素为: b'baz'


### 4.2 什么是构造函数和析构函数
在Cython中，通过`__cinit__`函数对类的实例进行构造，`__dealloc__`函数对类的实例进行析构。`__cinit__`与`__init__`的区别请参考:  
https://docs.cython.org/en/latest/src/userguide/special_methods.html#initialisation-methods   

我们先通过cdef定义出Matrix的类成员_name, row, col和指向Vector的指针matrix。然后定义`__cinit__`函数初始化这些类成员，其中matrix使用了new关键字。C用malloc，C++用`new`分配的C变量是动态分配的(堆分配的)。它的生命周期是直到用户明确删除它为止(在C中为free，在C++中为del)。接下来实现了`__dealloc__`函数，通过`del`关键字释放在堆内存上的vector。最后创建一个`show`函数打印Vector中的元素和Vector的地址。  
注意：  
1. uintptr_t可以正确地把指针的地址转为int类型，此处如果写成`<int>`会报错。
2. 如果mat是在file scope定义的全局变量，则不能用del进行删除。  
3. 通过property装饰器方便Python访问_name对象，因为_name会被转为Python的bytes型，而我们希望转为str类型。  

我们通过`cdef Matrix mat_copy = mat`创建一个mat的副本，需要查看mat_copy对象的matrix成员地址与mat对象的matrix成员地址是否相同。如果我们删除了mat之后，mat的matrix指针被释放，而mat_copy的matrix指针仍然还在指向这快内存，会引发不确定的错误。  
通过定义__copy__方法，可以让Matrix类与python的copy.copy方法兼容。  


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport uintptr_t
from cython.operator cimport dereference as deref, preincrement as inc
from copy import copy


cdef class Matrix:
    cdef int row, col
    cdef vector[int]* matrix
    cdef string _name

    def __cinit__(self, name, int row, int col):
        self._name = bytes(name, "utf8")
        self.row = row
        self.col = col
        self.matrix = new vector[int](row * col, 0)
        print(f"{name}的constructor执行完毕!")
        print()
    
    @property
    def name(self):
        return self._name.decode('utf8')

    def __dealloc__(self):
        del self.matrix
        print(f"{self.name}的destructor执行完毕!")
        print()
    
    def __copy__(self):
        cdef Matrix mat_copy
        mat_copy = Matrix(self.name, self.row, self.col)
        cdef int i
        for i in range(self.matrix[0].size()):
            mat_copy.matrix[0][i] = self.matrix[0][i]
        return mat_copy

    def show(self):
        print(f"{self.name}的元素为: {deref(self.matrix)}")
        cdef uintptr_t address
        address = <uintptr_t> self.matrix
        print(f"{self.name}的内存地址为: {address}")
        print()

def demo():
    cdef uintptr_t address
    # 创建mat对象
    cdef Matrix mat = Matrix("matrix", 2, 5)
    # 为mat的元素赋值
    print("为mat的元素赋值")
    mat.matrix[0] = list(range(10))
    # 打印mat的相关信息
    mat.show()
    # 创建mat对象的副本
    cdef Matrix mat_copy = copy(mat)
    # 打印mat副本的相关信息
    mat_copy.show()
    # 打印mat的地址
    print(f"mat对象的地址为{id(mat)}")
    print(f"mat_copy对象的地址为{id(mat_copy)}")
    print()
    # 删除mat对象
    del mat
    
demo()
```

    matrix的constructor执行完毕!
    
    为mat的元素赋值
    matrix的元素为: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    matrix的内存地址为: 140434756720368
    
    matrix的constructor执行完毕!
    
    matrix的元素为: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    matrix的内存地址为: 140434756648048
    
    mat对象的地址为4624972016
    mat_copy对象的地址为4624969832
    
    matrix的destructor执行完毕!
    
    matrix的destructor执行完毕!
    


### 4.3 何谓mutable和const?
我们需要计算Stack中最长的字符串有多长，设计一个max_elem_len函数，返回这个最大值。我们通过如下代码定义一个Stack。 
```Cython
cdef class MyStack:
    pass
```
编写代码`cdef int max_elem_len(const MyStack &s)`会编译报错，因为Stack是cdef cppclass生成的C++对象，而MyStack是cdef class生成的Python对象，Python对象不能用const Stack &s这样的语法，我们需要注意这两者的区别。正确的方法是用C++写一个Stack类，再通过.pxd文件和.pyx文件extern过来(对应C++的.h文件和.cpp文件)。代码如下 
```Cython
# distutils: language = c++
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "chapter_4_extern.h":
    cdef cppclass Stack:
        bool empty()
        bool pop(string &elem)
        bool full()
        bool peek(string &elem)
        bool push(const string &elem)
        const string *next() const
        void reset_next() const
```
Stack前跟了const关键字，那么照理说max_elem_len函数编译不会通过，因为调用了Stack的类成员函数不是const，无法保证这些函数不去修改Stack,也就无法保证Stack是const的。那么为什么最终编译通过了呢，我们通过mutable和const关键字解决了这个问题。运行如下代码，简单测试得到了正确的结果。  
注意：我们需要建立一个`chapter_4.pyx`文件，并通过`python setup.py build_ext --inplace`命令编译为`chapter_4.so`文件。因为chapter_4.pxd文件中不完全是extern过来的class，还有自定义的class，所以必须要这样做才能保证我们的代码正确运行。详见：https://cython.readthedocs.io/en/latest/src/userguide/sharing_declarations.html


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_4 cimport Stack
```


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_4 cimport Stack
from libcpp.string cimport string
from libcpp.vector cimport vector
from cython.operator cimport dereference as deref


cdef int max_elem_len(const Stack &s):
    cdef:
        const string* ptr
        int max_len = 0
        int cur_len = 0
    while 1:
        ptr = s.next()
        if ptr == NULL:
            break
        cur_len = deref(ptr).size()
        max_len = cur_len if cur_len > max_len else max_len
    s.reset_next()
    return max_len

cdef Stack s1
s1.push(b"ab")
s1.push(b"a")
s1.push(b"abc")
print(f"Maximum element length is {max_elem_len(s1)}!")
```

    Maximum element length is 3!


如下是Stack类的C++代码实现的其中一部分，省略了未被调用的方法。完整代码请参考`chapter_4_extern.cpp`文件。注意我们把next和reset_next方法定义的后面都跟了一个关键字`const`，这个关键字确保我们在方法内部不会修改Stack的值，但是我们确实需要修改_pos的值，所以需要把_next定义为mutable，意思是_next的值即便被更改了，依然认为整个类没有被更改。代码如下
```C++
#include <vector>
#include <string>
using namespace std;

class Stack
{
private:
    vector<string> _stack;
    mutable int _pos = -1;

public:
    const string *next() const
    {
        if (++_pos == _stack.size())
        {
            return NULL;
        }
        return &_stack[_pos];
    }

    void reset_next() const
    {
        _pos = -1;
    }
};
```

### 4.4 什么是this指针
我们需要设计一个copy_from()成员函数，实现Stack的拷贝。伪代码如下
```Cython
cdef Stack s1
cdef Stack s2
s2.push(b"foo")
s2.push(b"bar")
s1.copy_from(s2)  # 返回s1
```
想要把一个对象复制给另一个对象，需要先确定两个对象是否相同。我们如何在copy_from方法里面把s1和s2进行对比呢？这就需要用到this指针。this指针是在member function内用来指向其调用者。本例中，this指向s1。有点类似Python的self，但在C++类成员函数的定义中，形参不需要写this，编译器会帮助我们完成这部分工作。C++代码如下
```C++
bool copy_from(const Stack &s)
{
    if (this == &s)
    {
        return false;
    }
    _stack = s._stack;
    _pos = s._pos;
    return true;
}
```
如果拷贝成功了，返回true，否则返回false。运行如下代码测试copy_from功能，通过了测试。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_4 cimport Stack
from libcpp.string cimport string
from libcpp cimport bool
from cython.operator cimport dereference as deref

cdef:
    Stack s1
    Stack s2
    bool flag
    string elem
    const string* ptr

s2.push(b"foo")
s2.push(b"bar")
s2.push(b"baz")

flag = s1.copy_from(s2)
print(f"s2拷贝到s1, {'成功' if flag else '失败'}!")

flag = s2.copy_from(s2)
print(f"s2拷贝到s2, {'成功' if flag else '失败'}!")

print("s1的元素为:", end=" ")
while 1:
    ptr = s1.next()
    if ptr == NULL:
        s1.reset_next()
        break
    elem = deref(ptr)
    print(elem.decode('utf8'), end=", ")

print()
    
print("s2的元素为:", end=" ")
while 1:
    ptr = s2.next()
    if ptr == NULL:
        s2.reset_next()
        break
    elem = deref(ptr)
    print(elem.decode('utf8'), end=", ")
```

    s2拷贝到s1, 成功!
    s2拷贝到s2, 失败!
    s1的元素为: foo, bar, baz, 
    s2的元素为: foo, bar, baz, 

### 4.5 静态类成员
static(静态)data member用来表示唯一的、可在类的实例之间共享的member。对于class而言，static data member只有唯一的一份实体，我们需要加上`static`关键字进行定义。如果类成员函数只访问静态的类成员，那么这个函数也可以被定义为static。下面我们通过静态方法查看Stack实例的数量，其实就是维护一个被各个实例共享的变量，每调用一次constructor，变量加1，调用一次destructor，变量减1。实现的过程如下:

#### 4.5.1 在C++中声明Stack
首先在`chapter_4_extern.h`头文件中声明Stack和我们需要的方法。
```C++
class Stack
{
public:
    static int n_instance;
    Stack();
    ~Stack();
    static int count_instances();
```

#### 4.5.2 在C++中定义Stack
在`chapter_4_extern.cpp`文件编写如下代码，使得我们创建一个Stack实例之后n_instance会加一，实例被销毁之后n_instance会减一。注意`int Stack::n_instance = 0;`必须定义在Stack外面，也不能写在头文件里，否则会编译报错。
```C++
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
```

#### 4.5.3 在Cython中声明Stack
在`chapter_4.pxd`文件编写如下代码，注意我们只需要把constructor在这里声明，destructor会被Cython自动extern过来，不需要也没办法声明，声明之后会报错。使用`@staticmethod`装饰器可以声明C++的静态方法。
```Cython
cdef extern from "chapter_4_extern.h":
    cdef cppclass Stack:
        Stack() except +
        @staticmethod
        int count_instances()
```

#### 4.5.4 在Cython中编写函数进行测试
在`chapter_4.pyx`文件编写如下代码，`# distutils: sources = chapter_4_extern.cpp`可以让我们在编译的时候对`chapter_4_extern.cpp`文件做静态链接，详见[specify-c-language-in-setup-py](http://docs.cython.org/en/latest/src/userguide/wrapping_CPlusPlus.html?highlight=destructor#specify-c-language-in-setup-py)。`cdef Stack s4`并不会调用Stack的constructor，这点让人很奇怪，而调用s4的某个方法之后就会触发constructor。在这里被坑了好久，让我默默哭一会儿。
```Cython
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

```
运行测试结果如下:


```python
from chapter_4 import test_count_instances

test_count_instances()
```

    There are 7 instances of Stack!


### 4.6 打造一个Iterator Class
为了说明如何对class进行运算符重载操作，让我们体验一下如何实现一个iterator class，我们必须提供以下操作方式：
```Cython
from cython.operator cimport dereference as deref, preincrement as inc

cdef Stack s
s.push("foo")
s.push("bar")
s.push("baz")

cdef:
    Stack::iterator it = s.begin()
    Stack::iterator end = s.end()

while it != end:
    print(deref(it))
    inc(it)
```

为了让上述程序代码得以运行，我们必须为此iterator class定义`!=, *, ++`等运算符。与定义member functions的唯一差别就是不需要指定方法名称，只需要在运算符符号之前加上关键词operator即可。  

#### 4.6.1 begin()和end()方法
首先，我们需要为`Stack`添加`begin`和`end`方法，返回类型为`StackIterator`，由于`StackIterator`还没有被声明，所以我们需要通过前向声明的方式让编译器知道`StackIterator`的存在。由于这两个方法只是为了遍历Stack，而不会做任何修改，所以需要把方法声明为`const`。在`chapter_4_extern.h`头文件中编写代码如下：
```C++
//Forward declaration.
class StackIterator;

class Stack
{
    friend class StackIterator;

public:
    StackIterator begin() const;
    StackIterator end() const;
};
```

声明`StackIterator`和`Stack::begin`和`Stack::end`之后，需要对方法进行定义，方法的返回值就是一个`StackIterator`的实例，包含了Stack的信息`this`指针和下标位置。在`chapter_4_extern.cpp`文件中编写代码如下：
```C++
StackIterator Stack::begin() const
{
    return StackIterator(*this, 0);
}

StackIterator Stack::end() const
{
    int index = _elem.size();
    return StackIterator(*this, index);
}
```

#### 4.6.2 实现StackIterator
`chapter_4_extern.h`头文件中编写代码如下：  
定义`==`操作符，只要两个Iterator的下标相等即可，我们不需要判断两个Iterator指向的Stack是否一致。
```C++
bool operator==(const StackIterator &rhs) const
{
    return _index == rhs._index;
};
```

定义`!=`操作符，只需要调用刚刚定义好的`==`操作符，再取`!`操作即可。
```C++
bool operator!=(const StackIterator &rhs) const
{
    return !(*this == rhs);
};
```

定义`*`操作符，返回Stack对应位置的元素。
```C++
string operator*() const
{
    check_integrity();
    return _stack._elem[_index];
};
```

定义前置`++`操作符，把`_index`加1，之后需要检查是否数组越界，最后返回当前的`StackIterator`实例。
```C++
StackIterator &operator++()
{
    ++_index;
    check_integrity();
    return *this;
}; //prefix
```
定义后置`++`操作符，预先拷贝一份`StackIterator`的实例，把`_index`加1，之后需要检查是否数组越界，最后返回拷贝的实例。
```C++
StackIterator operator++(int)
{
    StackIterator tmp = *this;
    ++_index;
    check_integrity();
    return tmp;
}; //postfix, needs an unchanged object after operation.
```

最后要定义`StackIterator`的构造方法，注意只能用成员列表的方式来定义。`chapter_4_extern.cpp`文件中编写代码如下：  
```C++
StackIterator::StackIterator(const Stack &rhs, int index) : _stack(rhs), _index(index)
```

#### 4.6.3 测试StackIterator是否正常工作
我们无法通过`Cython`来导入`StackIterator`类，再测试`begin`和`end`是否正常工作，因为如果一个`C++ class`的构造函数带有参数的话，是没办法被Cython直接调用的。详细内容可以参考如下链接：  
`C++ class must have a nullary constructor to be stack allocated!`，https://azhpushkin.me/posts/cython-cpp-intro  

所以我们暂时只能够用`C++`来编写测试函数，代码如下：
```C++
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
```

我们在`chapter_4.pxd`文件中编写如下代码，将这个测试函数导入。
```Cython
cdef extern from "chapter_4_extern.h":
    string test_iterator()
```
测试结果如下，我们自己实现的`Iterator`成功遍历了`Stack`，与预期的一致。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_4 cimport test_iterator

print(test_iterator())
```

    b'foo, bar, baz, '



```python

```
