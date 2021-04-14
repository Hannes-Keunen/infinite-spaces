#include "Debug.h"
#include "Engine.h"

#include <signal.h>

int main(int argc, char* argv[])
{
    // signal(SIGSEGV, SignalHandler);
    // signal(SIGABRT, SignalHandler);

    // Run the main engine
    Engine engine;
    return engine.Run();
}
