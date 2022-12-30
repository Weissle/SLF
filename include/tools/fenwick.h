#include <vector>
namespace slf
{

template <typename T>
struct fenwick
{
    std::vector<T> p;
    int n, offset;
    fenwick() {}
    fenwick(int _n, int _offset)
    {
		reset(_n,_offset);
    }
    int lowbit(int x) { return x & (-x); }

    void reset(int _n, int _offset)
    {
        n = _n + 1;
        offset = _offset;
        p.clear();
        p.resize(n, T{});
    }

    T query(int x)
    {
        x += offset;
        T ret = 0;
        while (x)
        {
            ret += p[x];
            x -= lowbit(x);
        }
        return ret;
    }

    void add(int x, T val)
    {
        x += offset;
        while (x < n)
        {
            p[x] += val;
            x += lowbit(x);
        }
    }
};
} // namespace slf
