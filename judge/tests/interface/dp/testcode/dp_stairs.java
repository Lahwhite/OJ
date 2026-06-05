import java.util.Scanner;

/**
 * 爬楼梯（动态规划）
 * 输入：正整数 n（1 <= n <= 45）
 * 输出：每次可爬 1 或 2 阶，到达第 n 阶的不同方法数
 */
public class dp_stairs {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        int n = scanner.nextInt();
        scanner.close();

        if (n <= 1) {
            System.out.println(1);
            return;
        }

        long prev2 = 1;
        long prev1 = 2;
        for (int i = 3; i <= n; ++i) {
            long cur = prev2 + prev1;
            prev2 = prev1;
            prev1 = cur;
        }
        System.out.println(prev1);
    }
}
