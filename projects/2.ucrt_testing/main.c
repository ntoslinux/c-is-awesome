
//#define ASSERT_H_TEST

#ifdef ASSERT_H_TEST
#define NDEBUG
#include <assert.h>
void
assert_test()
{
    int x = 10;
    assert(x == 10);
}

void
static_assert_test()
{
    static_assert(1==0, "static assert failed");
}
#endif

//#define  ERROR_H_TEST

#ifdef ERROR_H_TEST
#include <errno.h>

void
errno_test()
{
    if (errno) {
        printf("asdfasdfaSDF");
    }
}
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SIZE 1024
int main()
{

    FILE *fp = NULL;
    size_t n;
    char *buf;
    clock_t start, end;


    buf = malloc(sizeof(char) * SIZE);
    start = clock();
    fp = fopen("C:\\repos\\EWDK_19h1_release_svc_prod3_18362_190416-1111.iso", "rb");
    if (fp == NULL) {
        return -1;
    }
    while ((n = fread(buf, sizeof(char), SIZE, fp)) > 0);

    end = clock();

    printf("\ntime=%lf", ((double) (end - start)) / CLOCKS_PER_SEC);

    fclose(fp);
    free(buf);
    return 0;
}
