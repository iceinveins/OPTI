PROJECT_ID=tls_origin
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 -DNDEBUG # -mavx2
if [ $? -eq 0 ]; then
    perf-cache-analyzer.sh -- ./${PROJECT_ID}.elf
fi

PROJECT_ID=tls_opti
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 -DNDEBUG # -mavx2
if [ $? -eq 0 ]; then
    perf-cache-analyzer.sh -- ./${PROJECT_ID}.elf
fi