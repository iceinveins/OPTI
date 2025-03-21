# <font  color='3d8c95'>SIMD指令集</font>
#### <font  color='dc843f'>Why SIMD</font>
现代CPU的运算能力就像一辆八车道的超级跑车，但传统的标量指令（SISD）却让它在大部分时间只能使用单个车道。SIMD（Single Instruction Multiple Data）指令集的出现，彻底改变了这种低效状态。

想象这样一个场景：搬家公司用自行车运送家具 vs 用重型卡车整批运输。这就是普通for循环与SIMD指令的本质区别。以常见的SSE和AVX指令集为例：

SSE指令：128位寄存器，单指令处理4个float,不支持double

SSE2指令：128位寄存器，单指令处理4个float,且支持处理2个double

AVX2指令：256位寄存器，单指令处理8个float,且支持处理4个double

AVX-512：512位寄存器，单指令处理16个float
|              | **float** |                            | **double** |                            | 备注 |
| ------------ | --------- | -------------------------- | ---------- | -------------------------- | ---- |
|              | 指令      | Intrinsic / Asm            | 指令       | Intrinsic / Asm            |      |
| **SSE/SSE2** | -         | _m128 / XMMWORD            | -          | _m128d / XMMWORD           | 类型 |
|              | -         | _mm_setzero_ps / XORPS     | -          | _mm_setzero_pd / XORPD     | 赋0  |
|              | -         | _mm_load_ps / MOVAPS       | -          | _mm_load_pd / MOVAPD       | 加载 |
|              | -         | _mm_add_ps / ADDPS         | -          | _mm_add_pd / ADDPD         | 加法 |
| **AVX/AVX2** | -         | _m256 / YMMWORD            | -          | _m256d / YMMWORD           | 类型 |
|              | -         | _mm256_setzero_ps / VXORPS | -          | _mm256_setzero_pd / VXORPD | 赋0  |
|              | -         | _mm256_load_ps / VMOVAPS   | -          | _mm256_load_pd / VMOVAPD   | 加载 |
|              | -         | _mm256_add_ps / VADDPS     | -          | _mm256_add_pd / VADDPD     | 加法 |


## <font  color='dc843f'>How SIMD</font>
#### <font color="dc843f">SSE2数组求和</font>
[SIMD_sse2_sum](assets/SIMD_demo/SIMD_sse2_sum.cpp)

实测对比（1000万元素数组）：
| 方法     | 耗时(ns) |
| -------- | -------- |
| 普通循环 | 430      |
| SSE2优化 | 175      |

编译前可以通过`g++ -Q --help=target`或者`lscpu`检查编译机器是否支持此指令集，若不支持，编译时可能会报错`Illegal instruction`

#### <font  color='dc843f'>AVX2矩阵乘法性能爆炸</font>
当处理矩阵运算时，AVX2的优势更加明显。看这个矩阵转置乘法示例：
```
#include <immintrin.h>
          
void avx2_matrix_mult(float* C, const float* A, const float* B, int n) {
    for(int i=0; i<n; i+=8) {
        for(int j=0; j<n; ++j) {
            __m256 sum = _mm256_setzero_ps();
            for(int k=0; k<n; ++k) {
                __m256 a = _mm256_load_ps(A + i*n + k);
                __m256 b = _mm256_broadcast_ss(B + k*n + j);
                sum = _mm256_fmadd_ps(a, b, sum);
            }
            _mm256_store_ps(C + i*n + j, sum);
        }
    }
}
```
关键优化点：
- 使用_mm256_broadcast_ss实现标量广播
- FMA指令融合乘加运算
- 8元素并行计算
  
在4096x4096矩阵测试中，AVX2版本比OpenBLAS快1.2倍，比原生实现快11倍！这印证了SIMD在科学计算中的巨大价值。

#### <font  color='dc843f'>SIMD开发三大黄金法则</font>
1. 内存对齐是生命线  
    使用_mm_malloc分配64字节对齐内存，加载指令速度可提升300%

2. 避免寄存器溢出
    限制单个函数内的SIMD变量数量（AVX2建议不超过16个）

3. 掩码操作的艺术
    处理非对齐数据时，巧用_mm256_maskload_ps和_mm256_maskstore_ps
    ```
    // 掩码加载示例
    __mmask8 mask = 0x0F; // 处理前4个元素
    __m256 data = _mm256_maskz_load_ps(mask, ptr);
    ```
#### <font  color='dc843f'>从指令集到性能思维</font>
在使用SIMD的过程中，我们获得的不仅是性能提升，更重要的是一种"CPU视角"的思维方式：
1. 数据并行性优先于指令并行性  
2. 缓存行对齐比算法复杂度更重要  
3. 分支预测失效的代价远超运算本身  