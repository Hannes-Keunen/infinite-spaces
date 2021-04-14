#include <cxxabi.h>
#include <execinfo.h>
#include <string>

static std::string Demangle(const std::string& symbol)
{
    size_t nameStart = symbol.find('(') + 1;
    size_t nameEnd = symbol.find('+');
    size_t offsetEnd = symbol.find(')');
    std::string mangledName = symbol.substr(nameStart, nameEnd - nameStart);
    std::string offset = symbol.substr(nameEnd, offsetEnd - nameEnd);
    if (mangledName.size() == 0)
        return symbol;

    int status;
    char* realName = abi::__cxa_demangle(mangledName.c_str(), nullptr, 0, &status);
    if (status == 0)
        return realName + offset;
    else
        return mangledName + offset;
}

void PrintStackTrace()
{
    void* array[32];
    int size = backtrace(array, 32);
    char** symbols = backtrace_symbols(array, size);

    printf("Stack trace:\n");
    for (int i = 1; i < size; i++) { printf("[%d]:\t%s\n", i, Demangle(symbols[i]).c_str()); }

    free(symbols);
}

void SignalHandler(int signum)
{
    printf("received signal %d\n", signum);
    PrintStackTrace();
}
