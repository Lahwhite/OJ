# 爬楼梯（动态规划）
# 输入：正整数 n（1 <= n <= 45）
# 输出：每次可爬 1 或 2 阶，到达第 n 阶的不同方法数

n = int(input())
if n <= 1:
    print(1)
else:
    prev2, prev1 = 1, 2
    for _ in range(3, n + 1):
        prev2, prev1 = prev1, prev2 + prev1
    print(prev1)
