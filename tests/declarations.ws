
io    = require("io")

declare:
    string h = "global-h-string"
    int i = 9
;

machine main:
    declare:
        int a
        shared int a0 = 1
        int b = 2
        float c
        float d = 4
        float e = 5.0
        string f = "f-string"
        shared string g = "shared-g-string"
    ;

    state main:
        io.print("a = ", a, " b = ", b, " c = ", c, "\n")
        io.print("d = ", d, " e = ", e, " f = ", f, "\n")
        io.print("Shared a0 = ", a0, " shared g = ", g, "\n")
        io.print("Global string h = ", h, "\n")
        io.print("Global int i = ", i, "\n")
    ;
;
