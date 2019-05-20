
#include <thread>
#include <chrono>

const char* test_declarations_ws = R"landru(

io = require("io")

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
        declare:
            int j = 10
        ;
        io.print("a = ", a, " b = ", b, " c = ", c, "\n")
        io.print("d = ", d, " e = ", e, " f = ", f, "\n")
        io.print("Shared a0 = ", a0, " shared g = ", g, "\n")
        io.print("Global string h = ", h, "\n")
        io.print("Global int i = ", i, "\n")
        io.print("Local j = ", j, "\n")
    ;
;

)landru";

void test_declarations()
{
#if 0
	// runtime environment
    LandruLibrary_t* library = landruCreateLibrary("landru");
    LandruVMContext_t* vmContext = landruCreateVMContext(library);
    landruInitializeStdLib(library, vmContext);

    // parse the program
    //
    LandruNode_t* rootNode = landruCreateRootNode();
    //std::vector<std::pair<std::string, Json::Value*> > jsonVars;
    bool success = !landruParseProgram(rootNode, /*&jsonVars,*/ test_declarations_ws, strlen(test_declarations_ws));
    printf("Compiling test_declarations %s\n", success? "succeeded": "failed");

    landruPrintRawAST(rootNode);

    // assemble the program
    //
    LandruAssembler_t* assembler = landruCreateAssembler(library);
    int lib_count = landruLoadRequiredLibraries(assembler, rootNode, library, vmContext);
    printf("Loaded %d libraries\n", lib_count);

    landruAssemble(assembler, rootNode);

    landruVMContextSetTraceEnabled(vmContext, true);
    //landruVMContextSetBreakpoint(123); vmContext.breakPoint = breakPoint;
        
    // prepare the context for running and launch the main machine
    //
    landruInitializeContext(assembler, vmContext);
    landruLaunchMachine(vmContext, "main");

    // run on a thread
    //
    bool run = true;
    std::thread t([&run, &vmContext]()
    {
        do 
        {
            using namespace std;
            chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
            chrono::duration<double> time_span = chrono::duration_cast<chrono::seconds>(now.time_since_epoch());
            run = landruUpdate(vmContext, time_span.count());
        } 
        while (run);
    });
    t.join();

    landruReleaseAssembler(assembler);
    landruReleaseRootNode(rootNode);
    landruReleaseLibrary(library);
    landruReleaseVMContext(vmContext);
#endif

    printf("Run successfully completed\n");
}

int main(int argc, char** argv)
{
    test_declarations();
    return 0;
}
