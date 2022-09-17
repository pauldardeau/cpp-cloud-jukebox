#include <filesystem>
#include "fs_test_case.h"
#include "OSUtils.h"

namespace fs = std::filesystem;

FSTestCase::FSTestCase(poivre::TestSuite& the_test_suite,
                       const std::string& the_test_dir) :
   test_suite(the_test_suite),
   test_dir(the_test_dir) {
   setup();
}

FSTestCase::~FSTestCase() {
   teardown();
}

void FSTestCase::setup() {
   if (chaudiere::OSUtils::directoryExists(test_dir)) {
      fs::remove_all(test_dir);
   }
   test_suite.requireFalse(chaudiere::OSUtils::directoryExists(test_dir), "test dir must not exist");
   test_suite.require(chaudiere::OSUtils::createDirectory(test_dir), "create test dir must succeed");
   test_suite.require(chaudiere::OSUtils::directoryExists(test_dir), "test dir must exist");
}

void FSTestCase::teardown() {
   fs::remove_all(test_dir);
}

