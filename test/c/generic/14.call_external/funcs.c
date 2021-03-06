// void-void func
__attribute__ ((noinline)) void f1() {
	printf("called f1\n");
}

// tests pass-by-val arg
__attribute__ ((noinline)) void f2(int x) {
	printf("called f2(%d)\n",x);
}

// tests pass-by-ref arg
__attribute__ ((noinline)) void f3(int* x) {
	*x += rand();
}

// tests multiple args
__attribute__ ((noinline)) void f4(int x, int y) {
	printf("called f4(%d,%d)\n",x,y);
}

// tests return val
__attribute__ ((noinline)) int f5() {
	int x = rand();

	if(x % 2 == 0) { return x; }
	else { return 0; }
}
