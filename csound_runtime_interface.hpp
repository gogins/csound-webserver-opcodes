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
        CSOUND *csound_;
        void *library_handle;
        /**
         * Functions obtained at run time from Csound.
         */
        int (*csoundCompileCsdText_)(CSOUND *, const char *);
        int (*csoundCompileOrc_)(CSOUND *, const char *);
        MYFLT (*csoundEvalCode_)(CSOUND *, const char *);
        MYFLT (*csoundGet0dBFS_)(CSOUND *);
        void (*csoundGetAudioChannel_)(CSOUND *, const char *, MYFLT *);
        MYFLT (*csoundGetControlChannel_)(CSOUND *, const char *, int *);
        int (*csoundGetDebug_)(CSOUND *);
        uint32_t (*csoundGetKsmps_)(CSOUND *);
        uint32_t (*csoundGetNchnls_)(CSOUND *);
        uint32_t (*csoundGetNchnlsInput_)(CSOUND *);
        MYFLT (*csoundGetScoreOffsetSeconds_)(CSOUND *);
        MYFLT (*csoundGetScoreTime_)(CSOUND *);
        MYFLT (*csoundGetSr_)(CSOUND *);
        void (*csoundGetStringChannel_)(CSOUND *, const char *, char *);
        void (*csoundInputMessage_)(CSOUND *, const char *);
        int (*csoundIsScorePending_)(CSOUND *);
        void (*csoundMessage_)(CSOUND *, const char *, ...);
        int (*csoundReadScore_)(CSOUND *, const char *);
        void (*csoundRewindScore_)(CSOUND *);
        int (*csoundScoreEvent_)(CSOUND *,char, const MYFLT *, long);
        void (*csoundSetControlChannel_)(CSOUND *, const char *, MYFLT);
        void (*csoundSetDebug_)(CSOUND *, int );
        void (*csoundSetScoreOffsetSeconds_)(CSOUND *, MYFLT);        
        void (*csoundSetScorePending_)(CSOUND *, int);
        void (*csoundSetStringChannel_)(CSOUND *, const char *, char *);
        MYFLT (*csoundTableGet_)(CSOUND *, int, int);
        int (*csoundTableLength_)(CSOUND *, int);
        int (*csoundTableSet_)(CSOUND *, int, int, MYFLT);
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
            csoundCompileCsdText_ = (int (*)(CSOUND *, const char *)) csound_->GetLibrarySymbol(library_handle, "csoundCompileCsdText");
            csoundCompileOrc_ = (int (*)(CSOUND *, const char *)) csound_->GetLibrarySymbol(library_handle, "csoundCompileOrc");
            csoundEvalCode_ = (MYFLT (*)(CSOUND *, const char *)) csound_->GetLibrarySymbol(library_handle, "csoundEvalCode");
            csoundGet0dBFS_ = (MYFLT (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGet0dBFS");
            csoundGetAudioChannel_ = (void (*)(CSOUND *, const char *, MYFLT *)) csound_->GetLibrarySymbol(library_handle, "csoundGetAudioChannel");///
            csoundGetControlChannel_ = (MYFLT (*)(CSOUND *, const char *, int *)) csound_->GetLibrarySymbol(library_handle, "csoundGetControlChannel");
            csoundGetDebug_ = (int (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetDebug");
            csoundGetKsmps_ = (uint32_t (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetKsmps");
            csoundGetNchnls_ = (uint32_t (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetNchnls");
            csoundGetNchnlsInput_ = (uint32_t (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetNchnlsInput");
            csoundGetScoreOffsetSeconds_ = (MYFLT (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetScoreOffsetSeconds");
            csoundGetScoreTime_ = (MYFLT (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetScoreTime");
            csoundGetSr_ = (MYFLT (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundGetSr");
            csoundGetStringChannel_ = (void (*)(CSOUND *, const char *, char *)) csound_->GetLibrarySymbol(library_handle, "csoundGetStringChannel");
            csoundInputMessage_ = (void (*)(CSOUND *, const char *)) csound_->GetLibrarySymbol(library_handle, "csoundInputMessage");///
            csoundIsScorePending_ = (int (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundIsScorePending");
            csoundMessage_ = (void (*)(CSOUND *, const char *format, ...)) csound_->GetLibrarySymbol(library_handle, "csoundMessage");///
            csoundReadScore_ = (int (*)(CSOUND *, const char *)) csound_->GetLibrarySymbol(library_handle, "csoundReadScore");
            csoundRewindScore_ = (void (*)(CSOUND *)) csound_->GetLibrarySymbol(library_handle, "csoundRewindScore");///
            csoundScoreEvent_ = (int (*)(CSOUND *,char, const MYFLT *, long)) csound_->GetLibrarySymbol(library_handle, "csoundScoreEvent");///
            csoundSetControlChannel_ = (void (*)(CSOUND *, const char *, MYFLT)) csound_->GetLibrarySymbol(library_handle, "csoundSetControlChannel");
            csoundSetDebug_ = (void (*)(CSOUND *, int )) csound_->GetLibrarySymbol(library_handle, "csoundSetDebug");
            csoundSetScoreOffsetSeconds_ = (void (*)(CSOUND *, MYFLT)) csound_->GetLibrarySymbol(library_handle, "csoundSetScoreOffsetSeconds");
            csoundSetScorePending_ = (void (*)(CSOUND *, int)) csound_->GetLibrarySymbol(library_handle, "csoundSetScorePending");
            csoundTableGet_ = (MYFLT (*)(CSOUND *, int, int)) csound_->GetLibrarySymbol(library_handle, "csoundTableGet");
            csoundTableLength_ = (int (*)(CSOUND *, int)) csound_->GetLibrarySymbol(library_handle, "csoundTableLength");
            csoundTableSet_ = (int (*)(CSOUND *, int, int, MYFLT)) csound_->GetLibrarySymbol(library_handle, "csoundTableSet");
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
