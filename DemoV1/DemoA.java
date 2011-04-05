import java.util.*;

class iNode {
    public int root;
    public iNode left;
    public iNode right;

    public iNode(int key) {
	root = key;
	left = null;
	right = null;
    }
}

class iTree {
    private iNode root;
    private boolean first;

    public iTree() {
	root = null;
    }

    public boolean add(int key) {
	if (root == null) {
	    root = new iNode(key);
	    return true;
	}
	else
	    return insert(key, root);
    }

    private boolean insert(int key, iNode node) {
	if (key < node.root)
	    if (node.left == null) {
		node.left = new iNode(key);
		return true;
	    }
	    else 
		return insert(key, node.left);
	else if (key > node.root)
	    if (node.right == null) {
		node.right = new iNode(key);
		return true;
	    }
	    else
		return insert(key, node.right);
	else
	    return false;
    }


    public void print() {
	first = true;
	inorder(root);
	System.out.print('\n');
    }
    
    private void inorder(iNode node) {
	if (node == null) return;

	inorder(node.left);
	visit(node);
	inorder(node.right);
    }

    private void visit(iNode node) {
	if (first) {
	    first = false;
	    System.out.print(' ');
	}
	else System.out.print(',');
	

	System.out.print(node.root);
    }
}

/* Without using an Array is DemoB */
class Message {
    private char msg[] = new char[100];
    int i;
    int end;

    public Message() {
	msg[0] = 'A';
	msg[1] = 'd';
	msg[2] = 'd';
	msg[3] = 'e';
	msg[4] = 'd';
	msg[5] = ' ';
	msg[6] = 'E';
	msg[7] = 'l';
	msg[8] = 'e';
	msg[9] = 'm';
	msg[10] = 'e';
	msg[11] = 'n';
	msg[12] = 't';
	msg[13] = ':';
	i = 14;
	msg[i] = 'D';
	msg[i+1] = 'u';
	msg[i+2] = 'p';
	msg[i+3] = 'l';
	msg[i+4] = 'i';
	msg[i+5] = 'c';
	msg[i+6] = 'a';
	msg[i+7] = 't';
	msg[i+8] = 'e';
	msg[i+9] = 'm';

	msg[24] = 'E';
	msg[25] = 'n';
	msg[26] = 't';
	msg[27] = 'e';
	msg[28] = 'r';
	
    }

    private void print(int i, int end) {
	while (i <= end) {
	    System.out.print(msg[i]);
	    i = i + 1;
	}
    }	

    /* Added */
    public void msg1(int elm) {
	print(0, 13);
	System.out.print(elm);
	System.out.print('\n');
    }

    /* Duplicate */
    public void msg2(int elm) {
	i = 14;
	end = (i + 9);
	print(i, end);
	System.out.print(msg[5]);
	print(6, 13);
	System.out.print(elm);
	System.out.print('\n');
    }
    
    public void msg3() {
	print(24, 28);
	i = 5;
	print(i, i);
	print(6, 13);
    }
}

class DemoA {
   
    public static void main(String arg[]) {
	Scanner cin = new Scanner(System.in);
	int key;
	iTree tree;
	Message msg = new Message();
	tree = new iTree();

	msg.msg3();
	key = cin.nextInt();
	System.out.print('\n');
	while (key != 0) {
	    if (tree.add(6 + key * 1000 - 3)) {
		msg.msg1(key);
		tree.print();
	    }
	    else 
		msg.msg2(key);

	msg.msg3();
	key = cin.nextInt();
	System.out.print('\n');
	}
    }
}