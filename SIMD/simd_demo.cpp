#include <iostream>
#include <chrono>
#include <cpuid.h>
#include <vector>
#include <random>
#include <functional>
#include <emmintrin.h>

template<typename T>
std::vector<T> GenerateDiffNumber(T min, T max, int num) {
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
    
    std::vector<T> diff;
    // std::vector<T> tmp(max - min + 1);
    // std::iota(tmp.begin(), tmp.end(), min);

    // for (int i = 0; i < num; ++i) {
    //     auto rnd = generator();
    //     diff.push_back(tmp[rnd - min]);
    //     std::swap(tmp[rnd - min], tmp[tmp.size() - 1]);
    //     tmp.pop_back();
    // }
    for (int i = 0; i < num; ++i) {
        diff.push_back(generator());
    }
    return diff;
}

float normal_sum(const std::vector<float>& arr, size_t n) {
    float sum = 0.0f;
    for(size_t i=0; i<n; ++i) {
        sum += arr[i];
    }
    return sum;
}

float sse_sum(const std::vector<float>& arr, size_t n) {
    __m128 sum = _mm_setzero_ps();
    for(size_t i=0; i<n; i+=4) {
        __m128 vec = _mm_loadu_ps(&arr[i]);
        sum = _mm_add_ps(sum, vec);
    }
    
    // 水平相加
    sum = _mm_add_ps(sum, sum);
    sum = _mm_add_ps(sum, sum);
    
    float result;
    _mm_store_ss(&result, sum);
    return result;
}

bool isSSE2Supported() {
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    return (edx & (1 << 26)) != 0;
}

int main() {
    if (isSSE2Supported()) {
        __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
        __m128 result = _mm_add_ps(a, b);
        std::cout << "SSE2 is supported. Result calculated." << std::endl;
    }
    else {
        std::cout << "SSE2 not supported. Quit!"  << std::endl;
        return -1;
    }
    int num = 204800;
    auto result = GenerateDiffNumber<float>(1.0f, 1000000.0f, num);
    // A
    {
        auto start = std::chrono::high_resolution_clock::now();
        normal_sum(result, num);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "A 耗时" << duration.count() << "纳秒" << std::endl;
    }
    // B
    {
        auto start = std::chrono::high_resolution_clock::now();
        sse_sum(result, num);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "B 耗时" << duration.count() << "纳秒" << std::endl;
    }
    return 0;
}
