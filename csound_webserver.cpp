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
#include <cpp-httplib/httplib.h>
#include <jsonrpccxx/server.hpp>

namespace csound_webserver {
    
    class CsoundWebServer;
    
    typedef csound::heap_object_manager_t<CsoundWebServer> webservers;
    
    static bool diagnostics_enabled = true;

    struct CsoundWebServer {
        ///std::shared_ptr<Csound> csound;
        int port = 8080;
        httplib::Server server;
        bool load_finished = false;
        CsoundWebServer(CSOUND *csound_, int port_) {
            ///csound = std::shared_ptr<Csound>(new Csound(csound_));
            if(port_ != -1) {
                port = port_;
            }
            server.listen("0.0.0.0", port);
        }
        virtual ~CsoundWebServer() {
            using namespace std::chrono_literals; 
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer...\n");
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer.\n");
        }
        static std::unique_ptr<CsoundWebServer> create(CSOUND *csound_, int rpc_channel_) {
            std::unique_ptr<CsoundWebServer> result(new CsoundWebServer(csound_, rpc_channel_));
            return result;
        }
        virtual void load_uri(const char *uri) {          
            ///csound_webserver_web_view_load_uri(web_view, uri);
        }
        virtual void load_html(const char *content, const char *base_uri) {
            ///csound_webserver_web_view_load_html(web_view, content, base_uri);
        }
    };

    class csound_webserver_create : public csound::OpcodeBase<csound_webserver_create> {
        public:
            // OUTPUTS
            MYFLT *i_browser_handle;
            // INPUTS
            MYFLT *i_port;
            MYFLT *i_diagnostics_enabled;
            int init(CSOUND *csound) {
                int result = OK;
                int port = *i_port;
                diagnostics_enabled = *i_diagnostics_enabled;
                CsoundWebServer *webkit = CsoundWebServer::create(csound, port).release();
                int handle = webservers::instance().handle_for_object(csound, webkit);
                *i_browser_handle = static_cast<MYFLT>(handle);
                return result;
            }
    };

    class csound_webserver_open_resource : public csound::OpcodeBase<csound_webserver_open_resource> {
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
            CsoundWebServer *browser;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_resource::init: this: %p\n", this);
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                char *S_window_title = S_window_title_->data;
                char *S_uri = S_uri_->data;
                int i_width = *i_width_;
                int i_height = *i_height_;
                bool i_fullscreen = *i_fullscreen_;
                browser = webservers::instance().object_for_handle(csound, i_browser_handle);
                ///browser->open(S_window_title, i_width, i_height, i_fullscreen);
                log(csound, "csound_webserver_open_resource::init: uri: %s\n", S_uri);
                browser->load_uri(S_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///browser->handle_events();
                return OK;
            }
    };

    class csound_webserver_open_html : public csound::OpcodeBase<csound_webserver_open_html> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_browser_handle_;
            STRINGDAT *S_html_text_;
            // STATE
            CsoundWebServer *browser;
            int init(CSOUND *csound) {
                int result = OK;
                int i_browser_handle = static_cast<int>(*i_browser_handle_);
                char *S_html = S_html_text_->data;
                browser = webservers::instance().object_for_handle(csound, i_browser_handle);
                ///browser->open(S_window_title, i_width, i_height, i_fullscreen);
                ///browser->load_html(S_html, S_base_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///browser->handle_events();
                return OK;
            }
    };

};

/**
 * i_webserver_handle webserver_create S_base_uri, i_port [, i_diagnostics_enabled]
 * webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
 * webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
 */
extern "C" {
    
    PUBLIC int csoundModuleInit_csound_webserver(CSOUND *csound) {
        std::fprintf(stderr, "csoundModuleInit_csound_webserver...\n");
        int status = csound->AppendOpcode(csound,
                (char *)"csound_webserver_create",
                sizeof(csound_webserver::csound_webserver_create),
                0,
                1,
                (char *)"i",
                (char *)"Sjo",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_create::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"csound_webserver_open_resource",
                sizeof(csound_webserver::csound_webserver_open_resource),
                0,
                3,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::init_,
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"csound_webserver_open_html",
                sizeof(csound_webserver::csound_webserver_open_html),
                0,
                3,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_html::init_,
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_html::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        return status;
    }

    PUBLIC int csoundModuleDestroy_csound_webserver(CSOUND *csound) {
        csound_webserver::webservers::instance().module_destroy(csound);
        return OK;
    }

#ifndef INIT_STATIC_MODULES
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
        return OK;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
        return csoundModuleInit_csound_webserver(csound);
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
        return csoundModuleDestroy_csound_webserver(csound);
    }
#endif
}

