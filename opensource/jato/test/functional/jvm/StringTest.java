package jvm;

/**
 * @author Vegard Nossum
 */
public class StringTest extends TestCase {
    public static void testUnicode() {
        String s = "æøå";
        assertEquals(s.length(), 3);

        String t = "ヒラガナ";
        assertEquals(t.length(), 4);

        String p = "\u1234\u5678\uabcd";
        assertEquals(0x1234, (int)p.charAt(0));
        assertEquals(0x5678, (int)p.charAt(1));
        assertEquals(0xabcd, (int)p.charAt(2));
    }

    public static void testStringConcatenation() {
        String a = "123";
        String b = "abcd";

        assertEquals("123abcd", a + b);
    }

    public static String test_literal = "Test";

    public static void testStringIntern() {
        String s1 = new String("Test");
        assertEquals(s1.intern(), test_literal);
    }

    public static void main(String args[]) {
        testUnicode();
        testStringConcatenation();
        testStringIntern();
    }
}
