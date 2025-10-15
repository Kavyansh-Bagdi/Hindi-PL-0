# Building a Hindi PL/0 Compiler in C

Hindi PL/0 will be a **simple programming language** with **static typing** and **Pascal-like syntax**. Since it is designed to be simple, we will implement a **single-pass compiler**, which generates the final code immediately as soon as the compiler has enough information.

> **Single-pass compiler:** A compiler that **reads and processes the source code only once**, performing **lexical analysis, parsing, semantic checks, and code generation** in a single pass, without backtracking.

The compiler will be implemented in **C**.  
**Unicode support:** UTF-8 (to handle Hindi characters properly).

**Basic Syntax**

```bash
program     = block "|" |

block       = [ "नियत" ident "=" number { "," ident "=" number } ";" ]
              [ "चर" ident { "," ident } ";" ]
              { "प्रक्रिया" ident ";" block ";" } statement |

statement   = [ ident ":=" expression
              | "आह्वान" ident
              | "आरम्भ" statement { ";" statement } "समापन"
              | "यदि" condition "तो" statement
              | "जबतक" condition "करो" statement ] |

condition   = "विषम" expression
              | expression ( "=" | "#" | "<" | ">" ) expression |

expression  = [ "+" | "-" ] term { ( "+" | "-" ) term } |
term        = factor { ( "*" | "/" ) factor } |
factor      = ident
              | number
              | "(" expression ")" |
ident       = "अ-ह" { "अ-ह0-9_" } |
number      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
```

**Rough workflow**

```bash
+-------+
| Start |
+-------+
    |                                      No
    +--+                   +------------------------------+
       |                   |                              |
       V                   |                              |
+-------------+            V                        ------------
| Read in     |   +-------------+    +-------+     / Ready to   \
|             |-->| Lex a token |--->| Parse |--->|              |
| source code |   +-------------+    +-------+     \ emit code? /
+-------------+            ^                        ------------
                           |                              |
                           |          +-----------+  Yes  |
                           |          | Emit code |<------+
                           |          +-----------+
                           |No              |
                           |                |
                           |                V
                           |          --------------
                           |         / End of       \  Yes +-----+
                           +--------|                |---->| End |
                                     \ source code? /      +-----+
                                      --------------
```

**Implementation Strategy**
We will follow the **Cowgol compiler strategy**:

- Each major task is implemented as a **separate binary**.
- Intermediate code is written to files, which serve as input for the next binary in the chain.
- Parsing is performed using an **LL(1) recursive-descent parser**.

```bash
+-------+     +--------+     +----------------+
| Lexer |<--->| Parser |---->| Code generator |
+-------+     +--------+     +----------------+
```

**Command**

```bash
# Generate compiler and object files
make

# Generate intermediate C code
make test TEST_MODE=-c

# Generate executable code
make test TEST_MODE=-o
```

**Limitations**

- Variable and function names cannot be written in Hindi.
- Recursion is not used in this project.

**References**

1. [Let's write a compiler by Brian Robert Callahan](https://briancallahan.net/blog/20210814.html)
