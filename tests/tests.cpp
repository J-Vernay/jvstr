#include "doctest.hpp"

#include "../jvstr/StrView.h"
#include "../jvstr/StrAlloc.h"

// C++ functionality: "test"_sv will be equivalent to jvmakeView("test")
StrView operator""_sv(char const* str, size_t) {
    return StrView_make(str);
}

// Doctest feature: how to display our type
doctest::String toString(StrView view) {
    doctest::String str;
    str += "\"";
    str += (doctest::String){view.begin, (unsigned)view.size};
    str += "\"";
    return str;
}




TEST_CASE("StrView strlen,extraction") {
    char const* input_str = "Hello, World!";
    StrView view = StrView_make(input_str);
    
    size_t input_size = strlen(input_str);
    
    // Checking extraction from first to last
    size_t nb_extracted = 0;
    for (StrView str = view; str.size > 0;) {
        char ch = jvstr_extract_first(&str);
        CHECK(ch == input_str[nb_extracted]);
        ++nb_extracted;
    }
    REQUIRE(nb_extracted == input_size);
    
    // Checking extraction from last to first.
    nb_extracted = 0;
    for (StrView str = view; str.size > 0;) {
        char ch = jvstr_extract_last(&str);
        ++nb_extracted;
        CHECK(ch == input_str[input_size-nb_extracted]);
    }
    REQUIRE(nb_extracted == input_size);
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

TEST_CASE("StrView equality,comparison,prefix,suffix") {
    struct Test {
        StrView A;
        StrView B;
        int cmp_sign; // strcmp(A,B)
        bool prefix;  // str_starts_with(A,B)
        bool suffix;  // str_ends_with(A,B)
    };
    
    struct Test tests[] = {
        { "test"_sv, "test"_sv, 0, true, true },       // "test" == "test"
        { "test"_sv, "testing"_sv, -1, false, false }, // "test" < "testing"
        { "testing"_sv, "test"_sv, 1, true, false },   // "testing" > "test"
        { "testing"_sv, "ing"_sv, 1, false, true },    // "testing" > "ing"
        { ""_sv, ""_sv, 0, true, true },               // "" == ""
        { "test"_sv, ""_sv, 1, true, true },           // "" < "test",
        { ""_sv, "test"_sv, -1, false, false }
    };
    
    // C++ feature: ranged-for loop, "for each test in tests"
    for (Test test : tests) {
        // Doctest feature: CAPTURE will display values on failure.
        CAPTURE(test.A);
        CAPTURE(test.B);
        bool should_be_equal = (test.cmp_sign == 0);
        CHECK(jvstr_equal(test.A, test.B) == should_be_equal);
        CHECK(sign(jvstr_compare(test.A, test.B)) == test.cmp_sign);
        CHECK(jvstr_starts_with(test.A, test.B, 0) == test.prefix);
        CHECK(jvstr_ends_with(test.A, test.B) == test.suffix);
    }
}

TEST_CASE("StrView find/search") {
    StrView HelloWorld = "Hello, World!"_sv;
    CHECK(jvstr_find(HelloWorld, 'H') == 0);
    CHECK(jvstr_find(HelloWorld, ' ') == 6);
    CHECK(jvstr_find(HelloWorld, '!') == 12);
    CHECK(jvstr_find(HelloWorld, 'l') == 2); // first 'l' is found
    CHECK(jvstr_find(HelloWorld, 'A') == HelloWorld.size); // not found
    
    CHECK(jvstr_find_unescaped(HelloWorld, 'o', 'e') == 4); // first 'o' not preceded by 'e'
    CHECK(jvstr_find_unescaped(HelloWorld, 'o', 'l') == 8); // first 'o' not preceded by 'l'
    CHECK(jvstr_find_unescaped(HelloWorld, 'H', 's') == 0); // first 'H' not preceded by 's'
    CHECK(jvstr_find_unescaped(HelloWorld, 'A', 'e') == HelloWorld.size); // first 'A' not preceded by 'e', not found
    
    CHECK(jvstr_rfind(HelloWorld, ' ') == 6);
    CHECK(jvstr_rfind(HelloWorld, 'l') == 10); // last 'l' is found
    CHECK(jvstr_rfind(HelloWorld, 'H') == 0); // last 'l' is found
    CHECK(jvstr_rfind(HelloWorld, 'A') == -1); // not found
    
    CHECK(jvstr_while_in(HelloWorld, "elH"_sv, 0) == 4);
    CHECK(jvstr_while_in(HelloWorld, "Hle"_sv, 0) == 4); // order is not important
    CHECK(jvstr_while_in(HelloWorld, "llHHHee"_sv, 0) == 4); // nor duplicates
    CHECK(jvstr_while_in(HelloWorld, "ABCDEFGHIJ"_sv, 0) == 1); // 'H' is included but not 'e'
    CHECK(jvstr_while_in(HelloWorld, "elo"_sv, 0) == 0); // 'H' is not included
    CHECK(jvstr_while_in(HelloWorld, ""_sv, 0) == 0);
    
    CHECK(jvstr_until_in(HelloWorld, "elH"_sv, 0) == 0); // 'H' is found
    CHECK(jvstr_until_in(HelloWorld, "ole"_sv, 0) == 1); // 'e' is found
    CHECK(jvstr_until_in(HelloWorld, ""_sv, 0) == HelloWorld.size);
    CHECK(jvstr_until_in(HelloWorld, " !,"_sv, 0) == 5); // ',' is found
    
    
    CHECK(jvstr_search(HelloWorld, "Hello"_sv) == 0);
    CHECK(jvstr_search(HelloWorld, "World"_sv) == 7);
    CHECK(jvstr_search(HelloWorld, "l"_sv) == 2); // first 'l' found
    CHECK(jvstr_search(HelloWorld, "ABC"_sv) == HelloWorld.size);
    CHECK(jvstr_search(HelloWorld, "Hella"_sv) == HelloWorld.size);
}
#include <cstdio>
TEST_CASE("StrView split") {
    // With str_find
    {
        // Note: with ",,", we detect an empty element betwen first ',' and second ','
        // This can be easily filtered by doing `if (jvstrlen(element) == 0) continue;`.
        StrView list = "some,list,of  things,hello,,world !"_sv;
        StrView expected[6] = { "some"_sv, "list"_sv, "of  things"_sv, "hello"_sv, ""_sv, "world !"_sv };
        int nb_split = 0;
        do {
            StrView element = jvstr_split(&list, jvstr_find(list, ','), 1);
            CAPTURE(element);
            CAPTURE(expected[nb_split]);
            CHECK(jvstr_equal(element, expected[nb_split]));
            ++nb_split;
        } while (list.size > 0);
        CHECK(nb_split == 6);
    }
    
    // With str_search_
    {
        StrView list = "another---list of---things---super-cool--testing---end"_sv;
        StrView expected[5] = { "another"_sv, "list of"_sv, "things"_sv, "super-cool--testing"_sv, "end"_sv };
        StrView delimiter = "---"_sv;
        int nb_split = 0;
        do {
            StrView element = jvstr_split(&list, jvstr_search(list, delimiter), delimiter.size);
            CAPTURE(element);
            CAPTURE(expected[nb_split]);
            CHECK(jvstr_equal(element, expected[nb_split]));
            ++nb_split;
        } while (list.size > 0);
        CHECK(nb_split == 5);
    }
    
    // With str_until_in
    {
        StrView list = "data0 data1\tdata2\tdata3 data4\r\ndata5"_sv;
        StrView expected[7] = { "data0"_sv, "data1"_sv, "data2"_sv, "data3"_sv, "data4"_sv, ""_sv, "data5"_sv };
        StrView delimiters = " \t\r\n"_sv;
        int nb_split = 0;
        do {
            StrView element = jvstr_split(&list, jvstr_until_in(list, delimiters, 0), 1);
            CAPTURE(element);
            CAPTURE(expected[nb_split]);
            CHECK(jvstr_equal(element, expected[nb_split]));
            ++nb_split;
        } while (list.size > 0);
        CHECK(nb_split == 7);
    }
}

TEST_CASE("StrAlloc") {
    StrAlloc str = StrAlloc_make("Hello, World!"_sv);
    CHECK(jvstr_equal(STRALLOC_ASVIEW(str), "Hello, World!"_sv));
    
    StrAlloc_reserve(&str, 100);
    CHECK(str.capacity >= 100);
    
    StrAlloc_insert(&str, "bonjour, "_sv, jvstr_find(STRALLOC_ASVIEW(str), 'W'));
    CAPTURE((STRALLOC_ASVIEW(str)));
    CHECK(jvstr_equal(STRALLOC_ASVIEW(str), "Hello, bonjour, World!"_sv));
    
    StrAlloc_append(&str, " :-)"_sv);
    CAPTURE((STRALLOC_ASVIEW(str)));
    CHECK(jvstr_equal(STRALLOC_ASVIEW(str), "Hello, bonjour, World! :-)"_sv));
    
    StrAlloc_destroy(&str);
}
