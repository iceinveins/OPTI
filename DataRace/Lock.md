# <font  color='3d8c95'>Lock</font>
|锁|简介|适合场景or对应实现|
|---|---|---|
|悲观锁|对数据被外界修改持悲观态度，持锁阻塞其他|写多读少|
|乐观锁|认为数据一般情况下不会冲突，CAS|读多写少，提高吞吐|
|自旋锁|互斥锁的一种实现，线程循环等待|持锁时间短且不希望睡眠唤醒的开销|
|分段锁|锁细粒度化|concurrentHashMap|
|读写锁|特殊自旋锁|提高并发行性|
|排它锁-共享锁|写锁-读锁|mutex-shared_mutex|
|公平锁|先到先得原则|mutex|
|写优先锁|[WFirstLock](assets/WFirstLock.cpp)|写优先|
|可重入锁|unique_lock通过计数支持可重入|unique_lock|