#ifndef ALLOC_H
#define ALLOC_H

#include <malloc.h>

#define new(type) malloc(sizeof(type))
#define new_n(type, count) malloc((count) * sizeof(type))
#define new_l(bytes) malloc(bytes)

static void print_mallinfo() {

    struct mallinfo minfo = mallinfo();
    printf("Memory info:\n"
           "  arena = %u\n"
           "  ordblks = %u\n"
           "  smblks = %u\n"
           "  hblks = %u\n"
           "  hblkhd = %u\n"
           "  usmblks = %u\n"
           "  fsmblks = %u\n"
           "  uordblks = %u\n"
           "  fordblks = %u\n"
           "  keepcost = %u\n",
           minfo.arena, minfo.ordblks, minfo.smblks, minfo.hblks, minfo.hblkhd,
           minfo.usmblks, minfo.fsmblks, minfo.uordblks, minfo.fordblks,
           minfo.keepcost);
}

#endif