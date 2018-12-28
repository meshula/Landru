
audio = require("audio")
time = require("time")
io = require("io")

declare:
    audio.buffer snare
    float sr
;

machine main:
    declare:
        audio.context ac
    ;

    state main:
        sr = ac.sampleRate()
        io.print("sample rate ", sr, "\n")
        snare = audio.buffer.load(ac, io.resolve("snare.wav"), sr)
        snare.play()
        on time.after(5):
            io.print("Time's up, it was swell\n")
            goto done;
    ;

    state done:
        io.print("Machine finished\n");
;
