
gl = require("gl")
time = require("time")

machine main:
    state main:
        gl.createWindow(640, 480, "Hello World")
        on time.after(5): goto done ;
    ;

    state done:;
;

