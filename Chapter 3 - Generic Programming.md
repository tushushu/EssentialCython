## 3. 泛型编程风格
作者: tushushu  
项目地址: https://github.com/tushushu/EssentialCython   
参考书籍: 《Essential C++ 中文版》  
编程环境: MacOS + Jupyter Notebook + Python 3.6


```python
%load_ext Cython
```

### 3.1 指针的算术运算
#### 3.1.1 array作为形参的特性
当我们写下如下函数
```C++
int min(int array[24]);
```
min()似乎仅能接受某个拥有24个元素的array，并且以传值的方式传入。事实上这两个假设都是错的：array并不会以传值的方式被复制一份，而且我们可以传递任意大小的array给min()。当数组被传给函数，或是由函数返回，仅有第一个元素的地址会被传递。下面我们来写代码验证这一点：


```cython
%%cython
print("声明一个min函数，形参为长度为24的数组")

cdef int cmin(int array[24]):
    print("array的大小为", sizeof(array))
    cdef:
        int n = 8
        int i
        int res
    if n == 0:
        return 0
    res = array[0]
    for i in range(n):
        if array[i] < res:
            res = array[i]
    # 我们把传入的array的首个元素改为666
    array[0] = 666
    return res

cdef int array[6]
array[:] = [8, 2, 1, 3, 6, 7]
print("定义长度为6的数组", array)
print()
res = cmin(array)
print("array的大小本应该是6 * 4 = 24，证明传入函数的是首元素的指针而不是array!")
print()
print("运算结果为", res)
print("函数正确运行，证明我们可以传递任意大小的array给min()!")
print()
print("打印刚才定义的长度为6的数组", array)
print("数组的值被更改，证明array并不会以传值的方式被复制一份!")
```

    声明一个min函数，形参为长度为24的数组
    定义长度为6的数组 [8, 2, 1, 3, 6, 7]
    
    array的大小为 8
    array的大小本应该是6 * 4 = 24，证明传入函数的是首元素的指针而不是array!
    
    运算结果为 1
    函数正确运行，证明我们可以传递任意大小的array给min()!
    
    打印刚才定义的长度为6的数组 [666, 2, 1, 3, 6, 7]
    数组的值被更改，证明array并不会以传值的方式被复制一份!


#### 3.1.2 指针与数组
现代编译器对于数组的访问都会自动优化为其对应的指针加偏移量的形式。虽然array是以第一个元素的指针传入cmin()中，但仍然可以通过下标的方式访问元素。因为所谓的下标操作就是将array的起始地址加上索引值，产生出某个元素的地址，然后该地址被dereference再返回元素值。我们现在写代码验证一下：  

注意：cython通过`[0]或者from cython.operator cimport dereference as deref`的方式来解引用，而不能像C/C++一样直接用星号来操作。


```cython
%%cython
from cython.operator cimport dereference as deref
from libc.stdint cimport uintptr_t

cdef uintptr_t address
print("创建地址变量address")
print()

cdef int array[8]
array[:] = [1, 2, 3, 4, 5, 6, 7, 8]
print("创建长度为8的数组array", array)
address = <uintptr_t>array
print("打印array的地址为", address)
print()

cdef int *ptr = &array[0]
print("创建指针ptr指向array的首个元素")
address = <uintptr_t>ptr
print("打印ptr的地址为", address)
print()

print("两者的地址一样!")
print()

address = <uintptr_t>(ptr + 1)
print("打印ptr + 1的地址为", address)
print("指针算术运算，会把指针所指类型的大小考虑进去。")
print()

print("通过下标的方式访问指针指向的下一个元素为:", deref(ptr))
print("通过指针加法的方式访问指针指向的下一个元素为:", deref(ptr + 1))
print("通过array指针加法的方式访问下一个元素为:", (array + 1)[0])
```

    创建地址变量address
    
    创建长度为8的数组array [1, 2, 3, 4, 5, 6, 7, 8]
    打印array的地址为 4569814592
    
    创建指针ptr指向array的首个元素
    打印ptr的地址为 4569814592
    
    两者的地址一样!
    
    打印ptr + 1的地址为 4569814596
    指针算术运算，会把指针所指类型的大小考虑进去。
    
    通过下标的方式访问指针指向的下一个元素为: 1
    通过指针加法的方式访问指针指向的下一个元素为: 2
    通过array指针加法的方式访问下一个元素为: 2


#### 3.1.3 实现一个泛型算法
假设我们需要完成以下工作。给定一个储存任意类型元素的vector或array，以及一个元素。如果此元素存在于vector或array内，我们必须返回一个指针指向该值；反之则返回0。
编写如下C++代码，实现find函数。
```C++
template <typename T>
T *find(const T *first, const T *last, const T &target)
{
    if (!first || !last)
    {
        return 0;
    }
    for (; first != last; ++first)
    {
        if (*first == target)
        {
            return first;
        }
    }
    return 0;
}
```
然后像以前一样，编写.pxd文件导入这个函数可以被Cython调用，可能是Cython对C++的支持问题，使用C++测试的时候没问题，但Cython代码会报错。
更改一下实现方式，让函数返回数组或者vector的下标，而不是一个指针。
```C++
template <typename T>
int find(const T *first, int size, const T target)
{
    if (!first)
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
```
下面写代码测试一下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_3 cimport find
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef int size

# 测试0: int array, 不可找到target
size = 8
cdef int itarget = 10
cdef int iarray[8]
iarray[:] = [1, 2, 3, 4, 5, 6, 7, 8]
print("target的下标为:", find(iarray, size, itarget))

# 测试1: int array, 可找到target
itarget = 4
print("target的下标为:", find(iarray, size, itarget))

# 测试2: float array, 可找到target
size = 4
cdef float ftarget = 2.0
cdef float farray[4]
farray[:] = [1.0, 2.0, 3.0, 4.0]
print("target的下标为:", find(farray, size, ftarget))

# 测试3: double array, 可找到target
size = 5
cdef double dtarget = 3.0
cdef double darray[5]
darray[:] = [1.0, 2.0, 3.0, 4.0, 5.0]
print("target的下标为:", find(darray, size, dtarget))

# 测试4: string array, 可找到target
size = 6
cdef string starget = b"hi"
cdef string sarray[6]
sarray[:] = [b"abc", b"d", b"ef", b"g", b"hi", b"jkl"]
print("target的下标为:", find(sarray, size, starget))

# 测试5: int vector, 可找到target
cdef vector[int] vec = [1, 2]
itarget = 2
size = vec.size()
print("target的下标为:", find(&vec[0], size, itarget))

# 测试6: int vector, 空vector
vec = []
itarget = 2
size = vec.size()
print("target的下标为:", find(&vec[0], size, itarget))
```

    target的下标为: -1
    target的下标为: 3
    target的下标为: 1
    target的下标为: 2
    target的下标为: 4
    target的下标为: 1
    target的下标为: -1


### 3.2 了解Iterator(泛型指针)
每个标准容器都提供一个名为begin()的操作函数，可返回一个iterator，指向第一个元素。另一个名为end()的操作函数会返回一个iterator，指向最后一个元素的下一个位置。以下是标准库iterator语法，iter指向一个vector，后者的元素类型为string。双冒号表示此iterator是位于string vector定义内的嵌套类型。  
C++  
```C++
vector<string>::iterator iter = svec.begin();
```
Cython
```Cython
vector[string].iterator it = svec.begin()
```
想要通过iterator取得元素值，可以采用跟一般指针一样的操作方式。  
如果想让3.1的find函数可以作用于list上，使用普通的指针是不可以的，因为list不是一段连续的内存。这时候就得用标准容器的iterator。重新定义find函数代码如下(注意这里不要用函数重载，否则因为编译器无法识别出两个find)：
```C++
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
```
const 修饰指针变量有以下三种情况。  

A: const 修饰指针指向的内容，则内容为不可变量。  
```C++ 
const int *p = 8;
```  
B: const 修饰指针，则指针为不可变量。  
```C++
int* const p = 8;
```  
C: const 修饰指针和指针指向的内容，则指针和指针指向的内容都为不可变量。  
```C++
const int* const p = 8;
```

对于 const 修饰函数参数可以分为三种情况:  
A：值传递的 const 修饰传递，一般这种情况不需要 const 修饰，因为函数会自动产生临时变量复制实参值。  
B：当 const 参数为指针时，可以防止指针被意外篡改。  
C：自定义类型的参数传递，需要临时对象复制参数，对于临时对象的构造，需要调用构造函数，比较浪费时间，因此我们采取 const 外加引用传递的方法。并且对于一般的 int、double 等内置类型，我们不采用引用的传递方式。  

编写.pxd文件导入这个函数可以被Cython调用，测试代码如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_3 cimport find2
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.list cimport list as clist


# 测试0: int array, 不可找到target
cdef int itarget = 10
cdef int iarray[8]
iarray[:] = [1, 2, 3, 4, 5, 6, 7, 8]
print("target的下标为:", find2(iarray, iarray+8, itarget))

# 测试1: int array, 可找到target
itarget = 4
print("target的下标为:", find2(iarray, iarray+8, itarget))

# 测试2: float array, 可找到target
cdef float ftarget = 2.0
cdef float farray[4]
farray[:] = [1.0, 2.0, 3.0, 4.0]
print("target的下标为:", find2(farray, farray+4, ftarget))

# 测试3: double array, 可找到target
cdef double dtarget = 3.0
cdef double darray[5]
darray[:] = [1.0, 2.0, 3.0, 4.0, 5.0]
print("target的下标为:", find2(darray, darray+5, dtarget))

# 测试4: string array, 可找到target
cdef string starget = b"hi"
cdef string sarray[6]
sarray[:] = [b"abc", b"d", b"ef", b"g", b"hi", b"jkl"]
print("target的下标为:", find2(sarray, sarray+6, starget))

# 测试5: int vector, 可找到target
cdef vector[int] vec = [1, 2]
itarget = 2
print("target的下标为:", find2(vec.begin(), vec.end(), itarget))

# 测试6: int vector, 空vector
vec = []
itarget = 2
print("target的下标为:", find2(vec.begin(), vec.end(), itarget))

# 测试7: int list, 可找到target
cdef clist[int] ilist = [1, 2, 3]
print("target的下标为:", find2(ilist.begin(), ilist.end(), itarget))

```

    target的下标为: -1
    target的下标为: 3
    target的下标为: 1
    target的下标为: 2
    target的下标为: 4
    target的下标为: 1
    target的下标为: -1
    target的下标为: 1


### 3.3 所有容器的共通操作
下列为所有容器类（以及string类）的共通操作：
1. equality(==)和inequality(!=)运算符，返回true或false。
2. assignment(=)运算符，将某个容器复制给另一个容器。
3. empty()方法会在容器无任何元素时返回true，否则返回false。
4. clear()方法删除所有元素。
5. begin()方法返回一个iterator，指向容器的第一个元素。
6. end()方法返回一个iterator，指向容器的最后一个元素的下一个位置。
7. insert()方法将单一或某个范围内的元素插入容器内
8. erase()方法将容器内的单一元素或某个范围内的元素删除。

以vector为例演示上述操作如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from cython cimport typeof

print("1. equality(==)和inequality(!=)运算符，返回true或false。")
cdef vector[int] v1 = [1, 2, 3]
cdef vector[int] v2 = [1, 2, 3, 4]
cdef vector[int] v3 = [1, 2, 3, 4]
print(f"v1 == v2 is {v1 == v2}")
print(f"v2 == v3 is {v2 == v3}")
print(f"v1 != v3 is {v1 != v3}")
print()

print("2. assignment(=)运算符，将某个容器复制给另一个容器。")
print(f"Before assignment v3 is {v3}!")
v3 = v1
print(f"After assignment v3 is {v3}!")
print()

print("3. empty()方法会在容器无任何元素时返回true，否则返回false。")
cdef vector[int] v4
print(f"v1 is {v1} and v1.empty() is {v1.empty()}!")
print(f"v4 is {v4} and v4.empty() is {v4.empty()}!")
print()

print("4. clear()方法删除所有元素。")
print(f"Before clear v3 is {v3}!")
print(f"After clear v3 is {v3}!")
print()

print("5. begin()方法返回一个iterator，指向容器的第一个元素。")
print(f"The type of v1.begin() is {typeof(v1.begin())}")
print()

print("6. end()方法返回一个iterator，指向容器的最后一个元素的下一个位置。")
print(f"The type of v1.end() is {typeof(v1.end())}")
print()

print("7. insert()方法将单一或某个范围内的元素插入容器内。")
print(f"Before insertion, v4 is {v4}!")
v4.insert(v4.begin(), 1)
v4.insert(v4.begin(), 2)
v4.insert(v4.begin(), 3)
v4.insert(v4.begin() + 1, 4)
v4.insert(v4.end(), 5)
print(f"After insertion, v4 is {v4}!")
print()

print("8. erase()方法将容器内的单一元素或某个范围内的元素删除")
print(f"Before erasing, v4 is {v4}!")
v4.erase(v4.begin() + 1)
v4.erase(v4.end() - 1)
print(f"After erasing, v4 is {v4}!")
print()
```

    1. equality(==)和inequality(!=)运算符，返回true或false。
    v1 == v2 is False
    v2 == v3 is True
    v1 != v3 is True
    
    2. assignment(=)运算符，将某个容器复制给另一个容器。
    Before assignment v3 is [1, 2, 3, 4]!
    After assignment v3 is [1, 2, 3]!
    
    3. empty()方法会在容器无任何元素时返回true，否则返回false。
    v1 is [1, 2, 3] and v1.empty() is False!
    v4 is [] and v4.empty() is True!
    
    4. clear()方法删除所有元素。
    Before clear v3 is [1, 2, 3]!
    After clear v3 is [1, 2, 3]!
    
    5. begin()方法返回一个iterator，指向容器的第一个元素。
    The type of v1.begin() is iterator
    
    6. end()方法返回一个iterator，指向容器的最后一个元素的下一个位置。
    The type of v1.end() is iterator
    
    7. insert()方法将单一或某个范围内的元素插入容器内。
    Before insertion, v4 is []!
    After insertion, v4 is [3, 4, 2, 1, 5]!
    
    8. erase()方法将容器内的单一元素或某个范围内的元素删除
    Before erasing, v4 is [3, 4, 2, 1, 5]!
    After erasing, v4 is [3, 2, 1]!
    


### 3.4 使用顺序性容器

顺序性容器用来维护一组排列有序、类型相同的元素，其中vector和list是两类最主要的顺序性容器。
1. vector以一块连续内存来存放元素。对vector进行随机下标访问效率为O(1)，随机位置插入或删除元素的效率为O(n)；
2. list以双向链接、非连续的内存来存放元素，每个元素都包含元素值，back指针和front指针。对list进行随机下标访问效率为O(n), 随机下标插入或删除元素的效率为O(1)。  

在Cython中定义C++顺序性容器对象的方式如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from libcpp.list cimport list as cpp_list  # 注意不要跟Python的list搞混
from libcpp.string cimport string


# 1. 产生空的容器
cdef cpp_list[string] slist
cdef vector[int] ivec;
print(f"1. slist is: {slist}")
print(f"1. ivec is: {ivec}")
print()

# 2. 产生特定大小的容器，并为每个元素指定初值
cdef cpp_list[int] ilist = cpp_list[int](8, -1)
cdef vector[string] svec = vector[string](4, b"unassigned")
print(f"2. ilist is: {ilist}")
print(f"2. svec is: {svec}")
print()

# 3. 通过C++容器定义
cdef vector[float] fvec1 = vector[float](4, 0)
cdef vector[float] fvec2 = vector[float](fvec1)
print(f"3. fvec1 is: {fvec1}")
print(f"3. fvec2 is: {fvec2}")
print()

# 4. 通过Python可迭代对象定义
cdef vector[double] dvec = [1.0, 2.0, 3.0, 4.0]
cdef cpp_list[float] flist = map(float, range(6))

print(f"4. dvec is: {dvec}")
print(f"4. flist is: {flist}")
```

    1. slist is: []
    1. ivec is: []
    
    2. ilist is: [-1, -1, -1, -1, -1, -1, -1, -1]
    2. svec is: [b'unassigned', b'unassigned', b'unassigned', b'unassigned']
    
    3. fvec1 is: [0.0, 0.0, 0.0, 0.0]
    3. fvec2 is: [0.0, 0.0, 0.0, 0.0]
    
    4. dvec is: [1.0, 2.0, 3.0, 4.0]
    4. flist is: [0.0, 1.0, 2.0, 3.0, 4.0, 5.0]


### 3.5 使用泛型算法
想要使用泛型算法，首先得包含对应的algorithm头文件，在C++里，使用`#include <algorithm>`，在Cython里，使用`cimport libcpp.algorithm`。让我们以vector来存储数列，以此练习泛型算法的运用。如果给定的值已存储于数列之中，is_elem()必须返回true；否则返回false。下面为四种可能被我们采用的泛型搜索算法：
1. find()用于搜索无需集合中是否存在某值；
2. binary_search()用于有序集合的搜索；
3. count()返回等于该数值的数量；
4. search()用于搜索序列中的子序列。

由于我们的vector必定以递增顺序储存其值，因此binary_search()是我们的最佳选择。Cython代码实现如下，注意提前终止查找，避免不必要的时间浪费：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.algorithm cimport binary_search
from libcpp.vector cimport vector
from libcpp cimport bool

cdef bool has_elem(vector[int] &vec, int elem):
    if vec.empty():
        return False
    if elem < vec[0]:
        return False
    if elem > vec[vec.size() - 1]:
        return False
    return binary_search(vec.begin(), vec.end(), elem)

cdef:
    vector[int] v = [1, 2, 3, 4, 5]
    bool res
    int elem

# 1. 中间值
elem = 3
res = has_elem(v, elem)
print(f"在vector - {v}中查找元素{elem}，结果为{res}!")

# 2. 最大值
elem = 5
res = has_elem(v, elem)
print(f"在vector - {v}中查找元素{elem}，结果为{res}!")

# 3. 最小值
elem = 1
res = has_elem(v, elem)
print(f"在vector - {v}中查找元素{elem}，结果为{res}!")

# 4. 超出左边界
elem = 0
res = has_elem(v, elem)
print(f"在vector - {v}中查找元素{elem}，结果为{res}!")

# 6. 超出右边界
elem = 6
res = has_elem(v, elem)
print(f"在vector - {v}中查找元素{elem}，结果为{res}!")
```

    在vector - [1, 2, 3, 4, 5]中查找元素3，结果为True!
    在vector - [1, 2, 3, 4, 5]中查找元素5，结果为True!
    在vector - [1, 2, 3, 4, 5]中查找元素1，结果为True!
    在vector - [1, 2, 3, 4, 5]中查找元素0，结果为False!
    在vector - [1, 2, 3, 4, 5]中查找元素6，结果为False!


### 3.6 如何设计一个泛型算法
用户给予一个整数vector，我们必须返回一个新的vector，其中内含原vector之中大于，等于或小于`target`的所有数值，我们允许用户指定不同的比较操作。我们需要用到如下标准库的函数或函数对象：
1. 泛型函数find_if，容器内的元素会被一一施以特定的二元运算，测试是否符合条件。如果找到符合条件的元素，搜索操作便结束，并返回一个iterator指向该元素。如果没有找到符合条件的元素，就返回容器的end();
2. C++`<functional>`头文件中的std::less, std::less_equal, greater, greater_equal, equal_to和not_equal_to等6个函数对象，函数对象有点类似于Python中带有`__call__`方法的类，其实例可以当作函数使用。
3. C++`<functional>`头文件中的std::bind2nd，作用是绑定二元函数的第二个参数，避免反复传参浪费时间。 

我们编写.pxd文件来extern C++的less和greater函数对象，其中`T=*`的星号表示可选参数，`less() except +`让constructor的错误由Python来handle。
```Cython
cdef extern from "<functional>" namespace "std" nogil:
    cdef cppclass less[T=*]:
        less() except +
        bool operator()(const T& lhs, const T& rhs) except +

    cdef cppclass greater[T=*]:
        greater() except +
        bool operator()(const T& lhs, const T& rhs) except +

```

由于Cython对泛型算法的支持很有限，我们需要编写C++代码如下：
```C++
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
```
测试filter函数，可以作用在vector, list等不同容器类型，以及int, float等不同元素类型，和less greater等不同operater函数上。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_3 cimport filter as cfilter, greater
from libcpp.vector cimport vector

cdef:
    vector[int] ivec = [1, 2, 3, 4, 5]
    int target = 3
    vector[int] output = vector[int](5, -1)


cfilter(ivec.begin(), ivec.end(), output.begin(), target, greater[int]())
print(output)    
```

    [4, 5, -1, -1, -1]



```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_3 cimport filter as cfilter, less
from libcpp.list cimport list as clist

cdef:
    clist[float] flist = [1, 2, 3, 4, 5]
    float target = 4
    clist[float] output = clist[float](5, -1)


cfilter(flist.begin(), flist.end(), output.begin(), target, less[float]())
print(output)    
```

    [1.0, 2.0, 3.0, -1.0, -1.0]


### 3.7 使用Map
Map被定义为一对数值，其中的key通常是个字符串，扮演索引的角色，另一个数值是value。字典便是map的一个不错的实例，比如统计文章中单词的词频。C++中的`map`可以替代Python的`dict`，严格的说，应该是C++中的`unordered_map`。两者的区别如下：  
 
|Differences|map|unordered_map|
|------|------|------| 
|Ordering|increasing  order(by default)|no ordering|
|Implementation|Self balancing BST|Hash Table|
|Search time|log(n)|O(1) -> Average, O(n) -> Worst Case|
|Insertion time|log(n) + Rebalance| Same as search|
|Deletion time|log(n) + Rebalance| Same as search|

因为`map`内的元素是有序的，Python用户通常需要的是无序的map。在Cython中使用`from libcpp.unordered_map cimport unordered_map`语句来导入`unordered_map`类。使用方法如下：
1. 初始化 - 通过Python的可迭代对象进行初始化，需要声明变量的嵌套类型
2. 遍历 - 让泛型指针自增，通过while循环进行遍历
3. 访问 - 使用deref(C++中的'*'操作符)来解引用，返回pair对象，通过.first来访问key, .second来访问Value
4. 查找 - 使用unordered_map.count，返回1或0；或者用unordered_map.find，返回一个泛型指针，如果指针指向unordered_map.end，则表示未找到。
5. 追加/修改 - unordered_map[key] = value。如果Key不存在，'[]'操作符会添加一个Key，并赋值为默认的Value，比如0.0。所以，除非确定不会产生错误，否则在修改Key对应的Value之前，要先判断Key是否存在。这与Python的DecaultDict有点相似。  

上述操作的代码演示如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.unordered_map cimport unordered_map


# 通过Python对象初始化
cdef unordered_map[int, float] mymap = {i: i/10 for i in range(10)}

# 遍历
cdef:
    unordered_map[int, float].iterator it = mymap.begin()
    unordered_map[int, float].iterator end = mymap.end()
print("开始遍历...")
while it != end:
    # 访问
    print("\tKey is %d, Value is %.1f" % (deref(it).first, deref(it).second))
    inc(it)
print()

# 查找
print("开始查找...")
if mymap.count(-2):
    print("\t元素-2存在!")
else:
    print("\t元素-2不存在!")

it = mymap.find(3)
if it != end:
    print("\t元素3存在, 它的值是%.1f!" % deref(it).second)
else:
    print("\t元素3不存在!")
print()

# 修改
print("修改元素...")
if mymap.count(3):
    mymap[3] += 1.0
mymap[-2]  # Key -2不存在，会被添加一个默认值0.0
print("\tKey is 3, Value is %.1f" % mymap[3])
print("\tKey is -2, Value is %.1f" % mymap[-2])
```

    开始遍历...
    	Key is 0, Value is 0.0
    	Key is 1, Value is 0.1
    	Key is 2, Value is 0.2
    	Key is 3, Value is 0.3
    	Key is 4, Value is 0.4
    	Key is 5, Value is 0.5
    	Key is 6, Value is 0.6
    	Key is 7, Value is 0.7
    	Key is 8, Value is 0.8
    	Key is 9, Value is 0.9
    
    开始查找...
    	元素-2不存在!
    	元素3存在, 它的值是0.3!
    
    修改元素...
    	Key is 3, Value is 1.3
    	Key is -2, Value is 0.0


### 3.8 使用Set
Set由一群key组合而成，如果我们想知道某值是否存在于某个集合内，就可以使用set。和C++的Map类似，Set也分为有序的Set和无序的unordered_set。两者的区别如下：
 
|Differences|set|unordered_set|
|------|------|------| 
|Ordering|increasing  order(by default)|no ordering|
|Implementation|Self balancing BST|Hash Table|
|Search time|log(n)|O(1) -> Average, O(n) -> Worst Case|
|Insertion time|log(n) + Rebalance| Same as search|
|Deletion time|log(n) + Rebalance| Same as search|

对于Python用户来说，C++的unordered_set可以实现类似Python set的功能。使用`from libcpp.unordered_set cimport unordered set`来引入unordered_set类，其操作方法如下：
1. 初始化 - 通过Python的可迭代对象进行初始化，需要声明变量的嵌套类型
2. 遍历 - 让泛型指针自增，通过while循环进行遍历
3. 访问 - 使用deref(C++中的'*'操作符)来解引用
4. 查找 - 使用unordered_set.count，返回1或0
5. 追加 - 使用unordered_set.insert，如果元素已经存在，则元素不会被追加
6. 交集、并集、差集 - 据我所知，unordered_set的这些操作需要开发者自己去实现，不如Python的Set用起来方便。

上述操作的代码演示如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.unordered_set cimport unordered_set


# 通过Python对象初始化
cdef unordered_set[int] myset = {i for i in range(5)}

# 遍历
cdef:
    unordered_set[int].iterator it = myset.begin()
    unordered_set[int].iterator end = myset.end()
print("开始遍历...")
while it != end:
    # 访问
    print("\tValue is %d" % deref(it))
    inc(it)
print()

# 查找
print("开始查找...")
if myset.count(-2):
    print("\t元素-2存在!")
else:
    print("\t元素-2不存在!")

print()

# 追加
print("追加元素...")
myset.insert(0)
myset.insert(-1)

print("\tMyset is: ", myset)
```

    开始遍历...
    	Value is 0
    	Value is 1
    	Value is 2
    	Value is 3
    	Value is 4
    
    开始查找...
    	元素-2不存在!
    
    追加元素...
    	Myset is:  {0, 1, 2, 3, 4, -1}


### 3.9 如何使用Iterator Inserter
回到先前3.6节对filter()的实现，我们output的容器和被filter的容器一样大，而我们只需要filter出部分或全部元素，这显然是浪费空间的。为什么不创建一个空的容器再往里插入元素呢？标准库提供了三种insertion adapter供我们选择，这些容器会把对iterator的赋值操作转换为其他操作：
1. back_inserter()会调用容器的puch_back()函数
2. inserter()会调用容器的insert()函数
3. front_inserter()会调用容器的push_front()函数

想要使用上述三种adapter，需要包含iterator头文件`#include <iterator>`, 对应的Cython语句是`from libcpp cimport iterator`，我们通过`back_inserter(output)`语句生成一个指针，供cfilter插入元素使用，代码示例如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.iterator cimport back_inserter
from chapter_3 cimport filter as cfilter, greater
from libcpp.vector cimport vector

cdef:
    vector[int] ivec = [1, 2, 3, 4, 5]
    int target = 3
    vector[int] output


cfilter(ivec.begin(), ivec.end(), back_inserter(output), target, greater[int]())
print(output)
```

    [4, 5]



```python

```
