#include <vector>
#include "test_performance.hpp"
 
int main(int argc, char** argv) {
    int size = std::stoi(argv[1]);
    std::vector<std::vector<uint64_t>> data(size, std::vector<uint64_t>(size));
    auto func=[&](int core_id) {
        for (int col = 0; col < size; ++col) {
            for (int row = 0; row < size; ++row) {
                data[row][col] = rand();
            }
        }
    };
    test_performance("test_nums", func, 1);
}