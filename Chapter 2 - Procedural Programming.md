## 2. 面向过程的编程风格
作者: tushushu  
项目地址: https://github.com/tushushu/EssentialCython   
参考书籍: 《Essential C++ 中文版》  
编程环境: MacOS + Jupyter Notebook + Python 3.6


```python
%load_ext Cython
```

### 2.1 如何编写函数
每一个函数必须定义以下四个部分：
1. 返回类型
2. 函数名
3. 参数列表
4. 函数体
  
例：计算斐波那契数列某个位置pos的元素值。  
解：我们编写一个名为fibon_elem的函数，求出对应位置pos的元素值，并直接修改变量elem。其中pos介于0与1024之间，如果传入的pos值合法则返回True，否则返回False。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp cimport bool


cdef bool fibon_elem(int pos, int &elem):
    # Cython不允许直接对引用进行赋值，需要用指针进行中介
    cdef int *_elem = &elem
    # 检查位置值是否合理
    if pos <= 0 or pos > 1024:
        return False
    # 位置值为1和2时，elem的值为1
    _elem[0] = 1
    cdef:
        int n_2 = 1
        int n_1 = 1
        int ix
    for ix in range(3, pos + 1):
        _elem[0] = n_2 + n_1
        n_2 = n_1
        n_1 = _elem[0]
    return True


# 测试
cdef:
    int pos
    int elem = 0
for pos in [1, 2, 3, 4, 5, 6, 1025]:
    sucess = fibon_elem(pos, elem)
    if sucess:
        msg = f"计算成功, 第{pos}个位置的元素值为{elem}!"
    else:
        msg = f"计算失败, <pos>参数为{pos}, 超过了合法范围!"
    print(msg)
```

    计算成功, 第1个位置的元素值为1!
    计算成功, 第2个位置的元素值为1!
    计算成功, 第3个位置的元素值为2!
    计算成功, 第4个位置的元素值为3!
    计算成功, 第5个位置的元素值为5!
    计算成功, 第6个位置的元素值为8!
    计算失败, <pos>参数为1025, 超过了合法范围!


注意：上面的代码中，_elem[0]表示对指针进行解引用，而不能像C/C++一样直接用星号来操作。如下代码给出更多的示例：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from cython.operator cimport dereference as deref

cdef int i = 3
cdef int *p = &i

# 使用dereference操作符
print(f"1. 指针p指向的变量的值为{deref(p)}!")

# 使用下标索引的方式
print(f"2. 指针p指向的变量的值为{p[0]}!")

# 给指针指向的变量赋值
# deref(p) = 4 # 代码会报错, "Cannot assign to or delete this"
p[0] = 4
print(f"3. 指针p指向的变量的值为{p[0]}!")
```

    1. 指针p指向的变量的值为3!
    2. 指针p指向的变量的值为3!
    3. 指针p指向的变量的值为4!


### 2.2 调用函数
本节将实现一个可对Vector内的整数值加以排序的函数。通过这个例子，我们可以审视两种参数传递方式：传址及传值。  
    首先编写一个冒泡排序算法，注意定义一个vector要用'vector[int]'而不是'vector\<int\>'，这点跟C++略有区别。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector


cdef void swap(int val1, int val2):
    cdef int temp = val1
    val1 = val2
    val2 = temp

    
cdef void bubble_sort(vector[int] vec):
    cdef:
        int ix = 0
        int jx = 0
    for ix in range(vec.size()):
        for jx in range(ix + 1, vec.size()):
            if vec[ix] > vec[jx]:
                swap(vec[ix], vec[jx])

# 测试
cdef vector[int] vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec)
print(f"排序后的vector: {vec}")
```

    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [8, 34, 3, 13, 1, 21, 5, 2]


上面的运行结果显然不是我们所期待的，首先我们看看swap函数是不是有bug。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++

cdef void swap(int val1, int val2):
    cdef int temp = val1
    val1 = val2
    val2 = temp

cdef:
    int val1 = 1
    int val2 = 2
print(f"交换之前val1={val1}, val2={val2}!")
swap(val1, val2)
print(f"交换之前val1={val1}, val2={val2}!")
```

    交换之前val1=1, val2=2!
    交换之前val1=1, val2=2!


试着交换val1和val2的值失败。我们再打印一下函数体内val1和val2的值。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++

cdef void swap(int val1, int val2):
    print(f"交换之前val1={val1}, val2={val2}!")
    cdef int temp = val1
    val1 = val2
    val2 = temp
    print(f"交换之前val1={val1}, val2={val2}!")

cdef:
    int val1 = 1
    int val2 = 2

swap(val1, val2)
```

    交换之前val1=1, val2=2!
    交换之前val1=2, val2=1!


发现在函数体内val1和val2交换成功了。所以产生bug的原因在于传给swap()的对象被复制了一份，原对象和副本之间没有任何关联。当我们调用一个函数时，会在内存中建立起一块特殊区域，称为“程序栈”。这块特殊区域提供了每个函数的参数的储存空间。它也提供了函数所定义的每个对象的内存空间。我们将这些对象称为local object。一旦函数完成，这块内存就会被释放掉，或者说从程序栈中pop出来。  
为了让程序正确工作，我们必须采用传址的方式而不是传值的方式来定义函数。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector


cdef void swap(int &val1, int &val2):
    # Cython不允许直接对引用进行赋值，需要用指针进行中介
    cdef:
        int *p1 = &val1
        int *p2 = &val2
        int temp = p1[0]
    p1[0] = p2[0]
    p2[0] = temp


cdef void bubble_sort(vector[int] &vec):
    cdef:
        int ix = 0
        int jx = 0
    for ix in range(vec.size()):
        for jx in range(ix + 1, vec.size()):
            if vec[ix] > vec[jx]:
                swap(vec[ix], vec[jx])

# 测试
cdef vector[int] vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec)
print(f"排序后的vector: {vec}")
```

    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [1, 2, 3, 5, 8, 13, 21, 34]


### 2.3 提供默认的参数值
C++允许我们为全部或部分参数设定默认值。在我们的冒泡排序程序中，可以设置升序排列或者降序排列，如果用户不指定asc参数，默认按照升序排列。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp cimport bool
from libcpp.vector cimport vector


cdef void swap(int &val1, int &val2):
    cdef:
        int *p1 = &val1
        int *p2 = &val2
        int temp = p1[0]
    p1[0] = p2[0]
    p2[0] = temp


cdef void bubble_sort(vector[int] &vec, bool asc = True):
    cdef:
        int ix = 0
        int jx = 0
    for ix in range(vec.size()):
        for jx in range(ix + 1, vec.size()):
            if asc:
                if vec[ix] > vec[jx]:
                    swap(vec[ix], vec[jx])
            else:
                if vec[ix] < vec[jx]:
                    swap(vec[ix], vec[jx])

# 测试 1
print("升序排列...")
cdef vector[int] vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec)
print(f"排序后的vector: {vec}")
print()

# 测试 2
print("降序排列...")
vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec, False)
print(f"排序后的vector: {vec}")
```

    升序排列...
    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [1, 2, 3, 5, 8, 13, 21, 34]
    
    降序排列...
    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [34, 21, 13, 8, 5, 3, 2, 1]


### 2.4 使用局部静态对象
2.1节的fibon_elem()函数每次被调用时，便计算出斐波那契数列从1到pos的所有值(其中pos是由用户指定)，然后返回。这里花费了一些不必要的工夫。请看以下对fibon_elem()的三次调用：
1. fibon_elem(24)
2. fibon_elem(8)
3. fibon_elem(18)

其实第一次调用便已经计算出第二次、第三次调用所需要计算的值。我们可以考虑把计算结果缓存起来，如果下一次函数调用的时候结果已经存在于缓存中，则不需要重新计算。  
如果在file scope定义缓存对象会过于冒险，打乱不同函数间的独立性。另外一个解法就是局部静态对象，与局部对象不同，它在每次函数调用的时候不会被重新创建，所以适合当作缓存。  

因为Cython还不支持static变量，所以我们先创建chapter_2_extern.cpp和chapter_2_extern.h文件，用C++编写这个函数。注意fibon_elem要放在_fibon_elem之后定义，否则会报错。
```C++
#include "chapter_2_extern.h"

static void _fibon_elem(int pos, int &elem, vector<int> &cache)
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

static int fibon_elem(int pos, int &elem)
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

```


创建chapter_2.pxd文件(相当于C/C++头文件)，编写Cython代码来加载这个函数。
```cython
from libcpp cimport bool
from libcpp.vector cimport vector


cdef extern from "chapter_2_extern.h":
    void _fibon_elem(int pos, int &elem, vector[int] &cache)
    int fibon_elem(int pos, int &elem)

```
然后通过执行`python setup.py build_ext --inplace`命令，将代码编译为.so文件。  
编写fibon_elem_py函数，测试代码和结果如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_2 cimport fibon_elem

def fibon_elem_py(pos):
    cdef int _elem = 0
    sucess = fibon_elem(pos, _elem)
    return _elem, sucess

for pos in [1, 2, 3, 4, 8, 5, 6, 1025]:
    elem, sucess = fibon_elem_py(pos)
    if sucess == 1:
        msg = f"计算成功, 第{pos}个位置的元素值为{elem}!"
    elif sucess == 2:
        msg = f"调用缓存, 第{pos}个位置的元素值为{elem}!"
    else:
        msg = f"计算失败, <pos>参数为{pos}, 超过了合法范围!"
    print(msg)
```

    计算成功, 第1个位置的元素值为1!
    调用缓存, 第2个位置的元素值为1!
    计算成功, 第3个位置的元素值为2!
    计算成功, 第4个位置的元素值为3!
    计算成功, 第8个位置的元素值为21!
    调用缓存, 第5个位置的元素值为5!
    调用缓存, 第6个位置的元素值为8!
    计算失败, <pos>参数为1025, 超过了合法范围!


### 2.5 声明inline函数

使用函数能够避免将相同代码重写多次的麻烦，还能减少可执行程序的体积，但也会带来程序运行时间上的开销。

函数调用在执行时，首先要在栈中为形参和局部变量分配存储空间，然后还要将实参的值复制给形参，接下来还要将函数的返回地址（该地址指明了函数执行结束后，程序应该回到哪里继续执行）放入栈中，最后才跳转到函数内部执行。这个过程是要耗费时间的。

另外，函数执行 return 语句返回时，需要从栈中回收形参和局部变量占用的存储空间，然后从栈中取出返回地址，再跳转到该地址继续执行，这个过程也要耗费时间。

总之，使用函数调用语句和直接把函数中的代码重新抄写一遍相比，节省了人力，但是带来了程序运行时间上的额外开销。

一般情况下，这个开销可以忽略不计。但是，如果一个函数内部没有几条语句，执行时间本来就非常短，那么这个函数调用产生的额外开销和函数本身执行的时间相比，就显得不能忽略了。假如这样的函数在一个循环中被上千万次地执行，函数调用导致的时间开销可能就会使得程序运行明显变慢。

作为特别注重程序执行效率，适合编写底层系统软件的高级程序设计语言，C++ 用 inline 关键字较好地解决了函数调用开销的问题。

在 C++ 中，可以在定义函数时，在返回值类型前面加上 inline 关键字。如：
```C++
inline int Max (int a, int b)
{
    if(a > b)
        return a;
    return b;
}
```
增加了 inline 关键字的函数称为“内联函数”。内联函数和普通函数的区别在于：当编译器处理调用内联函数的语句时，不会将该语句编译成函数调用的指令，而是直接将整个函数体的代码插人调用语句处，就像整个函数体在调用处被重写了一遍一样。

有了内联函数，就能像调用一个函数那样方便地重复使用一段代码，而不需要付出执行函数调用的额外开销。很显然，使用内联函数会使最终可执行程序的体积增加。以时间换取空间，或增加空间消耗来节省时间，这是计算机学科中常用的方法。

内联函数中的代码应该只是很简单、执行很快的几条语句。如果一个函数较为复杂，它执行的时间可能上万倍于函数调用的额外开销，那么将其作为内联函数处理的结果是付出让代码体积增加不少的代价，却只使速度提高了万分之一，这显然是不划算的。

有时函数看上去很简单，例如只有一个包含一两条语句的循环，但该循环的执行次数可能很多，要消耗大量时间，那么这种情况也不适合将其实现为内联函数。

另外，需要注意的是，调用内联函数的语句前必须已经出现内联函数的定义（即整个函数体），而不能只出现内联函数的声明。

参考链接: [C++内联函数（C++ inline）详解](http://c.biancheng.net/view/199.html)  

接下来我们测试内联函数对性能的提升，还是以斐波那契数列为例。经测试，使用内联函数后性能有所提升。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp cimport bool
from libcpp.vector cimport vector
from time import perf_counter


cdef long long fib_elem(long long n):
    if n < 2:
        return n
    return fib_elem(n - 1) + fib_elem(n - 2)


cdef inline long long inline_fib(long long n):
    if n < 2:
        return n;
    return inline_fib(n - 1) + inline_fib(n - 2)


cdef vector[long long] fib_seq(long long n):
    cdef long long i = 0
    cdef vector[long long] v
    while i < n:
        v.push_back(fib_elem(i))
        i += 1
    return v


cdef vector[long long] fast_fib_seq(long long n):
    cdef long long i = 0
    cdef vector[long long] v
    while i < n:
        v.push_back(inline_fib(i))
        i += 1
    return v


# 测试
cdef long long n = 40
# 不使用内联函数
start = perf_counter()
x = fib_seq(n)
run_time = round(perf_counter() - start, 5)
print(f"不使用内联函数运行时间为{run_time}秒!")
print("运算结果是:", x)
print()
# 使用内联函数
start = perf_counter()
y = fast_fib_seq(n)
inline_run_time = round(perf_counter() - start, 5)
print(f"内联函数运行时间为{inline_run_time}秒!")
print("运算结果是:", y)
```

    不使用内联函数运行时间为0.7078秒!
    运算结果是: [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393, 196418, 317811, 514229, 832040, 1346269, 2178309, 3524578, 5702887, 9227465, 14930352, 24157817, 39088169, 63245986]
    
    内联函数运行时间为0.68129秒!
    运算结果是: [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393, 196418, 317811, 514229, 832040, 1346269, 2178309, 3524578, 5702887, 9227465, 14930352, 24157817, 39088169, 63245986]


### 2.6 提供重载函数
现在我们来提供一个通用的vector_sum函数，我们是否可以传入不同类型甚至不同数量的参数给vector_sum呢？可以。如何办到？这必须通过所谓的函数重载机制。
```C++
int vector_sum(vector<int> &ivec);
float vector_sum(vector<float> &fvec);
```
既然名称相同，编译器如何知道应该调用哪一个函数呢？它会将调用提供的参数拿来和每个重载函数的参数进行比较，找出其中最合适的。  
编译器无法根据函数返回类型来区分两个具有相同名称的函数。以下便是不正确的写法，会产生编译错误：
```C++
ostream& display_message(char ch);
bool display_message(char ch);
```
我们用Cython来实现这个函数如下：

由于Cython不支持直接编写重载函数，我们先建立chapter_2_extern.cpp和chapter_2_extern.h文件，编写如下代码：
```C++
#include "chapter_2_extern.h"

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
```

创建chapter_2.pxd文件(相当于C/C++头文件)，导入我们写好的C++代码 

```Cython
cdef extern from "chapter_2_extern.h":
    int vector_sum(vector[int] &v)
    float vector_sum(vector[float] &v)
```


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from chapter_2 cimport vector_sum


cdef vector[int] ivec = [1, 2, 3, 4]
print("对元素为int类型的vector进行求和:", vector_sum(ivec))

cdef vector[float] fvec = [1.0, 2.0, 3.0]
print("对元素为float类型的vector进行求和:", vector_sum(fvec))
```

    对元素为int类型的vector进行求和: 10
    对元素为float类型的vector进行求和: 6.0


### 2.7 定义并使用模板函数
2.6的例子中，每个函数体都颇为相像。唯一的差别就是参数的类型，一个是`vector<int>`型，一个是`vector<float>`型。需要一种机制，让我们得以将单一函数的内容与各种类型的vector绑定起来。所谓function template(函数模板)便提供了这样的机制。 
function template以关键字template开场，其后紧接着以成对尖括号(<>)包围起来一个或多个标识符。这些标识符用以表示我们希望稍后决定的数据类型。用户每次利用这一模版产生函数，都必须提供确实的类型信息。这些标识符事实上扮演着占位符的角色，用来放置函数参数列表及函数体中的某些实际数据类型。例如：  
```C++
template <typename elemType>
void display_message(const string &msg, const vector<elemType> &vec){
    cout << msg;
    for (int ix = 0; ix < vec.size(); ++ix){
        elemType t = vec[ix];
        cout << t << ' ';
    }
}
```
一般而言，如果函数具备多种实现方式，我们可将它重载，其每份实例提供的是相同的通用服务。如果我们希望让程序代码主体不变，仅仅改变其中用到的数据类型，可以通过function template达到目的。  
Cython提供了融合类型(fused type)可以使用一个类型定义来引用多个类型。这使我们可以编写一个可以对多种类型的值进行操作的静态类型的cython函数。因此，融合类型允许泛型编程，并且类似于C++的template。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++

ctypedef fused int_or_float:
    int
    float

cdef int_or_float my_sum(int_or_float x, int_or_float y):
    return x + y

cdef int x = 1
cdef int y = 2
print(f"x + y 等于 {my_sum(x, y)}!")

cdef float a = 3.0
cdef float b = 4.0
print(f"a + b 等于 {my_sum(a, b)}!")
```

    x + y 等于 3!
    a + b 等于 7.0!


虽然上面的例子可以运行，但是Cython还不支持`vector[int_or_float] &v`这种写法(不支持&符号)，所以还是需要extern C++来实现vector_sum的模板函数。
```C++
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
```

然后用Cython脚本导入Cpp函数。
```Cython
cdef extern from "chapter_2_extern.h":
    T template_vector_sum[T](vector[T] &v)
```
注意template_vector_sum后面也要写上`[T]`，否则Cython无法识别出T(被这里坑了好久T-T)。写测试脚本如下：


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp.vector cimport vector
from chapter_2 cimport template_vector_sum

cdef vector[int] ivec = [1, 2, 3]
print("对vector[int]求和结果为", template_vector_sum(ivec))

cdef vector[float] fvec = [4.0, 5.0, 6.0]
print("对vector[float]求和结果为", template_vector_sum(fvec))

cdef vector[double] dvec = [7.0, 8.0, 9.0]
print("对vector[double]求和结果为", template_vector_sum(dvec))
```

    对vector[int]求和结果为 6
    对vector[float]求和结果为 15.0
    对vector[double]求和结果为 24.0


### 2.8 函数指针带来更大的灵活性
以2.3的冒泡排序为例，在循环体内部，要不停地执行`if asc`来判断该执行`vec[ix] > vec[jx]`还是`vec[ix] < vec[jx]`。这显然有些冗余，因为asc参数传进来的时候就不会再更改了。我们可以使用指针来达到这一目的，具体说是函数指针。  
所谓函数指针，其形式相当复杂。它必须指明其所指函数的返回类型和参数列表类型。我们这个例子中可以定义一个cmp(compare)函数指针，用于指向比较两个值大小的函数，分别定义greater_than和less_than两个函数来实现升序排列和降序排列，代码如下：  
注意：函数指针不能设置为函数的默认参数。


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from libcpp cimport bool
from libcpp.vector cimport vector


cdef inline void swap(int &val1, int &val2):
    cdef:
        int *p1 = &val1
        int *p2 = &val2
        int temp = p1[0]
    p1[0] = p2[0]
    p2[0] = temp
    

cdef inline bool greater_than(int val1, int val2):
    return val1 > val2


cdef inline bool less_than(int val1, int val2):
    return val1 < val2


cdef void bubble_sort(vector[int] &vec, bool (*cmp)(int, int)):
    cdef:
        int ix = 0
        int jx = 0
    for ix in range(vec.size()):
        for jx in range(ix + 1, vec.size()):
            if cmp(vec[ix], vec[jx]):
                swap(vec[ix], vec[jx])

# 测试 1
print("升序排列...")
cdef vector[int] vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec, greater_than)
print(f"排序后的vector: {vec}")
print()

# 测试 2
print("降序排列...")
vec = [8, 34, 3, 13, 1, 21, 5, 2]
print(f"排序前的vector: {vec}")
bubble_sort(vec, less_than)
print(f"排序后的vector: {vec}")
```

    升序排列...
    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [1, 2, 3, 5, 8, 13, 21, 34]
    
    降序排列...
    排序前的vector: [8, 34, 3, 13, 1, 21, 5, 2]
    排序后的vector: [34, 21, 13, 8, 5, 3, 2, 1]


### 2.9 设定头文件
Cython的.pxd文件类似C++的.h头文件，.pyx文件类似C++的.cpp文件。如果模块需要被Cython import就需要.pxd文件，如果需要被Python import则同时需要.pxd文件，.pyx文件以及被编译的.so文件。

1. 函数的定义只能有一份，但可以有多份声明
2. inline函数的定义和声明都要放在pxd文件中
3. 普通函数只可以把声明放在pxd文件中

```Cython
cdef inline int fmax(int a, int b):
    return a if a > b else b
```


```cython
%%cython --cplus --compile-args=-stdlib=libc++ --link-args=-stdlib=libc++
from chapter_2 cimport fmax

print(fmax(3, 2))
```

    3



```python

```
