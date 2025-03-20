
# <font  color='3d8c95'>LockFree无锁编程</font>
## <font  color='dc843f'>CAS指令与ABA问题的基础解决方案</font>

无锁编程利用CAS（Compare - And - Swap）指令避免锁竞争，但其经典问题——ABA问题——需特别注意。
```
/* Naive lock-free stack which suffers from ABA problem.*/
class Stack {
  std::atomic<Obj*> top_ptr;
  //
  // Pops the top object and returns a pointer to it.
  //
  Obj* Pop() {
    while (1) {
      Obj* ret_ptr = top_ptr;
      if (ret_ptr == nullptr) return nullptr;
      // For simplicity, suppose that we can ensure that this dereference is safe
      // (i.e., that no other thread has popped the stack in the meantime).
      Obj* next_ptr = ret_ptr->next;
      // If the top node is still ret, then assume no one has changed the stack.
      // (That statement is not always true because of the ABA problem)
      // Atomically replace top with next.
      if (top_ptr.compare_exchange_weak(ret_ptr, next_ptr)) {
        return ret_ptr;
      }
      // The stack has changed, start over.
    }
  }
  //
  // Pushes the object specified by obj_ptr to stack.
  //
  void Push(Obj* obj_ptr) {
    while (1) {
      Obj* next_ptr = top_ptr;
      obj_ptr->next = next_ptr;
      // If the top node is still next, then assume no one has changed the stack.
      // (That statement is not always true because of the ABA problem)
      // Atomically replace top with obj.
      if (top_ptr.compare_exchange_weak(next_ptr, obj_ptr)) {
        return;
      }
      // The stack has changed, start over.
    }
  }
};
```
This code can normally prevent problems from concurrent access, but suffers from ABA problems. Consider the following sequence:

Stack initially contains $top → A → B → C$

Thread 1 starts running pop:
```
ret_ptr = A;
next_ptr = B;
```
Thread 1 gets interrupted just before the `compare_exchange_weak`...
```
{ // Thread 2 runs pop:
  ret_ptr = A;
  next_ptr = B;
  top_ptr.compare_exchange_weak(A, B)  // Success, top = B
  return A;
} // Now the stack is top → B → C
{ // Thread 2 runs pop again:
  ret_ptr = B;
  next_ptr = C;
  top_ptr.compare_exchange_weak(B, C)  // Success, top = C
  return B;
} // Now the stack is top → C
delete B;
{ // Thread 2 now pushes A back onto the stack:
  A->next_ptr = C;
  top_ptr.compare_exchange_weak(C, A)  // Success, top = A
}
```
Now the stack is $top → A → C$

When Thread 1 resumes:
`compare_exchange_weak(A, B)`

This instruction succeeds because it finds top == ret_ptr (both are A), so it sets top to next_ptr (which is B). As B has been deleted the program will access freed memory when it tries to look at the first element on the stack. In C++, as shown here, accessing freed memory is undefined behavior: this may result in crashes, data corruption or even just silently appear to work correctly. ABA bugs such as this can be difficult to debug.

<font  color='235977'> compare_exchange_weak/compare_exchange_strong是著名的CAS(compare and set)  
参数传入期待值与新值，通过比较当前值与期待值的情况进行区别改变。  
a.compare_exchange_weak(b,c)其中a是当前值，b期望值，c新值  
a==b时：函数返回真，并把c赋值给a  
a!=b时：函数返回假，并把a复制给b  

>*函数签名*  
bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) noexcept;  
expected: A reference to the value expected to be found in the atomic object.  
desired: The value to store in the atomic object if it is as expected.  
success: The memory synchronization ordering for the read-modify-write operation if the comparison succeeds.  
failure: The memory synchronization ordering for the load operation if the comparison fails.
</font>

**问题分析：**

线程1在准备进行CAS操作时，线程2有可能弹出n1并且还会重新插入它，进而致使CAS出现误判
(你大爷还是你大爷，你大妈已经不是你大妈了)

**解决方案：**

*带标记的状态引用*  
一种常见的解决方法是在相关量上添加额外的 “标记” 或 “时间戳” 位。例如，一种在指针上使用比较并交换（CAS）操作的算法，可能会利用地址的低位来表示该指针已成功修改的次数。因此，即使地址相同，下一次比较并交换操作也会失败，因为标记位不匹配。这有时被称为 ABA’问题，因为第二个 A 与第一个 A 略有不同。这种带标记的状态引用也用于事务内存中。尽管可以使用带标记的指针来实现，但如果有双字宽度的 CAS 操作可用，那么使用单独的标记字段更为合适。
如果 “标记” 字段循环溢出，那么针对 ABA 问题的保证就不再成立。不过，据观察，在当前现有的 CPU 上，使用 60 位的标记时，只要程序运行周期（即不重启程序）限制在 10 年内，就不会出现循环溢出的情况；此外，有人认为，从实际应用角度来看，通常使用 40 - 48 位的标记就足以保证不会循环溢出。由于现代 CPU（特别是所有现代 x64 CPU）往往支持 128 位的 CAS 操作，这可以有力地保证避免 ABA 问题

*中间节点*  
一种正确但开销较大的方法是使用非数据元素的中间节点，从而在插入和删除元素时确保不变性。

*延迟回收*  
另一种方法是延迟对已删除数据元素的回收。延迟回收的一种方式是在具备自动垃圾回收器的环境中运行算法；但这里存在一个问题，即如果垃圾回收器不是无锁的，那么即使数据结构本身是无锁的，整个系统也不是无锁的。
延迟回收的另一种方式是使用一个或多个危险指针，这些指针指向在列表中原本不会出现的位置。每个危险指针代表一个正在进行的变更的中间状态；该指针的存在可确保进一步的同步。危险指针是无锁的，但每个线程最多只能跟踪固定数量的正在使用的元素。
延迟回收的还有一种方式是使用读 - 复制更新（RCU），这涉及将更新操作包含在 RCU 读端临界区中，然后在释放任何已删除的数据元素之前等待 RCU 宽限期。以这种方式使用 RCU 可确保在所有当前正在执行的操作完成之前，任何已删除的数据元素都不会重新出现。RCU 是无锁的，但并不适用于所有工作负载。

## <font  color='dc843f'>无锁队列</font>
优势：无锁设计避免了锁竞争，但是需注意内存管理方面的问题（可以结合epoch - based reclamation来解决内存回收 或者 对自身数据了解得情况下使用环形无锁队列。）

[atomic_queue](https://github.com/max0x7ba/atomic_queue/blob/master/include/atomic_queue/atomic_queue.h)  
[moodycamel::ReaderWriterQueue](https://github.com/cameron314/readerwriterqueue/blob/master/readerwriterqueue.h)  
[moodycamel::BlockingReaderWriterCircularBuffer](https://github.com/cameron314/readerwriterqueue/blob/master/readerwritercircularbuffer.h)  
