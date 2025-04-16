# <font color="3d8c95">代码大页</font>
代码大页是基于透明大页THP（Transparent Huge Pages）进行的优化扩展，支持将应用程序和动态链接库的可执行部分放入到大页（通常是2 MB或更大）中，有助于降低程序的iTLB miss，并提升CPU的2 MB iTLB利用率，避免内存碎片化或内存膨胀问题，提高内存利用效率，适用于数据库、大型应用程序等大代码段业务场景。本文主要介绍代码大页的使用方法以及性能收益等。

[阿里云 代码大页](https://www.alibabacloud.com/help/zh/alinux/user-guide/hugetext)