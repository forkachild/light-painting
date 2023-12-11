#ifndef ALLOC_H
#define ALLOC_H

#define new(type) malloc(sizeof(type))
#define new_n(type, count) malloc((count) * sizeof(type))
#define new_l(bytes) malloc(bytes)

#endif