void api_putchar(int c);
void api_end(void);

void HariMain(void)
{
    char a[100];
    a[10] = 'Y';
    api_putchar(a[10]);
    a[102] = 'U';
    api_putchar(a[102]);
    a[120] = 'A';
    api_putchar(a[120]);
    api_end();
}
