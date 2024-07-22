#ifndef UTIL_H
#define UTIL_H

typedef struct {
  double amp;
  double frq;
  int    idx;
} peak_t;

float binary_search(float arr[], int n, float target);

#endif // UTIL_H
