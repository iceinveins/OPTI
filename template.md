# <font  color='3d8c95'>H1</font>

## <font  color='dc843f'>H2</font>

<font  color='fed3a8'>*h*</font>

<font  color='235977'>RESERVE</font>

[atomicQueue](https://github.com/max0x7ba/atomic_queue?tab=readme-ov-file)

![alt text](XXX.png)

PROJECT_ID=XXX
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 -DNDEBUG # -mavx2
if [ $? -eq 0 ]; then
    ./${PROJECT_ID}.elf
fi