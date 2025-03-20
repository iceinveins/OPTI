g++ -o aba.elf aba.cpp -std=c++2a -march=native -O3 -latomic -lpthread

if [ $? -eq 0 ]; then
    ./aba.elf
fi