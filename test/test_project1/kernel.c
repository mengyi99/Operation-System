void(*printstr)(char *str);

void __attribute__((section(".entry_function"))) _start(void)
{
	char str[]="Hello OS!";
	printstr=(void *)0xffffffff8f0d5534;
	(*printstr)(str);
	return;

}
