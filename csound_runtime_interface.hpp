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
         * Functions obtained at run time from Csound.
         */
        int (*csoundCompileCsdText_)(CSOUND *, const char *);
        int (*csoundCompileOrc_)(CSOUND *, const char *);
        MYFLT (*csoundEvalCode_)(CSOUND *, const char *);
        MYFLT (*csoundGet0dBFS_)(CSOUND *);
        void (*csoundGetAudioChannel_)(CSOUND *csound, const char *, MYFLT *);
        MYFLT (*csoundGetControlChannel_)(CSOUND *csound, const char *name, int *);
        int (*csoundGetDebug_)(CSOUND *);
        uint32_t (*csoundGetKsmps_)(CSOUND *);
        uint32_t (*csoundGetNchnls_)(CSOUND *);
        uint32_t (*csoundGetNchnlsInput_)(CSOUND *);
        MYFLT (*csoundGetScoreOffsetSeconds_)(CSOUND *);
        MYFLT (*csoundGetScoreTime_)(CSOUND *);
        MYFLT (*csoundGetSr_)(CSOUND *);
        void (*csoundGetStringChannel_)(CSOUND *csound, const char *name, char *);
        void (*csoundInputMessage_)(CSOUND *, const char *);
        int (*csoundIsScorePending_)(CSOUND *);
        void (*csoundMessage_)(CSOUND *, const char *format, ...);
        int (*csoundReadScore_)(CSOUND *, const char *);

        /** 
         * Stores a pointer to Csound and obtains 
         * the handle required for looking up functions.
         */
        virtual int initialize(CSOUND *csound_) {
            csound = csound_;
            // A null library path indicates that symbols should be resolved 
            // from the running program (Csound usually) and its loaded 
            // dependencies (which must include the Csound library).
            int result = csound->OpenLibrary(&library_handle, nullptr);
            csoundCompileCsdText_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileCsdText");
            csoundCompileOrc_ = (int (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundCompileOrc");
            csoundEvalCode_ = (MYFLT (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundEvalCode");
            csoundGet0dBFS_ = (MYFLT (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGet0dBFS");
            csoundGetAudioChannel_ = (void (*)(CSOUND *, const char *, MYFLT *)) csound->GetLibrarySymbol(library_handle, "csoundGetAudioChannel");
            csoundGetControlChannel_ = (MYFLT (*)(CSOUND *, const char *, int *)) csound->GetLibrarySymbol(library_handle, "csoundGetControlChannel");
            csoundGetDebug_ = (int (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetDebug");
            csoundGetKsmps_ = (uint32_t (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetKsmps");
            csoundGetNchnls_ = (uint32_t (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetNchnls");
            csoundGetNchnlsInput_ = (uint32_t (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetNchnlsInput");
            csoundGetScoreOffsetSeconds_ = (MYFLT (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetScoreOffsetSeconds");
            csoundGetScoreTime_ = (MYFLT (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetScoreTime");
            csoundGetSr_ = (MYFLT (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundGetSr");
            csoundGetStringChannel_ = (void (*)(CSOUND *, const char *, char *)) csound->GetLibrarySymbol(library_handle, "csoundGetStringChannel");
            csoundInputMessage_ = (void (*)(CSOUND *, const char *)) csound->GetLibrarySymbol(library_handle, "csoundInputMessage");
            csoundIsScorePending_ = (int (*)(CSOUND *)) csound->GetLibrarySymbol(library_handle, "csoundIsScorePending");
            csoundMessage_ = (void (*)(CSOUND *, const char *format, ...)) csound->GetLibrarySymbol(library_handle, "csoundMessage");
            csoundReadScore_ = (int (*)(CSOUND *csound, const char *)) csound->GetLibrarySymbol(library_handle, "csoundReadScore");
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
        virtual MYFLT GetControlChannel(const char *name, int *err) {
            MYFLT result = csoundGetControlChannel_(csound, name, err);
            return result;
        }
        virtual int GetDebug() {
            int result = csoundGetDebug_(csound);
            return result;
        }
        virtual uint32_t GetKsmps() {
            uint32_t result = csoundGetKsmps_(csound);
            return result;
        }
        virtual uint32_t GetNchnls() {
            uint32_t result = csoundGetNchnls_(csound);
            return result;
        }
        virtual uint32_t GetNchnls_input() {
            uint32_t result = csoundGetNchnlsInput_(csound);
            return result;
        }
        virtual MYFLT GetScoreOffsetSeconds() {
            MYFLT result = csoundGetScoreOffsetSeconds_(csound);
            return result;
        }
        virtual MYFLT GetScoreTime() {
            MYFLT result = csoundGetScoreTime_(csound);
            return result;
        }
        virtual MYFLT GetSr() {
            MYFLT result = csoundGetSr_(csound);
            return result;
        }
        virtual void GetStringChannel(const char *name, char *buffer) {
            csoundGetStringChannel_(csound, name, buffer);
        }
        virtual void InputMessage(const char *sco_code) {
            csoundInputMessage_(csound, sco_code);
        }
        virtual int IsScorePending() {
            int result = csoundIsScorePending_(csound);
            return result;
        }
        virtual void Message(const char *message_) {
            csoundMessage_(csound, message_);
        }
        virtual int ReadScore(const char *sco_code) {
            int result = csoundReadScore_(csound, sco_code);
            return result;
        }
        
    };
/*
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
