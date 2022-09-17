#ifndef FS_TEST_CASE_H
#define FS_TEST_CASE_H

#include <string>

#include "TestSuite.h"


class FSTestCase {
private:
   poivre::TestSuite& test_suite;
   std::string test_dir;

   FSTestCase(const FSTestCase&);
   FSTestCase& operator=(const FSTestCase&);
public:
   FSTestCase(poivre::TestSuite& the_test_suite,
              const std::string& the_test_dir);
   ~FSTestCase();
   void setup();
   void teardown();
};

#endif

