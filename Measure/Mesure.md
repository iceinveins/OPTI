# <font color="3d8c95">测量</font>
静态插桩
```
#include <chrono>
// ...
auto A = std::chrono::high_resolution_clock::now();
auto B = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(B-A); 
this->getLog()->info("cost {}", duration.count());
```
空跑50ns

```
#include <chrono>
// ...
auto now = std::chrono::high_resolution_clock::now();
auto ns_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch); 
// <100ns
this->getLog()->info("cost {}", ns_since_epoch.count());
// <1000ns
```

测量时为了阻止代码被重排或删除，但允许内部优化逻辑，可使用内存屏障
```
void critical_code() {
    // 关键代码
    asm volatile ("" : : : "memory");
}
```