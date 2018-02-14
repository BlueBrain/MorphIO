#include "googletest/googletest/include/gtest/gtest.h"
#include "parse_ll.cpp"

#include <string>

using std::string;

void check_parse(string& input)
{
    minimorph::NeurolucidaParser np;
    bool ret = np.parse(input);
    ASSERT_TRUE(ret);
}

TEST(PARSE, COLOR)
{
    auto input = string("(Color Yello)");
    check_parse(input);

    input = string("(Color RGB (255, 255, 128))");
    check_parse(input);
}

TEST(PARSE, SEXP)
{
    string input(R""(
    (word)
    (word "string")
    (word 1.23)
    (word 123)
    (word -123.1e-10)
  )"");
    check_parse(input);
}

TEST(PARSE, SIMPLE_SEXP)
{
    string input("(ImageCoords)");
    check_parse(input);
}

TEST(PARSE, CELLBODY_START)
{
    string input(R""(
    ("CellBody"
        (CellBody)
    )
  )"");
    check_parse(input);
}

TEST(PARSE, CELLBODY_COLOR)
{
    string input(R""(
    ("CellBody"
        (Color RGB (255, 255, 128))
    )
  )"");
    check_parse(input);

    input = string(R""(
    ("CellBody"
        (Color RGB (255, 255, 128))
        (CellBody)
    )
  )"");
    check_parse(input);
}

TEST(PARSE, CELLBODY_SECTIONS)
{
    string input(R""(
    ("CellBody"
        (Color RGB (255, 255, 128))
        (CellBody)
        (1 2 3 4)
        (2 3 4 5)
        (3 4 5 6)
        (4 5 6 7)
    )
  )"");
    check_parse(input);
}

TEST(PARSE, SIMPLEST_NEURITE)
{
    std::string input(R""(
 ((Color RGB (255, 255, 128))
  (Axon)
  (1 1 1 1)
  (2 2 2 2)
  (3 3 3 3)
 )
)"");
    check_parse(input);
}

TEST(PARSE, SIMPLE_NEURITE)
{
    std::string input(R""(
 ((Color RGB (255, 255, 128))
  (Axon)
  (1 1 1 1)
  (2 2 2 2)
  (3 3 3 3)
  (
    (4 4 4 5)
    (5 5 5 5)
  |
    (6 6 6 6)
    (7 7 7 7)
  )
)
)"");
    check_parse(input);
}

TEST(PARSE, SIMPLE_NEURITE_MULTI_BRANCH)
{
    std::string input(R""(
 ((Color RGB (255, 255, 128))
  (Axon)
  (1 1 1 1)
  (2 2 2 2)
  (3 3 3 3)
  (
    (4 4 4 5)
    (5 5 5 5)
  |
    (6 6 6 6)
    (7 7 7 7)
  |
    (8 8 8 8)
    (9 9 9 9)
  )
)
)"");
    check_parse(input);
}

TEST(PARSE, NEURITE_WITH_SPINES)
{
    std::string input(R""(
( (Color DarkGreen)
  (Dendrite)
  (   31.72   -54.49     0.65     1.15)  ; Root
  (   28.33   -52.82     0.65     1.15)  ; 1, R
  (   27.92   -52.88    -0.63     1.18)  ; 2
  (   27.92   -52.88    -0.63     1.18)  ; 2
  <(   28.36   -54.52     0.31     0.37)>  ; Spine
  <(   27.27   -51.31    -0.44     0.37)>  ; Spine
)
)"");
    check_parse(input);
}

TEST(PARSE, NEURITE_BODY_AND_MARKER)
{
    std::string input(R""(
((Color Yellow)
  (Axon)
  (1 1 1 1)
  (2 2 2 2)
  (OpenCircle
      (Color Yellow)
      (Name "Marker 2")
      (1000 1000 1000 1000)
  )
)
)"");
    check_parse(input);
}

TEST(PARSE, NEURITE_BODY_WITH_BIFURCATIONS_AND_MARKER)
{
    std::string input(R""(
 ((Axon)
  (1 1 1 1)
  (2 2 2 2)
  (3 3 3 3)
  (
    (4 4 4 5)
    (5 5 5 5)
    (OpenCircle
        (Color Yellow)
        (Name "Marker 2")
        (1000 1000 1000 1000)
    )
    High
  |
    (6 6 6 6)
    (7 7 7 7)
  )
)
)"");
    check_parse(input);
}

TEST(PARSE, NEURITE_BODY__WITH_BIFURCATIONS_NESTED)
{
    std::string input(R""(
((Color Yellow)
 (Axon)
 (1 1 1 1)
 (2 2 2 2)
 (3 3 3 3)
 (
  (4 4 4 4)
  (4 4 4 4)
  (
   (5 5 5 5)
   (6 6 6 6)
  |
   (8 8 8 8)
   (9 9 9 9)
  )
 |
  (10 10 10 10)
  (11 11 11 11)
  (12 12 12 12)
 |
  (13 13 13 13)
  (14 14 14 14)
  (15 15 15 15)
 )
)
)"");
    check_parse(input);
}

TEST(PARSE, TOP_LEVEL_MARKER)
{
    std::string input(R""(
( (Color White)
  (Font "Trebuchet MS" 26)
  (  -15.11    15.56    -2.12)
  "E"
)  ;  End of text
)"");
    check_parse(input);
}
