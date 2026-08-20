//
//  main.cpp
//  STLProject
//
//  Created by Marko Tunjic on 05/05/16.
//  Copyright © 2016 Marko Tunjic. All rights reserved.
//

#include "algorithms.h"
#include "functors.h"
#include "utils.h"
#include <array>
#include <iostream>

int main(int argc, const char* argv[])
{
    // slide
    std::cout << "TEST SLIDE: \n"
              << "=========================\n";
    std::array<int, 10> a10{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
    std::cout << "Before gather: \n";
    print(a10);
    std::cout << "After we gather all less than 2: \n";
    auto insertPos = std::begin(a10) + 4;
    mt::gather(std::begin(a10), std::begin(a10) + 10, insertPos, less_than<int>{2});
    print(a10);
    std::cout << "=========================\n\n\n";

    // find_backward
    std::cout << "TEST FIND_BACKWARD: \n"
              << "=========================\n";
    std::vector<int> arr{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::cout << "Find 7 backward: \n";
    auto p7 = mt::find_backward(std::begin(arr), std::end(arr), 7);
    std::cout << "=========================\n\n\n";

    // remove_if_not
    std::cout << "TEST REMOVE_IF_NOT: \n"
              << "=========================\n";
    std::vector<int> v1{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
    std::cout << "Before remove: \n";
    print(v1);
    // mark for deletion
    auto p = mt::remove_if_not(std::begin(v1), std::end(v1), less_than<int>{5});
    v1.erase(p, v1.end());
    std::cout << "After remove not less than 5: \n";
    print(v1);
    std::cout << "=========================\n\n\n";

    // keep_if
    std::cout << "TEST KEEP_IF: \n"
              << "=========================\n";
    std::vector<int> v2{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
    std::cout << "Before: \n";
    print(v2);
    std::cout << "After keep all less than 8: \n";
    mt::keep_if(v2, less_than<int>{8});
    print(v2);
    std::cout << "=========================\n\n\n";

    // stable_partition_position
    int a[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};
    bool b[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
    auto pos = mt::stable_partition_position(
        std::begin(a), std::end(a), [&](auto i) { return *(std::begin(b) + (i - std::begin(a))); });
    for (auto f = std::begin(a), l = pos; f != l; ++f)
        std::cout << *f << " ";
    std::cout << "^ ";
    for (auto f = pos, l = std::end(a); f != l; ++f)
        std::cout << *f << " ";
    std::cout << std::endl;

    // 2 4 4 2 ^ 1 3 5 5 3 1

    // random
    std::cout << "TEST RANDOMIZE: \n"
              << "=========================\n";
    using card = int;
    std::array<card, 52> deck{};
    std::iota(begin(deck), end(deck), 0);
    // Shuffle the deck:
    mt::randomize(deck);
    print(deck);

    //int rInt = mt::pick_a_number(5, 15);
    //double rDouble = mt::pick_a_number(0.5, 5.0);
    //std::cout << "Random int: " << rInt << "\nRandom double: " << rDouble << '\n';

    std::cout << "=========================\n\n\n";

    // test random range
    std::cout << "TEST RANDOM RANGE: \n"
              << "=========================\n";
    std::vector<double> rd(5);
    mt::random(rd, 5.0, 0.5);  // gen radnom range with min and max limit
    print(rd);
    std::cout << "=========================\n\n\n";

    // test split
    std::cout << "TEST SPLIT: \n"
              << "=========================\n";
    std::string line{"man,meal,moon;fat,food,feel;cat,coat,cook;love,leg,lunch"};

    // split string to any container
    std::vector<std::string> vec_words;
    std::list<std::string> list_words;

    mt::split(line, ";", vec_words);
    mt::split(line, ",;.", list_words);

    print(vec_words);
    print(list_words);
    std::cout << "=========================\n\n\n";

    std::cout << std::endl;
    return 0;
}
