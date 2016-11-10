
time  = require("time")
io    = require("io")

machine main:
  state main: io.print("ping ")
              on time.after(0.5): launch("pong") ;;;

machine pong:
  state main: io.print("pong ") ;;
