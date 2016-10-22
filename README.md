
**Landru** is an actor oriented **Forth** dialect.

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

The fundamental execution unit is a *machine*. When machines are
encountered in a Landru file, they are compiled. There is one special
machine, named *main*.

```
machine main:
    // this machine is trivial
;
```

As in **Forth**, **Landru** language structures are declared, then surrounded
with a colon and semicolon. Here, *machine main* is pretty trivial.

Machines are made up of declared persistent state, and states. There is one
special state, named *main*. When a machine is started, declared state is
instantiated, and then *main* automatically runs.

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

Now, the *pingpong machine* will change state every half a second. Notice that
the *on* statement takes the form of *on condition* followed by the things to do
enclosed in **Forth**'s typical colon semicolon scoping syntax.

