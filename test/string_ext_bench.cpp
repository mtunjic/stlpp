//
//  bench.cpp
//  STLExt
//
//  Created by Marko Tunjic on 11/05/16.
//  Copyright © 2016 Marko Tunjic. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>

#include "string_ext.h"
#include <assert.h>
#include <chrono>
#include <locale>
#include <memory>

using namespace mt;
using namespace std;

int main(int argc, const char* argv[])
{
    timer t;

    string millionChars = random_string_ecape(1000000);  // One million
    //std::setlocale(LC_ALL, "en_US.UTF-8");
    auto badstr{u8"some,12<script>-1</script>=1test:\\\[29m;中文710-1---,.1888ƒ®£££Osss中文\n"};

    std::locale::global(std::locale{""});
    init_locale();

    //=====================================================================
    // => std::copy_if (string): 3,026ms
    //=====================================================================
    string dest1{}, before{}, after{};
    dest1.reserve(millionChars.length());
    t.start();

    copy_if(millionChars, dest1, ::isdigit);
    double s1time = t.stop();

    before.assign(millionChars.begin(), millionChars.begin() + 80);
    cout << "Before: " << before << endl;

    after.assign(dest1.begin(), dest1.begin() + 80);
    cout << "After: " << after << endl;
    cout << "\033[1;31m=> std::copy_if (string): " << s1time << "ms\n\033[0m\n";
    cout << "=================================================================\n\n\n";
    // 3,122ms

    //=====================================================================
    // => std::remove_copy_if (vector + fun): 3,162ms
    //=====================================================================
    string s2{}, before2{}, after2{};
    vector<char> src2(millionChars.begin(), millionChars.end());
    vector<char> dest2{};
    dest2.clear();
    dest2.reserve(src2.size());

    t.start();

    std::remove_copy_if(src2.begin(), src2.end(), std::back_inserter(dest2), ::isdigit);
    //[](const char c) {return ::isdigit(c); });

    double s2time = t.stop();

    before2.assign(millionChars.begin(), millionChars.begin() + 80);
    cout << "Before: " << before2 << endl;
    after2.assign(dest2.begin(), dest2.begin() + 80);

    cout << "After: " << after2 << endl;
    cout << "\033[1;31m=> std::remove_copy_if (vector + fun): " << s2time << "ms\n\033[0m"
         << std::endl;
    cout << "=================================================================\n\n\n";
    // 3ms

    //=====================================================================
    // => remove_copy_if (vector/raw loop iterator): 2,998ms
    //=====================================================================
    string s3{}, before3{}, after3{};
    vector<char> src3(millionChars.begin(), millionChars.end());
    vector<char> dest3{};
    dest3.clear();
    dest3.reserve(src3.size());
    t.start();

    for (auto it = src3.begin(), j = dest3.begin(), end = src3.end(); it != end; ++it, ++j) {
        auto ch = *it;
        if (::isdigit(ch))
            *j = ch;
    }
    double s3time = t.stop();

    before3.assign(millionChars.begin(), millionChars.begin() + 80);
    cout << "Before: " << before3 << endl;

    after3.assign(dest3.begin(), dest3.begin() + 80);
    cout << "After: " << after3 << endl;
    cout << "\033[1;31m=> remove_copy_if (vector/iterator): " << s3time << "ms\n\033[0m\n";
    cout << "=================================================================\n\n\n";
    // 3,160ms

    //=====================================================================
    // => remove_copy_if (string/ raw loop iterator): 4,281ms
    //=====================================================================
    string before4{}, after4{};
    string dest4{};
    string src4 = millionChars;  // One million

    dest4.clear();
    dest4.reserve(millionChars.length());

    t.start();

    for (auto it = src4.begin(), end = src4.end(); it != end; ++it) {
        auto ch = *it;
        if (::isdigit(ch))
            dest4 += *it;
    }

    double s4time = t.stop();

    before4.assign(src4.begin(), src4.begin() + 80);
    cout << "Before: " << before4 << endl;

    after4.assign(dest4.begin(), dest4.begin() + 80);
    cout << "After: " << after4 << endl;
    cout << "\033[1;31m=> remove_copy_if (string/iterator): " << s4time << "ms\n\033[0m\n";
    cout << "=================================================================\n\n\n";
    // 3,152ms
}
