.class public jvm/DupTest
.super jvm/TestCase

.field public static counter I

.method public static advanceCounter()I
    .limit stack 128

    getstatic jvm/DupTest/counter I
    iconst_1
    iadd
    putstatic jvm/DupTest/counter I

    ; we don't want to use 'dup' here because we shouldn't
    ; assume that dup works.
    getstatic jvm/DupTest/counter I
    ireturn
.end method

; Test 'dup' instruction
.method public static testDup()V
    .limit stack 128
    .limit locals 3

    iconst_0
    putstatic jvm/DupTest/counter I

    invokestatic jvm/DupTest/advanceCounter()I
    dup

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_3
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup_x1' instruction
.method public static testDup_x1()V
    .limit stack 128
    .limit locals 4

    iconst_0
    putstatic jvm/DupTest/counter I

    iconst_1
    invokestatic jvm/DupTest/advanceCounter()I
    dup_x1

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iconst_1
    iadd
    istore_3

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_3
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_3
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup_x2' instruction
.method public static testDup_x2()V
    .limit stack 128
    .limit locals 5

    iconst_0
    putstatic jvm/DupTest/counter I

    iconst_3
    iconst_2
    invokestatic jvm/DupTest/advanceCounter()I
    dup_x2

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iconst_3
    iadd
    istore_3

    iconst_4
    iadd
    istore 4

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_4
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_3
    bipush 6
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 4
    iconst_5
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup_x2' instruction when value2 has computational type of category 2
.method public static testDup_x2_long()V
    .limit stack 128
    .limit locals 5

    iconst_0
    putstatic jvm/DupTest/counter I

    dconst_0
    invokestatic jvm/DupTest/advanceCounter()I
    dup_x2

    iconst_1
    iadd
    istore_1

    dconst_1
    dadd
    dstore_2

    iconst_3
    iadd
    istore 4

    iconst_2
    iload_1
    invokestatic jvm/TestCase/assertEquals(II)V

    dconst_1
    dload_2
    invokestatic jvm/TestCase/assertEquals(DD)V

    bipush 4
    iload 4
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup2' instruction
.method public static testDup2()V
    .limit stack 128
    .limit locals 5

    iconst_0
    putstatic jvm/DupTest/counter I

    iconst_2
    invokestatic jvm/DupTest/advanceCounter()I
    dup2

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iconst_3
    iadd
    istore_3

    iconst_4
    iadd
    istore 4

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_4
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_3
    iconst_4
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 4
    bipush 6
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup2_x1' instruction
.method public static testDup2_x1()V
    .limit stack 128
    .limit locals 6

    iconst_0
    putstatic jvm/DupTest/counter I

    iconst_3
    iconst_2
    invokestatic jvm/DupTest/advanceCounter()I
    dup2_x1

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iconst_3
    iadd
    istore_3

    iconst_4
    iadd
    istore 4

    iconst_5
    iadd
    istore 5

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_4
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_3
    bipush 6
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 4
    bipush 5
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 5
    bipush 7
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

; Test 'dup2_x2' instruction
.method public static testDup2_x2()V
    .limit stack 128
    .limit locals 7

    iconst_0
    putstatic jvm/DupTest/counter I

    iconst_4
    iconst_3
    iconst_2
    invokestatic jvm/DupTest/advanceCounter()I
    dup2_x2

    iconst_1
    iadd
    istore_1

    iconst_2
    iadd
    istore_2

    iconst_3
    iadd
    istore_3

    iconst_4
    iadd
    istore 4

    iconst_5
    iadd
    istore 5

    bipush 6
    iadd
    istore 6

    iload_1
    iconst_2
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_2
    iconst_4
    invokestatic jvm/TestCase/assertEquals(II)V

    iload_3
    bipush 6
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 4
    bipush 8
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 5
    bipush 6
    invokestatic jvm/TestCase/assertEquals(II)V

    iload 6
    bipush 8
    invokestatic jvm/TestCase/assertEquals(II)V

    return
.end method

.method public static main([Ljava/lang/String;)V
    invokestatic jvm/DupTest/testDup()V
    invokestatic jvm/DupTest/testDup_x1()V
    invokestatic jvm/DupTest/testDup_x2()V
    invokestatic jvm/DupTest/testDup_x2_long()V
    invokestatic jvm/DupTest/testDup2()V
    invokestatic jvm/DupTest/testDup2_x1()V
    invokestatic jvm/DupTest/testDup2_x2()V
    return
.end method
