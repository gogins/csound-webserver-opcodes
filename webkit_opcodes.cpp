#include <gtk/gtk.h>
#include <future>
#include <chrono>
#include <csound/csound.hpp>
#include <csound/OpcodeBase.hpp>
#include <cstdio>
#include <ctime>
#include <ctime>
#include <queue>
#include <thread>
#include <jsoncpp/json/json.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <map>
#include <memory>
#include <glib-2.0/glib.h>
#include <glib-2.0/gmodule.h>
#include <libsoup-2.4/libsoup/soup.h>
#include <libsoup-2.4/libsoup/soup-address.h>
#include <webkit2/webkit2.h>

#include "csoundskeleton.h"

// From: https://wiki.gnome.org/Projects/WebKitGtk/ProgrammingGuide/Tutorial

namespace webkit_opcodes {
    /**
     * This class implements the skeleton of the Csound proxy using an
     * instance of Csound and a network connector provided by the opcodes.
     *
     * Because the Csound proxy is created by opcodes that exist only during 
     * the Csound performance, all Csound API methods for creating or 
     * destroying Csound, or for starting or stopping the Csound performance, 
     * have had to be omitted.
     *
     * In addition, Ajax (XMLHttpRequest) is used rather than WebSockets 
     * because Ajax is synchronous (request/response) which fits the use here, 
     * whereas WebSockets are asynchronous. Making them synchronous is 
     * possible, but then where are we?    
     */
    struct CsoundServer : public CsoundSkeleton {
        std::shared_ptr<Csound> csound;
        CsoundServer(std::shared_ptr<Csound> csound_, jsonrpc::AbstractServerConnector &connector) : csound(csound_), CsoundSkeleton(connector) {
            std::fprintf(stderr, "CsoundServer::CsoundServer: csound: %p connector: %p\n", csound.get(), &connector);
        };
        virtual ~CsoundServer() {}
        int CompileCsdText(const std::string& csd_text) override {
            int result = csound->CompileCsdText(csd_text.c_str());
            return result;
        }
        int CompileOrc(const std::string& orc_code) override {
            int result = csound->CompileOrc(orc_code.c_str());
            return result;
        }
        MYFLT EvalCode(const std::string& orc_code) override {
            MYFLT value = csound->CompileOrc(orc_code.c_str());
            return value;
        }
        double Get0dBFS() override {
            MYFLT value = csound->Get0dBFS();
            return value;
        }
        Json::Value GetAudioChannel(const std::string& channel_name) override {
            auto ksmps = csound->GetKsmps();
            std::vector<MYFLT> buffer(ksmps);
            csound->GetAudioChannel(channel_name.c_str(), &buffer.front());
            Json::Value value(Json::arrayValue);
            value.resize(ksmps);
            for(int i = 0; i < ksmps; ++i) {
                value[i] = buffer[i];
            }
            return value;
        }
        double GetControlChannel(const std::string& channel_name) override {
            MYFLT value = csound->GetControlChannel(channel_name.c_str());
            return value;
        }
        bool GetDebug() override {
            bool value = csound->GetDebug();
            return value;
        }
        int GetKsmps() override {
            int value = csound->GetKsmps();
            return value;
        }
        int GetNchnls() override {
            int value = csound->GetNchnls();
            return value;
        }
        int GetNchnlsInput() override {
            int value = csound->GetNchnlsInput();
            return value;
        }
        double GetScoreOffsetSeconds() override {
            MYFLT value = csound->GetScoreOffsetSeconds();
            return value;
        }
        double GetScoreTime() override {
            MYFLT value = csound->GetScoreTime();
            return value;
        }
        std::string GetStringChannel(const std::string &channel_name) override {
            char buffer[0x500];
            csound->GetStringChannel(channel_name.c_str(), buffer);
            return buffer;
        }
        int GetSr() override {
            int value = csound->GetSr();
            return value;
        }
        int InputMessage(const std::string &sco_text) override {
            int result = OK;
            csound->InputMessage(sco_text.c_str());
            return result;
        }
        bool IsScorePending() override {
            bool value = csound->IsScorePending();
            return value;
        }
        void Message(const std::string& message) override {
            csound->Message(message.c_str());
        }
        int ReadScore(const std::string& sco_code) override {
            int result = csound->ReadScore(sco_code.c_str());
            return result;
        }
        int RewindScore() override {
            int result = OK;
            csound->RewindScore();
            return result;
        }
        // Does nothing at this time.
        int ScoreEvent(const std::string& opcode_code, const Json::Value& pfields) override {
            int result = OK;
            return result;
        }
        //~ int SetAudioChannel(const std::string& channel_name, const Json::Value& channel_value) override {
        //~ int result = OK;
        //~ std::vector<MYFLT> buffer;
        //~ auto ksmps = csound->GetKsmps();
        //~ for (int i = 0; i < ksmps; ++i) {
        //~ buffer.push_back(channel_value[i].asDouble());
        //~ }
        //~ csound->SetAudioChannel(channel_name.c_str(), buffer.data());
        //~ return result;
        //~ }
        bool SetDebug(bool enabled) override {
            bool result = false;
            csound->SetDebug(enabled);
            return result;
        }
        int SetControlChannel(const std::string& channel_name, double channel_value) override {
            int result = OK;
            csound->SetControlChannel(channel_name.c_str(), channel_value);
            return result;
        }
        // Does nothing at this time.
        int SetMessageCallback(const Json::Value& callback)  override {
            int result = OK;
            return result;
        }
        int SetScoreOffsetSeconds(double seconds) override {
            int result = OK;
            csound->SetScoreOffsetSeconds(seconds);
            return result;
        }
        int SetScorePending(bool pending) override {
            int result = OK;
            csound->SetScorePending(pending);
            return result;
        }
        int SetStringChannel(const std::string& channel_name, const std::string& channel_value) override {
            int result;
            csound->SetStringChannel(channel_name.c_str(), (char *)channel_value.c_str());
            return result;
        }
        int TableLength(int table_number) override {
            auto result = csound->TableLength(table_number);
            return result;
        }
        double TableGet(int index, int table_number) override {
            auto result = csound->TableGet(index, table_number);
            return result;
        }
        int TableSet(int index, int table_number, double value) override {
            int result = OK;
            csound->TableSet(index, table_number, value);
            return result;
        }
    };

    static bool diagnostics_enabled = true;

    struct CsoundWebKit {
        std::shared_ptr<Csound> csound;
        std::shared_ptr<jsonrpc::HttpServer> network_server;
        std::shared_ptr<CsoundServer> csound_server;
        GtkWidget *main_window;
        WebKitWebView *web_view;
        WebKitSettings *webkit_settings;
        WebKitWebContext *webkit_context;
        int rpc_port = 8383;
        bool load_finished = false;
        std::queue<std::string> script_queue;
        CsoundWebKit(CSOUND *csound_, int rpc_port_) {
            gtk_init(nullptr, nullptr);
            csound = std::shared_ptr<Csound>(new Csound(csound_));
            if(rpc_port_ != -1) {
                rpc_port = rpc_port_;
            }
            network_server = std::shared_ptr<jsonrpc::HttpServer>(new jsonrpc::HttpServer(rpc_port));
            csound_server = std::shared_ptr<CsoundServer>(new CsoundServer(csound, *network_server));
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: network_server: Starting to listen on rpc_port_: %d\n", rpc_port_);
            network_server->StartListening();
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: network_server: Now listening on rpc_port_: %d...\n", rpc_port_);
        }
        static std::unique_ptr<CsoundWebKit> create(CSOUND *csound_, int rpc_channel_) {
            std::unique_ptr<CsoundWebKit> result(new CsoundWebKit(csound_, rpc_channel_));
            return result;
        }
        virtual void web_view_load_changed(WebKitWebView *web_view, WebKitLoadEvent load_event) {
            const char *provisional_uri = nullptr;
            const char *redirected_uri = nullptr;
            const char *loaded_uri = nullptr;
            switch(load_event) {
                case WEBKIT_LOAD_STARTED:
                    /* New load, we have now a provisional URI */
                    provisional_uri = webkit_web_view_get_uri(web_view);
                    /* Here we could start a spinner or update the
                     * location bar with the provisional URI */
                    if(diagnostics_enabled) {
                        std::fprintf(stderr, "WebKitCsound::web_view_load_changed: web_view: %p load_event: %d.\n", web_view, load_event);
                    }
                    break;
                case WEBKIT_LOAD_REDIRECTED:
                    redirected_uri = webkit_web_view_get_uri(web_view);
                    if(diagnostics_enabled) {
                        std::fprintf(stderr, "WebKitCsound::web_view_load_changed: web_view: %p load_event: %d.\n", web_view, load_event);
                    }
                    break;
                case WEBKIT_LOAD_COMMITTED:
                    /* The load is being performed. Current URI is
                     * the final one and it won't change unless a new
                     * load is requested or a navigation within the
                     * same page is performed */
                    loaded_uri = webkit_web_view_get_uri(web_view);
                    break;
                case WEBKIT_LOAD_FINISHED:
                    load_finished = true;
                    if(diagnostics_enabled) {
                        std::fprintf(stderr, "WebKitCsound::web_view_load_finished: web_view: %p load_event: %d.\n", web_view, load_event);
                    }
                    break;
            }
        }
        static void web_view_load_changed_(WebKitWebView *web_view,
            WebKitLoadEvent load_event,
            gpointer user_data) {
            ((CsoundWebKit *)user_data)->web_view_load_changed(web_view, load_event);
        }
        virtual int open(const char *window_title, int width, int height, bool fullscreen) {
            int result = OK;
            main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(main_window), window_title);
            gtk_window_set_default_size(GTK_WINDOW(main_window), width, height);
            if (fullscreen == true) {
                gtk_window_fullscreen(GTK_WINDOW(main_window));
            }
            web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
            g_signal_connect(web_view, "load_changed", G_CALLBACK(&CsoundWebKit::web_view_load_changed_), this);
            webkit_settings = webkit_web_view_get_settings(web_view);
            webkit_settings_set_enable_javascript(webkit_settings, true);
            webkit_settings_set_enable_webgl(webkit_settings, true);
            webkit_settings_set_enable_webaudio(webkit_settings, true);
            webkit_settings_set_media_playback_requires_user_gesture(webkit_settings, false);
            webkit_settings_set_enable_write_console_messages_to_stdout(webkit_settings, true);
            webkit_settings_set_enable_media_stream(webkit_settings, true);
            webkit_settings_set_enable_mediasource(webkit_settings, true);
            webkit_settings_set_enable_media_capabilities(webkit_settings, true);
            webkit_settings_set_enable_fullscreen(webkit_settings, true);
            webkit_settings_set_enable_developer_extras(webkit_settings, true);
            // TODO: See if these two are really a good idea.
            webkit_settings_set_allow_file_access_from_file_urls(webkit_settings, true);
            webkit_settings_set_allow_universal_access_from_file_urls(webkit_settings, true);
            webkit_context = webkit_web_view_get_context(web_view);
            gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(web_view));
            gtk_widget_grab_focus(GTK_WIDGET(web_view));
            gtk_widget_show_all(main_window);
            return result;
        }
        virtual void load_uri(const char *uri) {          
            webkit_web_view_load_uri(web_view, uri);
        }
        virtual void load_html(const char *content, const char *base_uri) {
            webkit_web_view_load_html(web_view, content, base_uri);
        }
        void web_view_javascript_finished(GObject *object, GAsyncResult *result) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebKit::run_javascript_callback...\n");
            WebKitJavascriptResult *js_result = nullptr;
            GError *error = nullptr;
            js_result = webkit_web_view_run_javascript_finish(WEBKIT_WEB_VIEW(object), result, &error);
            if(!js_result) {
                g_warning("CsoundWebKit::run_javascript_callback: js_result: %p: message: %s", js_result, error->message);
                g_error_free(error);
                return;
            }
            auto jsc_value = webkit_javascript_result_get_js_value(js_result);
            if (!(jsc_value_is_undefined(jsc_value) || jsc_value_is_null(jsc_value))) {
                gchar *str_value = jsc_value_to_string(jsc_value);
                if (diagnostics_enabled) g_print("CsoundWebKit::run_javascript_callback: value: %s\n", str_value);
            }
            webkit_javascript_result_unref(js_result);
        }
        static void web_view_javascript_finished_(GObject *object, GAsyncResult *result, gpointer user_data) {
            ((CsoundWebKit *)user_data)->web_view_javascript_finished(object, result);
        }
        virtual int run_javascript(std::string javascript_code) {
            int result = OK;
            script_queue.push(javascript_code);
            return result;
        }
        virtual void handle_events() {
            while (g_main_context_pending(NULL) == TRUE) {
                // Data race if this does not block?
                g_main_context_iteration(NULL, FALSE);
                ///std::this_thread::sleep_for(std::chrono::100us);
            }
            if (load_finished == true) {
                while (script_queue.empty() == false) {
                    auto javascript_code = script_queue.front();
                    script_queue.pop();
                    if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebKit::run_javascript: code: \"%s\"\n", javascript_code.c_str());
                    webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(web_view), javascript_code.c_str(), nullptr, web_view_javascript_finished_, this);
                }
            }
        }
    };

    static std::map<int, std::shared_ptr<CsoundWebKit> > browsers_for_handles;
    
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
                std::shared_ptr<CsoundWebKit> browser = CsoundWebKit::create(csound, rpc_port);
                int handle = browsers_for_handles.size();
                browsers_for_handles[handle] = browser;
                *i_browser_handle = handle;
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
            std::shared_ptr<CsoundWebKit> browser;
            int init(CSOUND *csound) {
                log(csound, "webkit_open_uri::init: this: %p\n", this);
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                char *S_window_title = S_window_title_->data;
                char *S_uri = S_uri_->data;
                int i_width = *i_width_;
                int i_height = *i_height_;
                bool i_fullscreen = *i_fullscreen_;
                browser = browsers_for_handles[i_browser_handle];
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
            std::shared_ptr<CsoundWebKit> browser;
            int init(CSOUND *csound) {
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                char *S_window_title = S_window_title_->data;
                char *S_html = S_html_->data;
                char *S_base_uri = S_base_uri_->data;
                int i_width = *i_width_;
                int i_height = *i_height_;
                bool i_fullscreen = *i_fullscreen_;
                browser = browsers_for_handles[i_browser_handle];
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

    class webkit_run_javascript : public csound::OpcodeBase<webkit_run_javascript> {
        public:
            // INPUTS
            MYFLT *i_browser_handle_;
            STRINGDAT *S_javascript_code_;
            int init(CSOUND *csound) {
                int result = OK;
                int i_browser_handle = *i_browser_handle_;
                auto browser = browsers_for_handles[i_browser_handle];
                char *javascript_code = S_javascript_code_->data;
                result = browser->run_javascript(javascript_code);
                return result;
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
    
    using namespace std::chrono_literals; 
    /**
     * Make the browsers accessible to other C++ modules in LLVM.
     * Tries waiting for browsers that are still in process of creation.
    */
       
    static void webkit_run_javascript_routine(int browser_handle, std::string javascript_code)
    {
       for (int sleep_i = 0; sleep_i < 2000; ++sleep_i) {
            if (webkit_opcodes::browsers_for_handles.find(browser_handle) != webkit_opcodes::browsers_for_handles.end()) {
                webkit_opcodes::browsers_for_handles[browser_handle]->run_javascript(javascript_code);
                std::fprintf(stderr, "webkit_execute_routine: executed.\n");
                return;
            }
            std::this_thread::sleep_for(10ms);
        }
        std::fprintf(stderr, "webkit_execute_routine: Error: no browser for handle %d! (%ld browsers)\n", browser_handle,  webkit_opcodes::browsers_for_handles.size());
    }
    static std::thread *execute_thread;
    PUBLIC void webkit_run_javascript(int browser_handle, std::string javascript_code) {
        execute_thread = new std::thread(&webkit_run_javascript_routine, browser_handle, javascript_code);
    };
    
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
        return 0;
    }

#ifndef INIT_STATIC_MODULES
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
        return 0;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
        return csoundModuleInit_webkit_opcodes(csound);
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
        return csoundModuleDestroy_webkit_opcodes(csound);
    }
#endif
}

