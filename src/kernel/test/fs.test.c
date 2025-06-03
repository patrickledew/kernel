#include "fs.test.h"
#include "core/fs/fs.h"
#include "util/assert.h"
#include "core/mem/alloc.h"
#include "core/mem/memory.h"
#include "util/strutil.h"
#include "util/logging.h"

void fs_test() {
    log_info("fs_test: running filesystem tests.");

    // 1. Open invalid file
    // int fd = file_open("/BOGUS.TXT");
    // assert_i(-1, fd);
    // // 2. Open invalid directory
    // fd = file_open("/bogusdir/BOGUS.TXT");
    // assert_i(-1, fd);
    // // 3. Close file that hasnt been opened, or fd outside range
    // assert_i(-1, file_close(42));
    // assert_i(-1, file_close(-5));
    // assert_i(-1, file_close(MAX_OPEN_FILES));
    // // 4. Open file successfully;
    // fd = file_open("/TEST.TXT");
    // assert_not_i(-1, fd);
    // assert_i(0, ktell(fd));

    // // 5. Writing to file
    // char test_str[20] = "Testing testing 123";
    // assert_not_i(-1, file_write(fd, test_str, strlen(test_str)));
    // assert_i(strlen(test_str), ktell(fd));
    // assert_not_i(-1, file_write(fd, test_str, strlen(test_str)));
    // assert_i(strlen(test_str) * 2, ktell(fd));
    // // 7. Reading it back
    // char* buf = alloc(8192);
    // memfill(buf, 8192, 0);
    
    // kseek(fd, 0); // Go to beginning of file
    // assert_not_i(-1, file_read(fd, buf, strlen(test_str)));
    // assert_s(test_str, buf); // Ensure string has been written there
    
    // memfill(buf, 8192, 0);

    // kseek(fd, strlen(test_str)); // Go to where second string was written
    // assert_not_i(-1, file_read(fd, buf, strlen(test_str), 0));
    // assert_s(test_str, buf); // Ensure string has been written there as well

    // memfill(buf, 8192, 0);

    // // 8. Writing more than 1 cluster and growing the file
    // kseek(fd, 0);
    // memfill(buf, 8192, 'A');
    // assert_not_i(-1, file_write(fd, buf, 8192, 0));
    // kseek(fd, 0);
    // memfill(buf, 8192, 0);
    // assert_not_i(-1, file_read(fd, buf, 8192, 0));
    // for (int i = 0; i < 8192; i++) {
    //     assert_u8('A', buf[i]);
    // }

    // file_close(fd);
    // uint8_t color = print_color_get();
    // print_color_set(0x0A);
    // log_info("fs_test: filesystem tests completed successfully.");
    // print_color_set(color);
    log_error("fs_test: Filesystem tests need to be re-implemented now that we are storing file position per-process.");
}