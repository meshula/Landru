
audio = require("audio")
time = require("time")
io = require("io")
gl = require("gl")

declare:
    audio.buffer snare
    float sr
;

machine main:
    declare:
        audio.context ac
    ;

    state main:
        sr = audio.context.sampleRate(ac)
        io.print("sample rate ", sr, "\n")
//        snare = audio.buffer.load(ac, "snare.wav", sr)
//        snare.play()
        on time.after(5):
            io.print("Time's up, it was swell\n")
            goto done;
    ;

    state done:
        io.print("Machine finished\n");
;
