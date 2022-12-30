#pragma once
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
namespace slf
{

inline bool open_fstream(const std::string& path, std::ifstream& stream)
{
    stream.open(path, std::ios_base::in);
    return stream.is_open();
}

template <typename T>
void sort_and_unique(std::vector<T>& container, bool need_sort = true)
{
    if (need_sort)
        std::sort(container.begin(), container.end());
    else
        assert(std::is_sorted(container.begin(), container.end()));
    container.erase(std::unique(container.begin(), container.end()),
                    container.end());
}

inline void sleep_for_us(size_t us)
{
    if (us)
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    else
        std::this_thread::yield();
}

} // namespace slf
/*
size_t calHashSuitableSize(const size_t need)
{
    size_t i = 8;
    while (i < need)
        i = i << 1;
    if (i * 0.8 > need)
        return i;
    else
        return i << 1;
}

#define PRINT_TIME_COST_S(INFO, T)                                             \
    std::cout << INFO << double(T) / CLOCKS_PER_SEC << endl
#define PRINT_TIME_COST_MS(INFO, T)                                            \
    std::cout << INFO << double(T) / (CLOCKS_PER_SEC / 1000) << endl
#define LOOP(V, H, T) for (auto V = H; V < T; ++V)
#define LOCK_TO_END(M) lock_guard<mutex> lg(M)
*/
