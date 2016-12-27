
audio = require("audio")
time = require("time")
io = require("io")
gl = require("gl")

machine main:
declare:
    audio.context ac;

    state main:
        io.print("sample rate ", audio.context.sampleRate(ac), "\n")
        on time.after(5):
            io.print("Time's up, it was swell\n")
            goto done;
    ;

    state done:
        io.print("Machine finished\n");
;
