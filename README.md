# Lisp_in_c
Welcome to the Lisperer programming language!

### Contents
* [Introduction](#introduction)<br/>
* [Installation](#installation)<br/>
* [Getting Started](#gettingStarted)<br/>
* [Arithmetic](#arithmetic)<br/>
* [Variables](#variables)<br/>
* [Conditions](#conditions)<br/>
* [Functions](#functions)<br/>
* [Lambda Expressions](#lambdaExpressions)<br/>
* [Lists](#lists)<br/>
* [Recursion](#recursion)<br/>
* [Final Points](#finalPoints)<br/>

<a name="introduction"/>

### Introduction

Lisperer is a simple programming language which I created as personal project. It is a variant of a Lisp programming language. The motivation for the project is simple. I used it as a means to gain a strong foundation in C, and to explore how programming languages work behind the scenes. While the language I have created can be used to do some cool things, it was never intended to be a fully-fledged, production-level language. 

This project is based off a book called [Build Your Own Lisp](http://www.buildyourownlisp.com/). I cannot recommend it highly enough for someone learning C for the first time.
The mpc.c and mpc.h files were created by Daniel Holden (orangeduck), the author of [Build Your Own Lisp](http://www.buildyourownlisp.com/). All credit goes to him.

<a name="installation"/>

### Installation

To install Lisperer, you will need a C compiler. Clone/download this repository, and compile with the following command:

Linux/Mac:

``
cc -std=c99 -Wall lisperer.c mpc.c -ledit -o lisperer
``

Windows:

``
cc -std=c99 -Wall lisperer.c mpc.c -o lisperer
``


<a name="gettingStarted"/>

### Getting Started

Like Python, Lisperer is an interpreted language. It can be run as an interactive prompt on the command line, or it can execute pre-written scripts. 

Interactive Prompt:

To start Lisperer as an interactive prompt, run the lisperer executable in your terminal with no arguments. You will see the following:

```
Lisperer Version 0.0.0.1
exit() to quit

lisperer>
```

After the ‘lisperer>’ prompt, you can start typing your Lisperer commands. To leave the interactive prompt, type ‘exit()’ and press enter.

Execute scripts:

To execute a lisperer script, run the lisperer executable in your terminal with your script as an argument. The name of your script should be surrounded in double quotes:

``
C:\example>lisperer “myscript.lsp”
``

<a name="arithmetic"/>

### Arithmetic

Lisperer allows four types of basic arithmetic:
* addition (+)
* subtraction (-)
* multiplication (*)
* division (/)

You can perform one of these operations following pattern:

``
<arithmetic operator> <first number> <second number> ….. <nth number>
``

For example:

```
lisperer> + 2 4 6
12
lisperer> - 10 2
8
lisperer> * 4 5 2 2
80
lisperer> / 100 2
50
```

<a name="variables"/>

### Variables

You can define variables with the following syntax:

``
def {<variableName>} <variableValue>
``

For example:

```
lisperer>def {decade} 10
()
```

You can then retrieve the value by referencing the variable:

```
lisperer>+ decade 8
18
```

Lisperer supports:
* integers – eg. 4
* strings – eg. “hello world”
* lists – eg. {“Jack” “Jill” “Bob”}

Booleans are expressed as integers:
* 0 represents false
* 1 represents true

<a name="conditions"/>

### Conditions

Lisperer has if statements to handle allow you to run code based on a condition:

``
if (<condition>) {<codeToRunIfTrue>} {<codeToRunIfFalse>}
``

In the following example, the if statement returns the age of the older person:

```
lisperer>def {johnsAge} 20
()
lisperer>def {marysAge} 25
()
lisperer>if {> johnsAge marysAge) {johnsAge} {marysAge}
25
```

<a name="functions"/>

### Functions

Functions can be defined like this:

``
fun {<functionName> <argument1> … <argumentN>} {<functionBody>}
``

For example:

```
lisperer>fun {add-numbers x y} {+ x y}
()
lisperer>add-numbers 10 23
33
```

<a name="lambdaExpressions"/>

### Lambda Expressions

Lisperer also supports Lambda Expressions, with the back-slash character:

``
(\ {<argument1> … <argumentN>} {<functionBody>})
``

The previous example can be achieved with a Lambda Expression:

```
lisperer>(\ {x y} {+ x y}) 10 23
33
```

<a name="lists"/>

### Lists

Lists can be defined using curly braces:

```
lisperer>def {myList} {“John” “Mary” “Lisa” “Martin”}
```

Lists can have any combinations of data types in them, such as numbers, strings, more lists and can even include code. For example, the following creates a perfectly valid list:

```
lisperer>def {myList} {“John” 23 {3 5 3} (+ 65 3)}
```

There are a number of built-in functions for working with lists:
* head – returns the first element in the list
* tail – returns every element except the first in a list
* join – joins two lists together
* list – creates a list from arguments passed to it
* eval - evaluates a list as if it was an expression

Note that none of these functions alters the original list.

For example:

```
lisperer>def {myList} {“Mary” 23 (+ 3 3)}
()
lisperer>def {otherList} {+ 20 6 (- 10 5)}
()
lisperer>head myList
{“Mary”}
lisperer>tail myList
{23 (+ 3 3)}
lisperer>join myList otherList
{“Mary” 23 (+ 3 3) + 20 6 (- 10 5)}
lisperer>list “here” “there” “everywhere”
{“here” “there” “everywhere”}
lisperer>eval otherList
31
```

<a name="recursion"/>

### Recursion

There are no loops in Lisperer (I never got around to adding them). However, recursion is supported. Recursive functions can be used in place of looping structures. For example, this function returns the length of a list:

```
(fun {listLength myList} {
  if (== myList {})
    {0}
    {+ 1 (listLength (tail myList))}
})
```

<a name="finalPoints"/>

### Final Points

Again, this language is not meant to be taken too seriously. A lot of work would be needed to turn it into something usable in a real project. However, from my perspective, it was a really fun, worthwhile project. If you are looking to learn C, and already have some experience in another language, I’d definitely recommend [Build Your Own Lisp](http://www.buildyourownlisp.com/).
