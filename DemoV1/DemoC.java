import java.util.*;

class DemoC {
    DemoC() {}

    public static int fib(int k) {
	if (k == 0) return 0;
	else if (k == 1) return 1;
	else return (fib(k - 1) + fib(k - 2));
    }

    public static void main(String arg[]) {
	Scanner cin = new Scanner(System.in);

	int k;
	int j;
	int sum;

	System.out.print('>');
	k = cin.nextInt();
	System.out.print('\n');

	while (k != 0) {
	    if (k < 0) {
		k = (k * -1);
		sum = 0;
		while (k > 0) {
		    j = k;
		    while (j >= 1) {
			if ((j % 3) == 0)
			    sum = 2 * sum + j * 3;
			j = j - 1;
		    }
		    k = k - 1;
		}

		System.out.print(sum);
		System.out.print('\n');
	    }
	    else {
		sum = DemoC.fib(k);
		System.out.print(sum);
		System.out.print('\n');
	    }

	    System.out.print('>');
	    k = cin.nextInt();
	    System.out.print('\n');

	}
    }
}
