/*
 * 爬楼梯（动态规划）
 * 输入：正整数 n（1 <= n <= 45）
 * 输出：每次可爬 1 或 2 阶，到达第 n 阶的不同方法数
 */
#include <stdio.h>

int main(void) {
    int n;
    if (scanf("%d", &n) != 1) {
        return 0;
    }
    if (n <= 1) {
        printf("1\n");
        return 0;
    }

    long long prev2 = 1; /* f(1) */
    long long prev1 = 2; /* f(2) */
    for (int i = 3; i <= n; ++i) {
        long long cur = prev2 + prev1;
        prev2 = prev1;
        prev1 = cur;
    }
    printf("%lld\n", prev1);
    return 0;
}
