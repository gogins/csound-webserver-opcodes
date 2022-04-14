#pragma once

#include <csdl.h>
#include <nlohmann/json.hpp>

extern "C" {
    typedef int (*csoundCompileCsdText_t)(CSOUND *, const char *);
    typedef int (*csoundCompileOrc_t)(CSOUND *, const char *);
    typedef MYFLT (*csoundEvalCode_t)(CSOUND *, const char *);
    typedef MYFLT (*csoundGet0dBFS_t)(CSOUND *);
    typedef void (*csoundGetAudioChannel_t)(CSOUND *, const char *, MYFLT *);
    typedef MYFLT (*csoundGetControlChannel_t)(CSOUND *, const char *, int *);
    typedef int (*csoundGetDebug_t)(CSOUND *);
    typedef uint32_t (*csoundGetKsmps_t)(CSOUND *);
    typedef uint32_t (*csoundGetNchnls_t)(CSOUND *);
    typedef uint32_t (*csoundGetNchnlsInput_t)(CSOUND *);
    typedef MYFLT (*csoundGetScoreOffsetSeconds_t)(CSOUND *);
    typedef MYFLT (*csoundGetScoreTime_t)(CSOUND *);
    typedef MYFLT (*csoundGetSr_t)(CSOUND *);
    typedef void (*csoundGetStringChannel_t)(CSOUND *, const char *, char *);
    typedef void (*csoundInputMessage_t)(CSOUND *, const char *);
    typedef int (*csoundIsScorePending_t)(CSOUND *);
    typedef void (*csoundMessage_t)(CSOUND *, const char *, ...);
    typedef int (*csoundReadScore_t)(CSOUND *, const char *);
    typedef void (*csoundRewindScore_t)(CSOUND *);
    typedef int (*csoundScoreEvent_t)(CSOUND *,char, const MYFLT *, long);
    typedef void (*csoundSetControlChannel_t)(CSOUND *, const char *, MYFLT);
    typedef void (*csoundSetDebug_t)(CSOUND *, int );
    typedef void (*csoundSetScoreOffsetSeconds_t)(CSOUND *, MYFLT);
    typedef void (*csoundSetScorePending_t)(CSOUND *, int);
    typedef void (*csoundSetStringChannel_t)(CSOUND *, const char *, char *);
    typedef MYFLT (*csoundTableGet_t)(CSOUND *, int, int);
    typedef int (*csoundTableLength_t)(CSOUND *, int);
    typedef int (*csoundTableSet_t)(CSOUND *, int, int, MYFLT);
    /**
     * Functions obtained at run time from Csound.
     */
    csoundCompileCsdText_t csoundCompileCsdText_;
    csoundCompileOrc_t csoundCompileOrc_;
    csoundEvalCode_t csoundEvalCode_;
    csoundGet0dBFS_t csoundGet0dBFS_;
    csoundGetAudioChannel_t csoundGetAudioChannel_;
    csoundGetControlChannel_t csoundGetControlChannel_;
    csoundGetDebug_t csoundGetDebug_;
    csoundGetKsmps_t csoundGetKsmps_;
    csoundGetNchnls_t csoundGetNchnls_;
    csoundGetNchnlsInput_t csoundGetNchnlsInput_;
    csoundGetScoreOffsetSeconds_t csoundGetScoreOffsetSeconds_;
    csoundGetScoreTime_t csoundGetScoreTime_;
    csoundGetSr_t csoundGetSr_;
    csoundGetStringChannel_t csoundGetStringChannel_;
    csoundInputMessage_t csoundInputMessage_;
    csoundIsScorePending_t csoundIsScorePending_;
    csoundMessage_t csoundMessage_;
    csoundReadScore_t csoundReadScore_;
    csoundRewindScore_t csoundRewindScore_;
    csoundScoreEvent_t csoundScoreEvent_;
    csoundSetControlChannel_t csoundSetControlChannel_;
    csoundSetDebug_t csoundSetDebug_;
    csoundSetScoreOffsetSeconds_t csoundSetScoreOffsetSeconds_;
    csoundSetScorePending_t csoundSetScorePending_;
    csoundSetStringChannel_t csoundSetStringChannel_;
    csoundTableGet_t csoundTableGet_;
    csoundTableLength_t csoundTableLength_;
    csoundTableSet_t csoundTableSet_;
};

namespace csound {

    /**
     * This provides a C++ interface to a _running_ instance of Csound. The 
     * interface includes many Csound API methods that have (wrongly, in my 
     * view) been omitted from `csdl.h`. Only those API methods that make 
     * sense during the Csound performance are included, and not all of 
     * those, though more could easily be added.
     */
    struct CsoundRuntimeInterface {
        CSOUND *csound_;
        void *library_handle;
        bool &interface_initialized() {
            static bool interface_initialized_ = false;
            return interface_initialized_;
        }
        /** 
         * Stores a pointer to Csound and obtains 
         * the handle required for looking up functions.
         */
        virtual int initialize(CSOUND *csound__) {
            csound_ = csound__;
            // A null library path indicates that symbols should be resolved 
            // from the running program (Csound usually) and its loaded 
            // dependencies (which must include the Csound library).
            int result = csound_->OpenLibrary(&library_handle, nullptr);
            if (interface_initialized() == false) {
                csoundCompileCsdText_ = (csoundCompileCsdText_t) csound_->GetLibrarySymbol(library_handle, "csoundCompileCsdText");
                csoundCompileOrc_ = (csoundCompileOrc_t) csound_->GetLibrarySymbol(library_handle, "csoundCompileOrc");
                csoundEvalCode_ = (csoundEvalCode_t) csound_->GetLibrarySymbol(library_handle, "csoundEvalCode");
                csoundGet0dBFS_ = (csoundGet0dBFS_t) csound_->GetLibrarySymbol(library_handle, "csoundGet0dBFS");
                csoundGetAudioChannel_ = (csoundGetAudioChannel_t) csound_->GetLibrarySymbol(library_handle, "csoundGetAudioChannel");///
                csoundGetControlChannel_ = (csoundGetControlChannel_t) csound_->GetLibrarySymbol(library_handle, "csoundGetControlChannel");
                csoundGetDebug_ = (csoundGetDebug_t) csound_->GetLibrarySymbol(library_handle, "csoundGetDebug");
                csoundGetKsmps_ = (csoundGetKsmps_t) csound_->GetLibrarySymbol(library_handle, "csoundGetKsmps");
                csoundGetNchnls_ = (csoundGetNchnls_t) csound_->GetLibrarySymbol(library_handle, "csoundGetNchnls");
                csoundGetNchnlsInput_ = (csoundGetNchnlsInput_t) csound_->GetLibrarySymbol(library_handle, "csoundGetNchnlsInput");
                csoundGetScoreOffsetSeconds_ = (csoundGetScoreOffsetSeconds_t) csound_->GetLibrarySymbol(library_handle, "csoundGetScoreOffsetSeconds");
                csoundGetScoreTime_ = (csoundGetScoreTime_t) csound_->GetLibrarySymbol(library_handle, "csoundGetScoreTime");
                csoundGetSr_ = (csoundGetSr_t) csound_->GetLibrarySymbol(library_handle, "csoundGetSr");
                csoundGetStringChannel_ = (csoundGetStringChannel_t) csound_->GetLibrarySymbol(library_handle, "csoundGetStringChannel");
                csoundInputMessage_ = (csoundInputMessage_t) csound_->GetLibrarySymbol(library_handle, "csoundInputMessage");///
                csoundIsScorePending_ = (csoundIsScorePending_t) csound_->GetLibrarySymbol(library_handle, "csoundIsScorePending");
                csoundMessage_ = (csoundMessage_t) csound_->GetLibrarySymbol(library_handle, "csoundMessage");///
                csoundReadScore_ = (csoundReadScore_t) csound_->GetLibrarySymbol(library_handle, "csoundReadScore");
                csoundRewindScore_ = (csoundRewindScore_t) csound_->GetLibrarySymbol(library_handle, "csoundRewindScore");///
                csoundScoreEvent_ = (csoundScoreEvent_t) csound_->GetLibrarySymbol(library_handle, "csoundScoreEvent");///
                csoundSetControlChannel_ = (csoundSetControlChannel_t) csound_->GetLibrarySymbol(library_handle, "csoundSetControlChannel");
                csoundSetDebug_ = (csoundSetDebug_t) csound_->GetLibrarySymbol(library_handle, "csoundSetDebug");
                csoundSetScoreOffsetSeconds_ = (csoundSetScoreOffsetSeconds_t) csound_->GetLibrarySymbol(library_handle, "csoundSetScoreOffsetSeconds");
                csoundSetScorePending_ = (csoundSetScorePending_t) csound_->GetLibrarySymbol(library_handle, "csoundSetScorePending");
                csoundTableGet_ = (csoundTableGet_t) csound_->GetLibrarySymbol(library_handle, "csoundTableGet");
                csoundTableLength_ = (csoundTableLength_t) csound_->GetLibrarySymbol(library_handle, "csoundTableLength");
                csoundTableSet_ = (csoundTableSet_t) csound_->GetLibrarySymbol(library_handle, "csoundTableSet");
                interface_initialized() = true;
            }
            return result;
        }
        virtual int CompileCsdText(const char *csd_text) {
            int result = csoundCompileCsdText_(csound_, csd_text);
            return result;
        }
        virtual int CompileOrc(const char *orc_text) {
            int result = csoundCompileOrc_(csound_, orc_text);
            return result;
        }
        virtual MYFLT EvalCode(const char *orc_code) {
            if (diagnostics_enabled) fprintf(stderr, "Csound.EvalCode: csound: %p orc_code: %s\n", csound_, orc_code);
            MYFLT result = csoundEvalCode_(csound_, orc_code);
            if (diagnostics_enabled) fprintf(stderr, "Csound.EvalCode: result: %f\n", result);
            return result;
        }
        virtual MYFLT Get0dBFS() {
            MYFLT result = csoundGet0dBFS_(csound_);
            return result;
        }
        virtual void GetAudioChannel(const char *name, MYFLT *samples) {
            csoundGetAudioChannel_(csound_, name, samples);
        }
        virtual MYFLT GetControlChannel(const char *name, int *err) {
            MYFLT result = csoundGetControlChannel_(csound_, name, err);
            return result;
        }
        virtual int GetDebug() {
            int result = csoundGetDebug_(csound_);
            return result;
        }
        virtual uint32_t GetKsmps() {
            uint32_t result = csoundGetKsmps_(csound_);
            return result;
        }
        virtual uint32_t GetNchnls() {
            uint32_t result = csoundGetNchnls_(csound_);
            return result;
        }
        virtual uint32_t GetNchnls_input() {
            uint32_t result = csoundGetNchnlsInput_(csound_);
            return result;
        }
        virtual MYFLT GetScoreOffsetSeconds() {
            MYFLT result = csoundGetScoreOffsetSeconds_(csound_);
            return result;
        }
        virtual MYFLT GetScoreTime() {
            MYFLT result = csoundGetScoreTime_(csound_);
            return result;
        }
        virtual MYFLT GetSr() {
            MYFLT result = csoundGetSr_(csound_);
            return result;
        }
        virtual void GetStringChannel(const char *name, char *buffer) {
            csoundGetStringChannel_(csound_, name, buffer);
        }
        virtual void InputMessage(const char *sco_code) {
            csoundInputMessage_(csound_, sco_code);
        }
        virtual int IsScorePending() {
            int result = csoundIsScorePending_(csound_);
            return result;
        }
        virtual void Message(const char *message_) {
            csoundMessage_(csound_, message_);
        }
        virtual int ReadScore(const char *sco_code) {
            int result = csoundReadScore_(csound_, sco_code);
            return result;
        }
        virtual void RewindScore() {
            csoundRewindScore_(csound_);
        }
        virtual int ScoreEvent(char opcode_code, std::vector<MYFLT> &pfields) {
            int result = csoundScoreEvent_(csound_, opcode_code, &pfields[0], pfields.size());
            return result;
        }
        virtual void SetControlChannel(const char *channel_name, MYFLT channel_value) {
            csoundSetControlChannel_(csound_, channel_name, channel_value);
        }
        virtual void SetDebug(int enabled) {
            csoundSetDebug_(csound_, enabled);
        }
        virtual void SetScoreOffsetSeconds(MYFLT score_time) {
            csoundSetScoreOffsetSeconds_(csound_, score_time);
        }
        virtual void SetScorePending(int pending) {
            csoundSetScorePending_(csound_, pending);
        }
        virtual void SetStringChannel(const char *channel_name, char *channel_value) {
            csoundSetStringChannel_(csound_, channel_name, channel_value);
        }
        virtual MYFLT TableGet(int table_number, int index) {
            MYFLT result = csoundTableGet_(csound_, table_number, index);
            return result;
        }
        virtual int TableLength(int table_number) {
            MYFLT result = csoundTableLength_(csound_, table_number);
            return result;
        }
        virtual void TableSet(int table_number, int index, MYFLT value) {
            csoundTableSet_(csound_, table_number, index,  value);
        }
    };
/* TODO:
    SetMessageCallback
*/    
}
