
io    = require("io")

machine main:
    declare:
        int a
        shared int a0 = 3
        int b = 1
        float c
        float d = 2
        float e = 2.0
        string f = "test"
        shared string g = "hello"
    ;

    state main:
        io.print("a = ", a, " b = ", b, " c = ", c, "\n")
        io.print("d = ", d, " e = ", e, " f = ", f, "\n")
        io.print("Shared a0 = ", a0, " shared g = ", g, "\n")
    ;
;
