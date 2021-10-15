<CsoundSyntheizer>
<CsOptions;
-m195 --opcode-libs="./webkit_opcodes.so"  
</CsOptions>
<CsInstruments>

alwayson "Browser" 

instr Browser
gi_browser webkit_create "return_channel"
print gi_browser
webkit_open_uri gi_browser, "Message", "file:///home/mkg/webkit-opcodes/message.html", 800, 600
webkit_inspector gi_browser
endin


prints "******* I'm about to try compiling a simple test C++ module....\n"

gS_source_code = {{

#include "clang_invokable.hpp"
#include <csound/csdl.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

// defined in this module to work around `__dso_handle` not being 
// defined in the C++ startup code.

void* __dso_handle = (void *)&__dso_handle;

extern "C" int csound_main(CSOUND *csound) {
    csound->Message(csound, ">>>>>>> Hello, world! This proves csound_main has been called with csound: %p.\\n", csound);
    std::vector<std::string> strings;
    strings.push_back("A test string...");
    csound->Message(csound, ">>>>>>> This proves a lot of libstdc++ stuff works: strings.size(): %ld strings[0]: %s\\n", strings.size(), strings[0].c_str());
    std::cerr << ">>>>>>> Now that we have manually defined our own __dso_handle, this proves std::cerr works as well!" << std::endl;
    return 0;
};

struct Hello : public ClangInvokableBase {
    int init(CSOUND *csound, OPDS *opds, MYFLT **outputs, MYFLT **inputs) override {
        csound->Message(csound, ">>>>>>> This proves clang_invoke has called into this module.\\n");
        const char *result = ">>>>>>> This proves clang_invoke can be used as an opcode that returns a string and multiplies a number by 2.";
        STRINGDAT *message = (STRINGDAT *)outputs[0];
        message->data = csound->Strdup(csound, (char *)result);
        message->size = std::strlen(result);
        MYFLT number = *inputs[0];
        MYFLT multiplied = number * 2.;
        *outputs[1] = multiplied;
        return OK;
    }
};

extern "C" {
    ClangInvokable *hello_factory() {
        auto instance = new Hello();
        std::fprintf(stderr, ">>>>>>> hello_factory created: %p\\n", instance);
        return instance;
    }
};

}}

gi_result clang_compile "csound_main", gS_source_code, "-v -std=c++14 -I/usr/local/include/csound -I.", "/usr/lib/gcc/x86_64-linux-gnu/9/libstdc++.so"

instr 2
prints "******* instr 1...\n"
prints "******* Trying to invoke Hello...\n"
S_message, i_number clang_invoke "hello_factory", 1, 2
prints "******* clang_invoke returned: \"%s\" and %d\n", S_message, i_number
prints "******* instr 1 finished.\n"
endin

</CsInstruments>
<CsScore>
f 0 [10 * 60]
i 2 2 2
</CsScore>
</CsoundSynthesizer>
