#include "tests.h"

#include "test_utils.h"
#include "test_property_set.h"


void Tests::run() {
   TestUtils test_utils;
   test_utils.run();

   TestPropertySet test_ps;
   test_ps.run();
}

int main(int argc, char* argv[]) {
   Tests tests;
   tests.run();
}
