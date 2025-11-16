#include <filesystem>
#include "fs_test_case.h"
#include "OSUtils.h"

namespace fs = std::filesystem;

using namespace chaudiere;

FSTestCase::FSTestCase(TestSuite& the_test_suite,
                       const std::string& the_test_dir) :
   test_suite(the_test_suite),
   test_dir(the_test_dir) {
   setup();
}

FSTestCase::~FSTestCase() {
   teardown();
}

void FSTestCase::setup() {
   if (OSUtils::directoryExists(test_dir)) {
      fs::remove_all(test_dir);
   }
   test_suite.requireFalse(OSUtils::directoryExists(test_dir), "test dir must not exist");
   test_suite.require(OSUtils::createDirectory(test_dir), "create test dir must succeed");
   test_suite.require(OSUtils::directoryExists(test_dir), "test dir must exist");
}

void FSTestCase::teardown() {
   fs::remove_all(test_dir);
}

