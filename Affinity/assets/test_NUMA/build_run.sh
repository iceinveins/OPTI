PROJECT_ID=test_NUMA
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 -DNDEBUG # -mavx2
if [ $? -eq 0 ]; then
    numactl --cpubind=0 --membind=0 ./${PROJECT_ID}.elf 20000
    numactl --cpubind=0 --membind=1 ./${PROJECT_ID}.elf 20000 
fi