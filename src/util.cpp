#include "util.hpp"

#include <math.h>

#include "defines.hpp"

// Method to compare which one is the more close.
// We find the closest by taking the difference
// between the target and both values. It assumes
// that val2 is greater than val1 and target lies
// between these two.
float get_closest(float val1, float val2, float target)
{
  return (target - val1 <= val2 - target) ? val1 : val2;
}

// Returns element closest to target
float binary_search(float arr[], int n, float target)
{
  if (target <= arr[0]) {
    return arr[0];
  }

  if (target >= arr[n - 1]) {
    return arr[n - 1];
  }

  int left = 0, right = n, mid = 0;

  while (left < right) {
    mid = (left + right) / 2;

    if (target == mid) {
      return arr[mid];
    }

    if (target < arr[mid]) {
      if (mid > 0 && target > arr[mid - 1]) {
        return get_closest(arr[mid - 1], arr[mid], target);
      }
      right = mid;
    } else {
      if (mid < n - 1 && target < arr[mid + 1]) {
        return get_closest(arr[mid], arr[mid + 1], target);
      }
      left = mid + 1;
    }
  }

  return arr[mid];
}
