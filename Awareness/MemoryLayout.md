# <font color="3d8c95">多线程内存布局</font>

关键在于区分与进程的内存布局。
1、由一个栈切分出多个栈分配给不同的线程
2、共用一份代码，但是各个线程的pc（指令寄存器）指向的代码不同

![alt text](assets/images/memoryLayout.png)