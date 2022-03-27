#include <future>
#include <chrono>
#include <csound.hpp>
#include <OpcodeBase.hpp>
#include <cstdio>
#include <ctime>
#include <queue>
#include <thread>
#include <map>
#include <memory>

namespace csound_webserver {
    
    class CsoundWebserver;
    
    typedef csound::heap_object_manager_t<CsoundWebserver> webservers;
    

    static bool diagnostics_enabled = true;

    struct CsoundWebserver {
        std::shared_ptr<Csound> csound;
        int rpc_port = 8383;
        bool load_finished = false;
        std::queue<std::string> script_queue;
        CsoundWebserver(CSOUND *csound_, int rpc_port_) {
            csound = std::shared_ptr<Csound>(new Csound(csound_));
            if(rpc_port_ != -1) {
                rpc_port = rpc_port_;
            }
        }
        virtual ~CsoundWebserver() {
            using namespace std::chrono_literals; 
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebserver::~CsoundWebserver...\n");
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebserver::~CsoundWebserver.\n");
        }
        static std::unique_ptr<CsoundWebserver> create(CSOUND *csound_, int rpc_channel_) {
            std::unique_ptr<CsoundWebserver> result(new CsoundWebserver(csound_, rpc_channel_));
            return result;
        }
        virtual void load_uri(const char *uri) {          
            ///webkit_web_view_load_uri(web_view, uri);
        }
        virtual void load_html(const char *content, const char *base_uri) {
            ///webkit_web_view_load_html(web_view, content, base_uri);
        }
        virtual int run_javascript(std::string javascript_code) {
            int result = OK;
            script_queue.push(javascript_code);
            return result;
        }
        virtual void handle_events() {
    };

    class webkit_create : public csound::OpcodeBase<webkit_create> {
        public:
            // OUTPUTS
            MYFLT *i_browser_handle;
            // INPUTS
            MYFLT *i_rpc_port;
            MYFLT *i_diagnostics_enabled;
            int init(CSOUND *csound) {
                int result = OK;
                int rpc_port = *i_rpc_port;
                diagnostics_enabled = *i_diagnostics_enabled;
                CsoundWebserver *webkit = CsoundWebserver::create(csound, rpc_port).release();
                int handle = webservers::instance().handle_for_object(csound, webkit);
                *i_browser_handle = static_cast<MYFLT>(handle);
                return result;
            }
    };

    class webkit_open_uri : public csound::OpcodeBase<webkit_open_uri> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_browser_handle_;
            STRINGDAT *S_window_title_;
            STRINGDAT *S_uri_;
            MYFLT *i_width_;
            MYFLT *i_height_;
            MYFLT *i_fullscreen_;
            // STATE
            CsoundWebserver *browser;
            int init(CSOUND *csound) {
                log(csound, "webkit_open_uri::init: this: %p\n", this);
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                char *S_window_title = S_window_title_->data;
                char *S_uri = S_uri_->data;
                int i_width = *i_width_;
                int i_height = *i_height_;
                bool i_fullscreen = *i_fullscreen_;
                browser = webservers::instance().object_for_handle(csound, i_browser_handle);
                browser->open(S_window_title, i_width, i_height, i_fullscreen);
                log(csound, "webkit_open_uri::init: uri: %s\n", S_uri);
                browser->load_uri(S_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                browser->handle_events();
                return OK;
            }
    };

    class webkit_open_html : public csound::OpcodeBase<webkit_open_html> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_browser_handle_;
            STRINGDAT *S_window_title_;
            STRINGDAT *S_html_;
            STRINGDAT *S_base_uri_;
            MYFLT *i_width_;
            MYFLT *i_height_;
            MYFLT *i_fullscreen_;
            // STATE
            CsoundWebserver *browser;
            int init(CSOUND *csound) {
                int result = OK;
                int i_browser_handle = static_cast<int>(*i_browser_handle_);
                char *S_window_title = S_window_title_->data;
                char *S_html = S_html_->data;
                char *S_base_uri = S_base_uri_->data;
                int i_width = *i_width_;
                int i_height = *i_height_;
                bool i_fullscreen = *i_fullscreen_;
                browser = webservers::instance().object_for_handle(csound, i_browser_handle);
                browser->open(S_window_title, i_width, i_height, i_fullscreen);
                log(csound, "webkit_open_html::init: title: %s\n", S_window_title);
                browser->load_html(S_html, S_base_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                browser->handle_events();
                return OK;
            }
    };

};

/**
 * Here is the syntax for all the WebKit opcodes:
 *
 * i_webkit_handle webkit_create [, i_rpc_port [, i_diagnostics_enabled]]
 * webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height [, i_fullscreen]
 * webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height [, i_fullscreen]
 * webkit_run_javascript i_webkit_handle, S_javascript_code [, i_asynchronous]
 *
 * In addition, each Web page opened by these opcodes has a JavaScript
 * interface to the invoking instance of Csound.
 */
extern "C" {
    
    PUBLIC int csoundModuleInit_webkit_opcodes(CSOUND *csound) {
        std::fprintf(stderr, "csoundModuleInit_webkit_opcodes...\n");
        int status = csound->AppendOpcode(csound,
                (char *)"webkit_create",
                sizeof(webkit_opcodes::webkit_create),
                0,
                1,
                (char *)"i",
                (char *)"jo",
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_create::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webkit_open_uri",
                sizeof(webkit_opcodes::webkit_open_uri),
                0,
                3,
                (char *)"",
                (char *)"iSSiio",
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_open_uri::init_,
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_open_uri::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webkit_open_html",
                sizeof(webkit_opcodes::webkit_open_html),
                0,
                3,
                (char *)"",
                (char *)"iSSSiio",
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_open_html::init_,
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_open_html::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webkit_run_javascript",
                sizeof(webkit_opcodes::webkit_run_javascript),
                0,
                1,
                (char *)"",
                (char *)"iS",
                (int (*)(CSOUND*,void*)) webkit_opcodes::webkit_run_javascript::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        return status;
    }

    PUBLIC int csoundModuleDestroy_webkit_opcodes(CSOUND *csound) {
        webkit_opcodes::webservers::instance().module_destroy(csound);
        return OK;
    }

#ifndef INIT_STATIC_MODULES
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
        return OK;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
        return csoundModuleInit_webkit_opcodes(csound);
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
        return csoundModuleDestroy_webkit_opcodes(csound);
    }
#endif
}

