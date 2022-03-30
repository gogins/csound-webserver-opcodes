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
        std::string base_uri;
        int port;
        httplib::Server server;
        std::thread *listener_thread;
        CsoundWebServer(CSOUND *csound_, const std::string &base_uri_, int port_) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer...\n");
            base_uri = base_uri_;
            if(port_ != -1) {
                port = port_;
            }
            server.set_mount_point("/", base_uri.c_str());
            listener_thread = new std::thread(&CsoundWebServer::listen, this);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer.\n");
       }
        virtual ~CsoundWebServer() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer...\n");
            server.stop();
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer.\n");
        }
        virtual void listen() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen...");
            // Was "0.0.0.0" for all interfaces -- for security we limit this 
            // to localhost.
            server.listen("localhost", port);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen.");
        }
        static std::unique_ptr<CsoundWebServer> create(CSOUND *csound_, const std::string &base_uri_, int port_) {
            std::unique_ptr<CsoundWebServer> result(new CsoundWebServer(csound_, base_uri_, port_));
            return result;
        }
        virtual void load_resource(const char *uri) {          
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
            STRINGDAT *S_base_uri;
            MYFLT *i_port;
            MYFLT *i_diagnostics_enabled;
            int init(CSOUND *csound) {
                int result = OK;
                std::string base_uri_ = S_base_uri->data;
                int port = *i_port;
                diagnostics_enabled = *i_diagnostics_enabled;
                CsoundWebServer *webkit = CsoundWebServer::create(csound, base_uri_, port).release();
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
            STRINGDAT *S_uri_;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_resource::init: this: %p\n", this);
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                char *S_uri = S_uri_->data;
                server = webservers::instance().object_for_handle(csound, i_browser_handle);
                ///server->open(S_window_title, i_width, i_height, i_fullscreen);
                log(csound, "csound_webserver_open_resource::init: uri: %s\n", S_uri);
                server->load_resource(S_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///server->handle_events();
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
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                int result = OK;
                int i_browser_handle = static_cast<int>(*i_browser_handle_);
                char *S_html = S_html_text_->data;
                server = webservers::instance().object_for_handle(csound, i_browser_handle);
                ///server->open(S_window_title, i_width, i_height, i_fullscreen);
                ///server->load_html(S_html, S_base_uri);
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///server->handle_events();
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
                (char *)"webserver_create",
                sizeof(csound_webserver::csound_webserver_create),
                0,
                1,
                (char *)"i",
                (char *)"Sio",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_create::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_open_resource",
                sizeof(csound_webserver::csound_webserver_open_resource),
                0,
                3,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::init_,
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_open_html",
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

