PROJECT_ID=GlobalLock
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 # -mavx2
if [ $? -eq 0 ]; then
    ./${PROJECT_ID}.elf
fi

PROJECT_ID=SegmentedLock
echo "==================${PROJECT_ID}======================"
g++ -o ${PROJECT_ID}.elf ${PROJECT_ID}.cpp -std=c++2a -latomic -lpthread -march=native -msse2 -O3 # -mavx2
if [ $? -eq 0 ]; then
    ./${PROJECT_ID}.elf
fi