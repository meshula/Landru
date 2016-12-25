
gl = require("gl")
time = require("time")
io = require("io")

machine main:
declare:
    gl.window w;

    state main:
        w = gl.createWindow(640, 480, "Hello World")
        on time.after(30):
            io.print("Time's up, it was swell\n")
            goto done;
        on gl.windowClosed(w):
            io.print("Goodbye cruel world\n")
            goto done;
        on gl.windowResized(w):
            param float width;
            param float height;
            io.print("resized to ", width, " height ", height, "\n");
    ;

    state done:
        io.print("Machine finished");
;
