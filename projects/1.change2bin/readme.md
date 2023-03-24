# Change 2 Bin

- It is always better to use visual studio for developing projects initially because we get intellisense and can easily look at various definition of the functions.
- I learned, COM interfaces and their functions need to be accessed differently between C vs C++ projects. For example in using DIA SDK,
  - In C++ we use,
      ```C++
          DiaSymbol->findChildren(...)
      ```
  - In C we have to use,
      ```C
          IDiaSymbol_findChildren(DiaSymbol, ...);
      ```
  - Also, The above definition is available only when we declare below macros
      ```C
          #define COBJMACROS
          #define CINTERFACE
      ```
  - I found this from https://github.com/processhacker/processhacker/blob/master/phnt/include/phnt_windows.h
