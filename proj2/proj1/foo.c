#include <stdio.h>

int main() {
  int n;
  int sum;
  sum = 0;
  for (n = 0; n < 10; n++)
    sum = sum + n*n;
  printf("sum: %d\n", sum);
  return 0;
}
