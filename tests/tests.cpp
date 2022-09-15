#include "tests.h"

#include "test_utils.h"


void Tests::run() {
   TestUtils test_utils;
   test_utils.run();
}

int main(int argc, char* argv[]) {
   Tests tests;
   tests.run();
}
