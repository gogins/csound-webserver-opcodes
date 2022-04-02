#pragma once

#include <csdl.h>

namespace csound {

    /**
     * This creates a C++ interface to a _running_ instance of Csound. The 
     * interface includes many Csound API methods that have (wrongly, in my 
     * view) been omitted from `csdl.h`. Only those API methods that make 
     * sense during the Csound performance are included, and not all of 
     * those, though more could easily be added.
     */
    struct CsoundRuntimeInterface {
        CSOUND *csound;
        void *library_handle;
        /**
         * Functions from the Csound library.
         */
        int (*CompileCsdText_)(CSOUND *, const char *);
        int (*CompileOrc_)(CSOUND *, const char *);
        MYFLT (*EvalCode_)(CSOUND *, const char *);
        /** 
         * Stores a pointer to Csound and obtains 
         * the handle required for looking up functions.
         */
        virtual int initialize(CSOUND *csound_) {
            csound = csound_;
            // A null library path indicates that symbols should be resolved 
            // against the running program (Csound usually) and its loaded 
            // dependencies (which must include the Csound library).
            int result = csound->OpenLibrary(&library_handle, nullptr);
            CompileCsdText_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileCsdText");
            CompileOrc_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileOrc");
            EvalCode_ = (MYFLT (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundEvalCode");
            return result;
        }
        virtual int CompileCsdText(const char *csd_text) {
            int result = CompileCsdText_(csound, csd_text);
            return result;
        }
        virtual int CompileOrc(const char *orc_text) {
            int result = CompileOrc_(csound, orc_text);
            return result;
        }
        virtual MYFLT EvalCode(const char *orc_code) {
            MYFLT result = EvalCode_(csound, orc_code);
            return result;
        }
    };
        /*
    CompileCsdText
    CompileOrc
    EvalCode
    Get0dBFS
    GetAudioChannel
    GetControlChannel
    GetDebug
    GetKsmps
    GetNchnls
    GetNchnlsInput
    GetScoreOffsetSeconds
    GetScoreTime
    GetSr
    GetStringChannel
    InputMessage
    IsScorePending
    Message
    ReadScore
    RewindScore
    ScoreEvent
    SetControlChannel
    SetDebug
    SetMessageCallback
    SetScoreOffsetSeconds
    SetScorePending
    SetStringChannel
    TableGet
    TableLength
    TableSet    
    */
    
}
