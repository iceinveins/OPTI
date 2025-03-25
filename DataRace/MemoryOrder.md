# <font  color='3d8c95'>MemoryOrder内存序</font>
用于控制内存访问顺序，也可以通过内存屏障[MemoryBarrier](extension/MemoryBarrier.md)实现，但内存屏障指令属于硬件底层指令，与平台相关。而c++ memory_order是语言级抽象，可移植性更高！

## <font  color='dc843f'>4种关系术语</font>
***sequenced-before***
>同一个线程之内，语句A的执行顺序在语句B前面，那么就成为A sequenced-before B。 它不仅仅表示两个操作之间的先后顺序，还表示了操作结果之间的可见性关系。 两个操作A和操作B，如果有A sequenced-before B，除了表示操作A的顺序在B之前，还表示了操作A的结果操作B可见。 例如：语句A是sequenced-before语句B的。
```
r2 = x.load(std::memory_order_relaxed); // A 
y.store(42, std::memory_order_relaxed); // B
```

***happens-before***
>happens-before关系表示的不同线程之间的操作先后顺序。 如果A happens-before B，则A的内存状态将在B操作执行之前就可见。 happends-before关系满足传递性、非自反性和非对称性。 happens before包含了inter-thread happens before和synchronizes-with两种关系。

***synchronizes-with***
>synchronizes-with关系强调的是变量被修改之后的传播关系（propagate）， 即如果一个线程修改某变量的之后的结果能被其它线程可见，那么就是满足synchronizes-with关系的[9]。 另外synchronizes-with可以被认为是跨线程间的happends-before关系。 显然，满足synchronizes-with关系的操作一定满足happens-before关系了。

***Carries dependency***
>同一个线程内，表达式A sequenced-before 表达式B，并且表达式B的值是受表达式A的影响的一种关系， 称之为Carries dependency。这个很好理解，例如：
```
int *a = &var1;     // A
int *b = &var2;     // B
c = *a + *b;        // C
```
执行语句A和B与语句C之间就存在Carries dependency关系， 因为c的值直接依赖于*a和*b的值。

## <font  color='dc843f'>C++六种内存顺序</font>
***指令重排问题***
有时候，我们会用一个变量作为标志位，当这个变量等于某个特定值的时候就进行某些操作。但是这样依然可能会有一些意想不到的坑，例如两个线程以如下顺序执行：
| step | thread A   | thread B       |
| ---- | ---------- | -------------- |
| 1    | a = 1      |                |
| 2    | flag= true |                |
| 3    |            | if flag== true |
| 4    |            | assert(a == 1) |

当B判断flag为true后，断言a为1，看起来的确是这样。那么一定是这样吗？可能不是，因为编译器和CPU都可能将指令进行重排。实际上的执行顺序可能变成这样：
| step | thread A    | thread B       |
| ---- | ----------- | -------------- |
| 1    | flag = true |                |
| 2    |             | if flag== true |
| 3    |             | assert(a == 1) |
| 4    | a = 1       |                |

这就导致了step3的时候断言失败。  
## <font  color='dc843f'>为什么编译器和CPU在执行时会对指令进行重排呢？</font>
因为现代CPU采用多发射技术（同时有多条指令并行）、流水线技术，为了避免流水线断流，CPU会进行适当的指令重排。这在计算机组成原理的流水线一节有所涉及，如果是单线程任务，那么一切正常，CPU和编译器对代码顺序调换是符合程序逻辑的，不会出错。但一到多线程编程中，结果就可能如上例所示。

C++11引入了非常重要的内存顺序概念，用以解决上述指令重排问题。  
实际上，之前介绍的atomic类型的成员函数有一个额外参数，以store为例：
```
void store( T desired, std::memory_order order = std::memory_order_seq_cst )
```
这个参数代表了该操作使用的内存顺序，用于控制变量在不同线程见的顺序可见性问题，不只store，其他成员函数也带有该参数。
c++11提供了六种内存顺序供选择：
```
typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;
```
之前在场景2中，因为指令的重排导致了意料之外的错误，通过使用原子变量并选择合适内存序，可以解决这个问题。下面先来看看这几种内存序

## <font  color='dc843f'>memory_order_release/memory_order_acquire</font>
- memory_order_release用于store成员函数。简单地说，就是<font color="fed3a8">写不后</font>。即，写语句不能调到本条语句之后。
- memory_order_acquire用于load成员函数就是<font color="fed3a8">读不前</font>

<font  color='fed3a8'>*release和acquire构成了synchronize-with关系*</font>，也就是同步关系。 在这个关系下：线程A中所有发生在release x之前的值的写操作， 对线程B的acquire x之后的任何操作都可见。

举个例子：  
假设flag为一个 atomic特化的bool 原子量，a为一个int变量，并且有如下时序的操作：

| step | thread A                               | thread B                                      |
| ---- | -------------------------------------- | --------------------------------------------- |
| 1    | a = 1                                  |                                               |
| 2    | flag.store(true, memory_order_release) |                                               |
| 3    |                                        | if( true == flag.load(memory_order_acquire) ) |
| 4    |                                        | assert(a == 1)                                |

***在这种情况下，step1不会跑到step2后面去，step4不会跑到step3前面去。***

这样一来，保证了当读取到flag为true的时候a一定已经被写入为1了。  
换一种比较严谨的描述方式可以总结为：对于同一个原子量，release操作前的写入，一定对随后acquire操作后的读取可见。 这两种内存序是需要配对使用的，这也是将他们放在一起介绍的原因。
还有一点需要注意的是：只有对同一个原子量进行操作才会有上面的保证，比如step3如果是读取了另一个原子量flag2，是不能保证读取到a的值为1的。

## <font  color='dc843f'>memory_order_release/memory_order_consume</font>
memory_order_release还可以和memory_order_consume搭配使用。
- memory_order_release的作用跟上面介绍的一样。
- memory_order_consume用于load操作。
这个组合比上一种更宽松，comsume只阻止对这个原子量有依赖的操作重排到前面去，而非像aquire一样全部阻止。

<font  color='fed3a8'>*comsume操作防止在其后对原子变量有依赖的操作(Carries dependency)被重排到前面去，对无依赖的操作不做限定。*</font>
这种情况下：对于同一个原子变量，release操作所依赖的写入，一定对随后consume操作后依赖于该原子变量的操作可见。

将上面的例子稍加改造来展示这种内存序，假设flag为一个 atomic特化的bool 原子量，a为一个int变量，b、c各为一个bool变量，并且有如下时序的操作：

| step | thread A                           | thread B                                       |
| ---- | ---------------------------------- | ---------------------------------------------- |
| 1    | b = true                           |                                                |
| 2    | a = 1                              |                                                |
| 3    | flag.store(b,memory_order_release) |                                                |
| 4    |                                    | while (!(c = flag.load(memory_order_consume))) |
| 5    |                                    | assert(a == 1)  // may fail                    |
| 6    |                                    | assert(c == true)  // always success           |
| 7    |                                    | assert(b == true)  // always success           |

step4使得c依赖于flag，所以step6断言成功。
由于flag依赖于b，b在之前的写入是可见的，此时b一定为true，所以step7的断言一定会成功。而且<font  color='fed3a8'>*这种依赖关系具有传递性*</font>，假如b又依赖与另一个变量d，则d在之前的写入同样对step4之后的操作可见。
那么a呢？很遗憾在这种内存序下a并不能得到保证，step5的断言可能会失败。
## <font  color='dc843f'>memory_order_acq_rel</font>
这个选项看名字就很像release和acquire的结合体，实际上它的确兼具两者的特性。

这个操作用于“读取-修改-写回”这一类既有读取又有修改的操作，例如CAS操作。可以将这个操作在内存序中的作用想象为将release操作和acquire操作捆在一起，<font  color='fed3a8'>*因此任何读写操作的重排都不能跨越这个调用。*</font>

依然以一个例子来说明，flag为一个 atomic特化的bool 原子量，a、c各为一个int变量，b为一个bool变量：

| step | thread A                                     | thread B                                                                      |
| ---- | -------------------------------------------- | ----------------------------------------------------------------------------- |
| 1    | a = 1                                        |                                                                               |
| 2    | flag.store(true,memory_order_release)        |                                                                               |
| 3    |                                              | b = true                                                                      |
| 4    |                                              | c = 2                                                                         |
| 5    |                                              | while (!flag.compare_exchange_weak(b, false, memory_order_acq_rel)){b = true} |
| 6    |                                              | assert(a == 1)  // always success                                             |
| 7    | if (true == flag.load(memory_order_acquire)) |                                                                               |
| 8    | assert(c == 2)  // always success            |                                                                               |

由于memory_order_acq_rel同时具有memory_order_release与memory_order_acquire的作用，因此step2可以和step5组合成上面提到的release/acquire组合，因此step6的断言一定会成功，而step5又可以和step7组成release/acquire组合，step8的断言同样一定会成功。

## <font  color='dc843f'>memory_order_seq_cst</font>
该内存顺序是各个成员函数的内存顺序的<font  color='fed3a8'>*默认选项*</font>。

seq_cst表示顺序一致性内存模型，在这个模型约束下不仅同一个线程内的执行结果是和程序顺序一致的， 每个线程间互相看到的执行结果和程序顺序也保持顺序一致。

## <font  color='dc843f'>memory_order_relaxed</font>
它仅仅只保证其成员函数操作本身是原子不可分割的，但是对于顺序性不做任何保证。

程序计数器是一种典型的应用场景：
```
#include <cassert>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> cnt = {0};
void f()
{
    for (int n = 0; n < 1000; ++n) {
        cnt.fetch_add(1, std::memory_order_relaxed);
    }
}
int main()
{
    std::vector<std::thread> v;
    for (int n = 0; n < 10; ++n) {
        v.emplace_back(f);
    }
    for (auto& t : v) {
        t.join();
    }
    assert(cnt == 10000);    // never failed
    return 0;
}
```
cnt是共享的全局变量，多个线程并发地对cnt执行RMW（Read Modify Write）原子操作。 这里只保证cnt的原子性，其他有依赖cnt的地方不保证任何的同步。

## <font  color='dc843f'>volatile</font>
<font  color='fed3a8'>*volatile关键字能够防止指令被编译器优化，那它能提供线程间(inter-thread)同步语义吗？ 答案是：不能！！！*</font>

尽管volatile能够防止单个线程内对volatile变量进行reorder，但多个线程同时访问同一个volatile变量，线程间是完全不提供同步保证。
而且，volatile不提供原子性！
并发的读写volatile变量是会产生数据竞争的，同时non volatile操作可以在volatile操作附近自由地reorder。

看一个例子，执行下面的并发程序，不出意外的话，你不会得到一个为0的结果。
```
#include <thread>
#include <iostream>

volatile int count = 0;

void increase() {
    for (int i = 0; i < 1000000; i++) {
        count++;
    }
}

void decrease() {
    for (int i = 0; i < 1000000; i++) {
        count--;
    }
}

int main() {
    std::thread t1(increase);
    std::thread t2(decrease);
    t1.join();
    t2.join();
    std::cout << count << std::endl;
```

---
参考文档

https://www.codedump.info/post/20191214-cxx11-memory-model-1/
https://www.codedump.info/post/20191214-cxx11-memory-model-2/
[Understanding compare_exchange_weak() in C++11 - Stack Overflow](https://stackoverflow.com/questions/25199838/understanding-stdatomiccompare-exchange-weak-in-c11)