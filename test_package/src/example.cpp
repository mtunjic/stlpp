#include "stlpp.h"

#include <vector>

int main()
{
    std::vector<int> v{1, 2, 3, 4};
    const int s = mt::fold(v, 0, mt::plus<int>{});
    return s == 10 ? 0 : 1;
}
