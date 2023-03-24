/*
 * Copyright (C) 2011 Joonas Reynders
 *
 * This file is released under the GPL version 2 with the following
 * clarification and special exception:
 *
 *   Linking this library statically or dynamically with other modules is
 *   making a combined work based on this library. Thus, the terms and
 *   conditions of the GNU General Public License cover the whole
 *   combination.
 *
 *   As a special exception, the copyright holders of this library give you
 *   permission to link this library with independent modules to produce an
 *   executable, regardless of the license terms of these independent
 *   modules, and to copy and distribute the resulting executable under terms
 *   of your choice, provided that you also meet, for each linked independent
 *   module, the terms and conditions of the license of that module. An
 *   independent module is a module which is not derived from or based on
 *   this library. If you modify this library, you may extend this exception
 *   to your version of the library, but you are not obligated to do so. If
 *   you do not wish to do so, delete this exception statement from your
 *   version.
 *
 * Please refer to the file LICENSE for details.
 */
package test.java.lang;

import jvm.TestCase;
import java.lang.reflect.Method;
import java.lang.reflect.Field;

/**
* @author Joonas Reynders
*/
public class JNITest extends TestCase {
  static {
    System.load("./test/functional/jni/libjnitest.so");
  }

  native static public String staticReturnPassedString(String str);
  native static public int staticReturnPassedInt(int i);
  native static public long staticReturnPassedLong(long l);
  native static public boolean staticReturnPassedBoolean(boolean b);
  native static public short staticReturnPassedShort(short s);
  native static public byte staticReturnPassedByte(byte b);
  native static public char staticReturnPassedChar(char c);
  native static public float staticReturnPassedFloat(float f);
  native static public double staticReturnPassedDouble(double d);

  native static public String[] staticReturnPassedStringArray(String[] str);
  native static public int[] staticReturnPassedIntArray(int[] i);
  native static public long[] staticReturnPassedLongArray(long[] l);
  native static public boolean[] staticReturnPassedBooleanArray(boolean[] b);
  native static public short[] staticReturnPassedShortArray(short[] s);
  native static public byte[] staticReturnPassedByteArray(byte[] b);
  native static public char[] staticReturnPassedCharArray(char[] c);
  native static public float[] staticReturnPassedFloatArray(float[] f);
  native static public double[] staticReturnPassedDoubleArray(double[] d);

  native public String returnPassedString(String str);
  native public int returnPassedInt(int i);
  native public long returnPassedLong(long l);
  native public boolean returnPassedBoolean(boolean b);
  native public short returnPassedShort(short s);
  native public byte returnPassedByte(byte b);
  native public char returnPassedChar(char c);
  native public float returnPassedFloat(float f);
  native public double returnPassedDouble(double d);

  native public String[] returnPassedStringArray(String[] str);
  native public int[] returnPassedIntArray(int[] i);
  native public long[] returnPassedLongArray(long[] l);
  native public boolean[] returnPassedBooleanArray(boolean[] b);
  native public short[] returnPassedShortArray(short[] s);
  native public byte[] returnPassedByteArray(byte[] b);
  native public char[] returnPassedCharArray(char[] c);
  native public float[] returnPassedFloatArray(float[] f);
  native public double[] returnPassedDoubleArray(double[] d);

  native static public String staticToUpper(String str);

  native static public boolean staticTestJNIEnvIsInitializedCorrectly();

  // JNI 1.6 native test functions
  native static public int getVersion();
  native static public Class<Object> defineClass(String name, ClassLoader classloader);
  native static public boolean testDefineClassThrowsExceptionWithBrokenClass();
  native static public Class<Object> findClass(String name);
  native static public java.lang.reflect.Method passThroughFromAndToReflectedMethod(java.lang.reflect.Method plusOneMethod);
  native static public java.lang.reflect.Field passThroughFromAndToReflectedField(java.lang.reflect.Field intField);
  native static public Object getSuperclass(Class<?> clazz);
  native static public boolean isAssignableFrom(Class<?> clazz1, Class<?> clazz2);
  native static public int jniThrow(Throwable throwable);
  native static public int jniThrowNew(Class<?> clazz, String message);
  native static public boolean testJniExceptionOccurredAndExceptionClear(Throwable throwable);
  native static public boolean testIsSameObject(Object obj, Object sameObj, Object differentObj);
  native static public boolean testAllocObject(Class<?> clazz);
  native static public Object testNewObject(Class<?> clazz, String constructorSignature, Object args);
  native static public Object testNewObjectA(Class<?> clazz, String constructorSignature, Object args);
  native static public Object testNewObjectV(Class<?> clazz, String constructorSignature, Object args);
  native static public Class<?> testGetObjectClass(Object obj);
  native static public boolean isInstanceOf(Object obj, Class<?> clazz);
  native static public boolean testMethodID(Class<?> clazz, String methodName, String signature);

  private static JNITest jniTest = new JNITest();

  public static void testReturnPassedString() {
    assertEquals("testString", staticReturnPassedString("testString"));
    assertEquals("testString", jniTest.returnPassedString("testString"));
    assertEquals("testString", staticReturnPassedStringArray(new String[]{"testString"})[0]);
    assertEquals("testString", jniTest.returnPassedStringArray(new String[]{"testString"})[0]);
  }

  public static void testReturnPassedInt() {
    assertEquals(42, staticReturnPassedInt(42));
    assertEquals(42, jniTest.returnPassedInt(42));
    assertEquals(42, staticReturnPassedIntArray(new int[]{42})[0]);
    assertEquals(42, jniTest.returnPassedIntArray(new int[]{42})[0]);
  }

  public static void testReturnPassedLong() {
    assertEquals(42l, staticReturnPassedLong(42l));
    assertEquals(0xdeadbeefcafebabeL, staticReturnPassedLong(0xdeadbeefcafebabeL));
    assertEquals(42l, jniTest.returnPassedLong(42l));
    assertEquals(0xdeadbeefcafebabeL, jniTest.returnPassedLong(0xdeadbeefcafebabeL));
    assertEquals(42l, staticReturnPassedLongArray(new long[]{42l})[0]);
    assertEquals(42l, jniTest.returnPassedLongArray(new long[]{42l})[0]);

    // java.lang.Long.MAX_VALUE
    assertEquals(9223372036854775807l, staticReturnPassedLong(9223372036854775807l));
    assertEquals(9223372036854775807l, jniTest.returnPassedLong(9223372036854775807l));
    
    // java.lang.Long.MIN_VALUE
    assertEquals(-9223372036854775808l, staticReturnPassedLong(-9223372036854775808l));
    assertEquals(-9223372036854775808l, jniTest.returnPassedLong(-9223372036854775808l));
  }

  public static void testReturnPassedBoolean() {
    assertEquals(true, staticReturnPassedBoolean(true));
    assertEquals(true, jniTest.returnPassedBoolean(true));
    assertEquals(true, staticReturnPassedBooleanArray(new boolean[]{true})[0]);
    assertEquals(true, jniTest.returnPassedBooleanArray(new boolean[]{true})[0]);
  }

  public static void testReturnPassedShort() {
    short s = 42;
    assertEquals(42, staticReturnPassedShort(s));
    assertEquals(42, jniTest.returnPassedShort(s));
    assertEquals(42, staticReturnPassedShortArray(new short[]{s})[0]);
    assertEquals(42, jniTest.returnPassedShortArray(new short[]{s})[0]);
  }

  public static void testReturnPassedByte() {
    byte b = 42;
    assertEquals(42, staticReturnPassedByte(b));
    assertEquals(42, jniTest.returnPassedByte(b));
    assertEquals(42, staticReturnPassedByteArray(new byte[]{b})[0]);
    assertEquals(42, jniTest.returnPassedByteArray(new byte[]{b})[0]);
  }

  public static void testReturnPassedChar() {
    assertEquals('a', staticReturnPassedChar('a'));
    assertEquals('a', jniTest.returnPassedChar('a'));
    assertEquals('a', staticReturnPassedCharArray(new char[]{'a'})[0]);
    assertEquals('a', jniTest.returnPassedCharArray(new char[]{'a'})[0]);
  }

  public static void testReturnPassedFloat() {
    assertEquals(42.0f, staticReturnPassedFloat(42.0f));
    assertEquals(42.0f, jniTest.returnPassedFloat(42.0f));
    assertEquals(42.0f, staticReturnPassedFloatArray(new float[]{42.0f})[0]);
    assertEquals(42.0f, jniTest.returnPassedFloatArray(new float[]{42.0f})[0]);
  }

  public static void testReturnPassedDouble() {
    assertEquals(42.0, staticReturnPassedDouble(42.0));
    assertEquals(42.0, jniTest.returnPassedDouble(42.0));
    assertEquals(42.0, staticReturnPassedDoubleArray(new double[]{42.0})[0]);
    assertEquals(42.0, jniTest.returnPassedDoubleArray(new double[]{42.0})[0]);
  }

  public static void testStringManipulation() {
    assertEquals("TESTSTRING", staticToUpper("testString"));
  }

  public static void testJNIEnvIsInitializedCorrectly() {
    assertTrue(staticTestJNIEnvIsInitializedCorrectly());
  }

  // JNI 1.6 API function tests
  public static void testGetVersion() {
    assertEquals(0x00010006, getVersion());
  }

  public static void testDefineClass() {
    Class<Object> jniTestFixtureClass = defineClass("test/functional/jni/JNITestFixture.class", ClassLoader.getSystemClassLoader());
    assertNotNull(jniTestFixtureClass);
    assertEquals("class jni.JNITestFixture", jniTestFixtureClass.toString());

    try {
      Object jniTestFixtureInstance = jniTestFixtureClass.newInstance();
      assertEquals("jni.JNITestFixture@", jniTestFixtureInstance.toString().substring(0, "jni.JNITestFixture@".length()));
      assertEquals("TESTSTRING", jniTestFixtureInstance.getClass().getMethod("toUpper", new Class[]{String.class}).invoke(null, "testString"));
    } catch (Exception e) {
      throw new RuntimeException(e);
    }

    assertThrows(new Block(){
      public void run() throws Throwable {
        testDefineClassThrowsExceptionWithBrokenClass();
      }
    }, java.lang.ClassFormatError.class);
  }

  public static void testFindClass() {
    Class<Object> foundStringClass = findClass("java/lang/String");
    assertNotNull(foundStringClass);
    assertTrue(foundStringClass.isInstance(new String()));

    Class<Object> foundObjectArrayClass = findClass("[Ljava/lang/Object;");
    assertNotNull(foundObjectArrayClass);
    assertTrue(foundObjectArrayClass.isInstance(new Object[0]));

    assertThrows(new Block() {
      public void run() throws Throwable {
        findClass("does/not/Exist");
      }
    }, java.lang.NoClassDefFoundError.class);
  }

  static int plusOne(int i) {
    return i + 1;
  }

  public static void testFromAndToReflectedMethod() {
    Method plusOneMethod = null;

    try {
      plusOneMethod = JNITest.class.getDeclaredMethod("plusOne", int.class);
      assertEquals(1, plusOneMethod.invoke(null, 0));
      Method jniCycledPlusOneMethod = (Method) passThroughFromAndToReflectedMethod(plusOneMethod);
      assertEquals(plusOneMethod, jniCycledPlusOneMethod);
      assertEquals(1, jniCycledPlusOneMethod.invoke(null, 0));
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  static int intValue = 1;

  public static void testFromAndToReflectedField() {
    Field intField = null;

    try {
      intField = JNITest.class.getDeclaredField("intValue");
      assertEquals(1, intField.getInt(jniTest));
      Field jniCycledIntField = (Field) passThroughFromAndToReflectedField(intField);
      assertEquals(intField, jniCycledIntField);
      assertEquals(1, jniCycledIntField.getInt(jniTest));
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  public static void testGetSuperclass() {
    assertNull(getSuperclass(java.lang.Object.class));
    assertEquals(jvm.TestCase.class, getSuperclass(JNITest.class));
    assertNull(getSuperclass(java.lang.Runnable.class));
  }

  public static void testIsAssignableFrom() {
    assertTrue(isAssignableFrom(jniTest.getClass(), jniTest.getClass()));
    assertTrue(isAssignableFrom(jniTest.getClass(), jniTest.getClass().getSuperclass()));
    assertFalse(isAssignableFrom(jniTest.getClass().getSuperclass(), jniTest.getClass()));
  }

  public static void testThrow() {
    assertThrows(new Block() {
      public void run() throws Throwable {
        jniThrow(new NullPointerException("Test"));
      }
    }, NullPointerException.class);
  }

  public static void testThrowNew() {
    assertThrows(new Block() {
      public void run() throws Throwable {
        jniThrowNew(NullPointerException.class, "Test");
      }
    }, NullPointerException.class);

    try {
      jniThrowNew(RuntimeException.class, "Test exception");
    } catch (RuntimeException e) {
      assertEquals("Test exception", e.getMessage());
    }
  }

  public static void testExceptionOccurredAndExceptionClear() {
    assertTrue(testJniExceptionOccurredAndExceptionClear(new NullPointerException()));
  }

  public static void testIsSameObject() {
    String str = "same";
    String sameStr = str;
    String differentStr = "different";

    assertTrue(testIsSameObject(str, sameStr, differentStr));
  }

  public static void testAllocObject() {
    assertTrue(testAllocObject(Object.class));
    assertThrows(new Block() {
      public void run() throws Throwable {
        testAllocObject(Runnable.class);
      }
    }, InstantiationException.class);

    assertThrows(new Block() {
      public void run() throws Throwable {
        testAllocObject(ClassLoader.class);
      }
    }, InstantiationException.class);
  }

  public static void testNewObject() {
    assertEquals("test", testNewObject(String.class, "(Ljava/lang/String;)V", "test"));
    assertEquals("test", testNewObject(String.class, "([C)V", "test".toCharArray()));

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObject(Runnable.class, "()V", null);
      }
    }, InstantiationException.class);

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObject(ClassLoader.class, "()V", null);
      }
    }, InstantiationException.class);
  }

  public static void testNewObjectA() {
    assertEquals("test", testNewObjectA(String.class, "(Ljava/lang/String;)V", "test"));
    assertEquals("test", testNewObjectA(String.class,  "([C)V", "test".toCharArray()));

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObjectA(Runnable.class, "()V", null);
      }
    }, InstantiationException.class);

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObjectA(ClassLoader.class, "()V", null);
      }
    }, InstantiationException.class);
  }

  public static void testNewObjectV() {
    assertEquals("test", testNewObjectV(String.class, "(Ljava/lang/String;)V", "test"));
    assertEquals("test", testNewObjectV(String.class,  "([C)V", "test".toCharArray()));

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObjectV(Runnable.class, "()V", null);
      }
    }, InstantiationException.class);

    assertThrows(new Block() {
      public void run() throws Throwable {
        testNewObjectV(ClassLoader.class, "()V", null);
      }
    }, InstantiationException.class);
  }

  public static void testGetObjectClass() {
    assertEquals(String.class, testGetObjectClass(""));
  }

  public static void testIsInstanceOf() {
    assertTrue(isInstanceOf(jniTest, JNITest.class));
  }

  public static void testMethodID() {

    assertTrue(testMethodID(JNITest.class, "testMethodID", "()V"));

    assertThrows(new Block() {
      public void run() throws Throwable {
        testMethodID(JNITest.class, "testMethodID", "(Ljava/lang/String;)V");
      }
    }, NoSuchMethodError.class);

    assertThrows(new Block() {
      public void run() throws Throwable {
        testMethodID(JNITest.class, "nosuchmethod", "()V");
      }
    }, NoSuchMethodError.class);
  }

  public static void main(String[] args) {
    testReturnPassedString();
    testReturnPassedInt();
    testReturnPassedLong();
    testReturnPassedBoolean();
    testReturnPassedShort();
    testReturnPassedByte();
    testReturnPassedChar();
    testReturnPassedFloat();
    testReturnPassedDouble();
    testStringManipulation();
    testJNIEnvIsInitializedCorrectly();
    testGetVersion();
    testDefineClass();
    testFindClass();
    testFromAndToReflectedMethod();
    testFromAndToReflectedField();
    testGetSuperclass();
    testIsAssignableFrom();
    testThrow();
    testThrowNew();
    testExceptionOccurredAndExceptionClear();
    testIsSameObject();
    testAllocObject();
    testNewObject();
    testNewObjectA();
    testNewObjectV();
    testGetObjectClass();
    testIsInstanceOf();
    testMethodID();
  }
}
