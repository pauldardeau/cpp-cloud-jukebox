#include "tests.h"

#include "test_utils.h"
#include "test_property_set.h"
#include "test_argument_parser.h"
#include "test_jukebox_db.h"
#include "test_s3_storage_system.h"
#include "test_fs_storage_system.h"
#include "test_jukebox.h"


void Tests::run() {
   TestUtils test_utils;
   test_utils.run();

   TestPropertySet test_ps;
   test_ps.run();

   TestArgumentParser test_ap;
   test_ap.run();

   TestJukeboxDB test_jbdb;
   test_jbdb.run();

   TestS3StorageSystem test_s3;
   test_s3.run();

   TestFSStorageSystem test_fs;
   test_fs.run();

   TestJukebox test_jb;
   test_jb.run();
}

int main(int argc, char* argv[]) {
   Tests tests;
   tests.run();
}
