
time  = require("time")
io    = require("io")

machine main:
  state ping: io.print("ping! ")
              on time.after(0.5): goto pong ;;
  state pong: io.print("pong! ")
              on time.after(0.5): goto ping ;;
  state main: goto ping ;;
