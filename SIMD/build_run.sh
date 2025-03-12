g++ -o simd_demo.elf simd_demo.cpp -march=native -msse2 -O3 # -mavx2
if [ $? -eq 0 ]; then
    ./simd_demo.elf
fi