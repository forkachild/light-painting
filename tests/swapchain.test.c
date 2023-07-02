#include "swapchain.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

static Swapchain swapchain = {
    .size = 0,
    .length = 0,
    .cursor = NULL,
};

static void test_init() {
    swapchain_init(&swapchain, 1024, 3);
    assert(swapchain.cursor != NULL && "Swapchain cursor is NULL after init");
    assert(swapchain.length == 3 && "Swapchain length is not correct");
    assert(swapchain.size == 1024 && "Swapchain buffer size not correct");
}

int main() { return EXIT_SUCCESS; }