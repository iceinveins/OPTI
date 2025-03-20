g++ -o lockFreeQueue.elf lockFreeQueue.cpp -std=c++2a -march=native -O3 -latomic -lpthread
if [ $? -eq 0 ]; then
    ./lockFreeQueue.elf
fi