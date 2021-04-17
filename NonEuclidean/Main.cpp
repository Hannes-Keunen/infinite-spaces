#include "Engine.h"

#include <string.h>

int main(int argc, char* argv[])
{
    Engine::Args args;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--enableVr") == 0)
        {
            args.enableVr = true;
        }
        else if (strcmp(argv[i], "--showMinimap") == 0)
        {
            args.showMinimap = true;
        }
        else if (strcmp(argv[i], "--physicalSize") == 0)
        {
            args.physicalSize = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--roomSize") == 0)
        {
            args.roomSize = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--removalStrategy") == 0)
        {
            if (strcmp(argv[++i], "immediate") == 0)
            {
                args.removalStrategy = RemovalStrategy::IMMEDIATE;
            }
            else if (strcmp(argv[i], "keep_one") == 0)
            {
                args.removalStrategy = RemovalStrategy::KEEP_ONE;
            }
            else
            {
                printf("Warning: Invalid removal strategy: %s. Must be one of \"immediate\", \"keep_one\".\n", argv[i]);
            }
        }
        else
        {
            printf("Warning: Invalid argument: %s\n", argv[i]);
        }
    }

    // Run the main engine
    Engine engine(args);
    return engine.Run();
}
