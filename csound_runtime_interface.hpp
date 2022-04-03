#pragma once

#include <csdl.h>
#include <nlohmann/json.hpp>

namespace csound {

    /**
     * This provides a C++ interface to a _running_ instance of Csound. The 
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
        int (*csoundCompileCsdText_)(CSOUND *, const char *);
        int (*csoundCompileOrc_)(CSOUND *, const char *);
        MYFLT (*csoundEvalCode_)(CSOUND *, const char *);
        MYFLT (*csoundGet0dBFS_)(CSOUND *);
        void (*csoundGetAudioChannel_)(CSOUND *csound, const char *, MYFLT *);
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
            csoundCompileCsdText_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileCsdText");
            csoundCompileOrc_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileOrc");
            csoundEvalCode_ = (MYFLT (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundEvalCode");
            csoundGet0dBFS_ = (MYFLT (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGet0dBFS");
            csoundGetAudioChannel_ = (void (*)(CSOUND *csound, const char *, MYFLT *)) csound->GetLibrarySymbol(library_handle, "csoundGetAudioChannel");
            return result;
        }
        virtual int CompileCsdText(const char *csd_text) {
            int result = csoundCompileCsdText_(csound, csd_text);
            return result;
        }
        virtual int CompileOrc(const char *orc_text) {
            int result = csoundCompileOrc_(csound, orc_text);
            return result;
        }
        virtual MYFLT EvalCode(const char *orc_code) {
            MYFLT result = csoundEvalCode_(csound, orc_code);
            return result;
        }
        virtual MYFLT Get0dBFS() {
            MYFLT result = csoundGet0dBFS_(csound);
            return result;
        }
        virtual void GetAudioChannel(const char *name, MYFLT *audio_buffer) {
            csoundGetAudioChannel_(csound, name, audio_buffer);
        }
    };
        /*
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
