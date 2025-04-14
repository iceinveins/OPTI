#include <vector>
#include <random>
#include <functional>

template<typename T>
std::vector<T> GenerateDiffNumber(T min, T max, int cnt) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::function<T ()> generator;
    if (std::is_same<T, int>::value) {
        std::uniform_int_distribution<int> dist(min, max);
        generator = [=]() mutable {return dist(gen);};
    }
    else if(std::is_same<T, float>::value){
        std::uniform_real_distribution<float> dist(min, max);
        generator = [=]() mutable {return dist(gen);};
    }
    else if(std::is_same<T, double>::value){
        std::uniform_real_distribution<double> dist(min, max);
        generator = [=]() mutable {return dist(gen);};
    }
    
    std::vector<T> diff(cnt);
    // std::vector<T> tmp(max - min + 1);
    // std::iota(tmp.begin(), tmp.end(), min);

    // for (int i = 0; i < cnt; ++i) {
    //     auto rnd = generator();
    //     diff.push_back(tmp[rnd - min]);
    //     std::swap(tmp[rnd - min], tmp[tmp.size() - 1]);
    //     tmp.pop_back();
    // }
    for (int i = 0; i < cnt; ++i) {
        diff[i]=generator();
    }
    return diff;
}