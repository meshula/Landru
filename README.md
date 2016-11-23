
**Landru** is a reactive state machine oriented **Forth** dialect.

**Forth** is executed line by line as source is interpreted. **Landru** is compiled.

Comments are **C++** style.

```
// This is a comment.
```

**Landru** allows the import of modules using the *require* keyword.

```
audio = require("audio")
time  = require("time")
io    = require("io")
```

**Landru** has a number of standard libraries, including
 - io -- input and output
 - time -- time related utilities
 - int -- integer math
 - real -- real number math

There are some optional libraries as well, such as
 - audio

The fundamental execution unit is a *machine*. When machines are
encountered in a Landru file, they are compiled. There is one special
machine, named *main*. It's special because it is executed automatically as
soon as the whole file is compiled.

```
machine main:
    // this machine is trivial
;
```

As in **Forth**, **Landru** language structures are declared, then surrounded
with a colon and semicolon. Here, *machine main* is pretty trivial.

Machines are made up of declared persistent state, and states. There is one
special state, named *main*. When a machine is started, the machine's state
variables are instantiated, and then *main* automatically runs.

```
machine main:
  declare: audio.Context audioContext;

  state main:
  ;
;
```

Whitespace is not significant.

```
// The same program.

machine main:
 declare: audio.Context audioContext;
 state main: ;;
```

**Forth** uses a prefix notation where parameters are loaded up on a stack, then
a function is invoked. **Landru** also has a data stack, loaded up between
parenthesis. Commas between arguments are optional. Functions are always
executed on an object. There are no free functions.

```
  int.add(3 5)
  int.add(3, 5)
```

A *machine* can launch another *machine*.

```
machine pong:
  state main: io.print("pong") ;;

machine ping:
  state main: io.print("ping")
              launch("pong")   ;;
```

A *machine* can launch another instance of its own kind.

```
machine recurse:
   state main: launch("recurse") ;;
```

The *recurse* machine does not lock up the execution environment,
because *lauch* instructions do not execute immediately. Instead, they are
queued, and run when a state yields.

A *machine* retires when it has no pending state transitions. **Landru**
execution continues until all launched machines retire, or are otherwise
terminated.

A real *machine* will have more than one state. State transitions are invoked
by the *goto* statement.

```
machine pingpong:
  state ping: io.print("ping") goto pong ;
  state pong: io.print("pong") goto ping ;
  state main: goto ping ;;
```

Notice that the statements executed by the state are bracketed by **Forth*'s
colon-semicolon scoping syntax.

The *pingpong machine* will print out ping and pong like crazy. (Not completely
crazy, gotos yield before they take affect.) Actor based
languages are event driven, and **Landru** is no exception. **Landru** uses
the *on* keyword to indicate events that will be responded to.

The *time* standard library has an after event. The *pingpong machine* can be
brought under control by involving a timer.

```
machine pingpong:
  state ping: io.print("ping")
              on time.after(0.5): goto pong ;;
  state pong: io.print("pong")
              on time.after(0.5): goto ping ;;
  state main: goto ping ;;
```

This *pingpong machine* will change state every half a second. Notice that
the *on* statement takes the form of *on condition* followed by the things to do
enclosed in **Forth**'s typical colon semicolon scoping syntax.

variables
---------

Variables have a hierarchy of scopes.

Local variables are declared within a scope, and don't persist beyond it.

```
machine localVar:
  state main:
    float a = 3
    io.print("a is ", a);;
```

The variable *a* won't be avalable outside of the state, and will be initialized
to 3 every time the state is initialized.

```
machine instanceVar:
  declare: float b = 3;
  state main:
    io.print("b is ", b);;
```

When a machine of type *instanceVar* is launched, it will have its own copy of *b*,
initialized to 3. If a state modifies b, the new value is seen by any other states
within that instatiated machine, but not by other machines of the same time.

```
machine sharedInstanceVar:
  declare: shared float c = 3;
  state main:
    io.print("c is ", c) ;;
```

When the first machine of type *sharedInstanceVar* is instantiated, *c* will be
have the value of 3. If *c* is modified, all other instances of *sharedInstanceVar*
will see that new value. When new instances are created, they will share the
already existing *c*. If all copies of *sharedInstanceVar* exit, *c* will also
disappear, meaning the next *sharedINstanceVar* object will reinitialize *c* with
the value 3.

```
require("test")
machine requireVar:
  state main:
    io.print("test.d is ", d) ;;
```

If the module test is known to have a value *d* in it, a machine can access it
using dotted scope syntax.

declare:
  float e = 3 ;

machine globalVar:
  state main:
    io.print("global e is ", e) ;;
```

Variables declared in the global scope persist until all launched machines have
exited and the script stops running.

Scoping rules are that the most local scope has precedence. So a local variable
is preferred to machine variable (shared or instance), is preferred to a required
module's variable or a global variable. If for some reason namespacing caused a
global variable to have the same name as a module's variable, the module
variable hides the global variable.

