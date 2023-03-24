package test.java.lang.reflect;

import jvm.TestCase;

import java.lang.reflect.Field;
import java.lang.annotation.Annotation;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FieldTest extends TestCase {
  public static void testGetAnnotation() throws Exception {
    Field f = TaggedClass.class.getField("myField");
    Tag tag = f.getAnnotation(Tag.class);
    testTag(tag);

    Annotation[] allAnnotations = f.getDeclaredAnnotations();
    assertNotNull(allAnnotations);
    tag = (Tag)allAnnotations[0];
    testTag(tag);
  }

  public static void testTag(Tag tag) throws Exception {
    assertNotNull(tag);

    assertEquals(Byte.MAX_VALUE, tag.byteValue());
    assertEquals(Character.MAX_VALUE, tag.charValue());
    assertEquals(Short.MAX_VALUE, tag.shortValue());
    assertEquals(Integer.MAX_VALUE, tag.intValue());
    assertEquals(Long.MAX_VALUE, tag.longValue());
    assertEquals(Float.MAX_VALUE, tag.floatValue());
    assertEquals(Double.MAX_VALUE, tag.doubleValue());
    assertEquals("hello, world", tag.stringValue());
    assertEquals(Required.YES, tag.enumValue());
    assertEquals(Object.class, tag.classValue());
    assertNotNull(tag.annotationValue());

    assertArrayEquals(new byte[]     { Byte.MIN_VALUE,      Byte.MAX_VALUE      }, tag.byteArrayValue());
    assertArrayEquals(new char[]     { Character.MIN_VALUE, Character.MAX_VALUE }, tag.charArrayValue());
    assertArrayEquals(new short[]    { Short.MIN_VALUE,     Short.MAX_VALUE     }, tag.shortArrayValue());
    assertArrayEquals(new int[]      { Integer.MIN_VALUE,   Integer.MAX_VALUE   }, tag.intArrayValue());
    assertArrayEquals(new long[]     { Long.MIN_VALUE,      Long.MAX_VALUE      }, tag.longArrayValue());
    assertArrayEquals(new float[]    { Float.MIN_VALUE,     Float.MAX_VALUE     }, tag.floatArrayValue());
    assertArrayEquals(new double[]   { Double.MIN_VALUE,    Double.MAX_VALUE    }, tag.doubleArrayValue());
    assertArrayEquals(new String[]   { "hello, world",      "Hello, World!"     }, tag.stringArrayValue());
    assertArrayEquals(new Required[] { Required.YES,        Required.NO         }, tag.enumArrayValue());
    assertArrayEquals(new Class<?>[] { Integer.class,       Long.class          }, tag.classArrayValue());
    assertNotNull(tag.annotationArrayValue());
  }

  public static class TaggedClass {
    @Tag(
      byteValue        = Byte.MAX_VALUE,
      charValue        = Character.MAX_VALUE,
      shortValue       = Short.MAX_VALUE,
      intValue         = Integer.MAX_VALUE,
      longValue        = Long.MAX_VALUE,
      floatValue       = Float.MAX_VALUE,
      doubleValue      = Double.MAX_VALUE,
      stringValue      = "hello, world",
      enumValue        = Required.YES,
      classValue       = Object.class,
      annotationValue  = @Tag2,

      byteArrayValue   = { Byte.MIN_VALUE,      Byte.MAX_VALUE      },
      charArrayValue   = { Character.MIN_VALUE, Character.MAX_VALUE },
      shortArrayValue  = { Short.MIN_VALUE,     Short.MAX_VALUE     },
      intArrayValue    = { Integer.MIN_VALUE,   Integer.MAX_VALUE   },
      longArrayValue   = { Long.MIN_VALUE,      Long.MAX_VALUE      },
      floatArrayValue  = { Float.MIN_VALUE,     Float.MAX_VALUE     },
      doubleArrayValue = { Double.MIN_VALUE,    Double.MAX_VALUE    },
      stringArrayValue = { "hello, world",      "Hello, World!"     },
      enumArrayValue   = { Required.YES,        Required.NO         },
      classArrayValue  = { Integer.class,       Long.class          },
      annotationArrayValue = { @Tag2, @Tag2 }
    )
    public String myField = null;
  }

  @Retention(RetentionPolicy.RUNTIME)
  public @interface Tag {
    byte       byteValue();
    char       charValue();
    short      shortValue();
    int        intValue();
    long       longValue();
    float      floatValue();
    double     doubleValue();
    String     stringValue();
    Required   enumValue();
    Class<?>   classValue();
    Tag2       annotationValue();

    byte[]     byteArrayValue();
    char[]     charArrayValue();
    short[]    shortArrayValue();
    int[]      intArrayValue();
    long[]     longArrayValue();
    float[]    floatArrayValue();
    double[]   doubleArrayValue();
    String[]   stringArrayValue();
    Required[] enumArrayValue();
    Class<?>[] classArrayValue();
    Tag2[]     annotationArrayValue();
  }

  public static enum Required { YES, NO }

  public @interface Tag2 { }

  public static class BaseType {
    public Object baseField;
  }

  public static class Type extends BaseType {
    public Object field;
  }

  public static void testSet() throws Exception {
    Type t = new Type();
    Field field = t.getClass().getField("field");
    Object o1 = new Object();
    field.set(t, o1);
    assertEquals(o1, t.field);

    Field baseField = t.getClass().getField("baseField");
    Object o2 = new Object();
    baseField.set(t, o2);
    assertEquals(o2, t.baseField);
  }

  public static void main(String[] args) throws Exception {
    testGetAnnotation();
    testSet();
  }
}
