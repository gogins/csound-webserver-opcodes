#include <App.h>
#include <gtk/gtk.h>
#include <csound/csound.hpp>
#include <csound/OpcodeBase.hpp>
#include <cstdio>
#include <string>
#include <jsoncpp/json/json.h>
#include <map>
#include <memory>
#include <glib-2.0/glib.h>
#include <glib-2.0/gmodule.h>
#include <libsoup-2.4/libsoup/soup.h>
#include <libsoup-2.4/libsoup/soup-address.h>
#include <webkit2/webkit2.h>
#include <jsonrpccpp/server/connectors/httpserver.h>

#include "csoundskeleton.h"

// From: https://wiki.gnome.org/Projects/WebKitGtk/ProgrammingGuide/Tutorial

namespace webkit_opcodes {
    
void on_gjs_csound_hello() {
    std::printf("Hello from Csound!\n");
}

/**
 * This class implements the skeleton of the Csound proxy using an 
 * instance of Csound and a network connector provided by the opcodes.
 *
 * Because the Csound proxy is created by opcodes that exist only during the 
 * Csound performance, all Csound API methods for creating or destroying 
 * Csound, or for starting or stopping the Csound performance, have had to be 
 * omitted.
 */
struct CsoundServer : public CsoundSkeleton {
    std::shared_ptr<Csound> csound;
    CsoundServer(std::shared_ptr<Csound> csound_, jsonrpc::AbstractServerConnector &connector) : csound(csound_), CsoundSkeleton(connector) {
        std::fprintf(stderr, "CsoundServer::CsoundServer: csound: %p connector: %p\n", csound.get(), &connector);
    };
    virtual ~CsoundServer(){}     
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
        for (int i = 0; i < ksmps; ++i) {
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

/* ws->getUserData returns one of these */
struct PerSocketData {
    /* Fill with user data */
};

struct CsoundWebKit {
    std::shared_ptr<Csound> csound;
    std::shared_ptr<jsonrpc::HttpServer> network_server;
    std::shared_ptr<CsoundServer> csound_server;
    std::shared_ptr<uWS::App> websocket_server;
    std::thread *websocket_server_thread;
    GtkWidget *main_window;
    WebKitWebView *web_view;
    WebKitSettings *webkit_settings;
    WebKitWebContext *webkit_context;
    WebKitWebInspector *webkit_inspector;
    bool diagnostics_enabled;
    int rpc_port = 8383;
     CsoundWebKit(CSOUND *csound_, int rpc_port_) {
        diagnostics_enabled = true;
        gtk_init(nullptr, nullptr);
        csound = std::shared_ptr<Csound>(new Csound(csound_));
        if (rpc_port_ != -1) {
            rpc_port = rpc_port_;
        }
        websocket_server_thread = new std::thread([this]() {
        });
        network_server = std::shared_ptr<jsonrpc::HttpServer>(new jsonrpc::HttpServer(rpc_port));
        csound_server = std::shared_ptr<CsoundServer>(new CsoundServer(csound, *network_server));
        std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: network_server: Starting to listen on rpc_port_: %d\n", rpc_port_);
        network_server->StartListening();
        std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: network_server: Now listening on rpc_port_: %d...\n", rpc_port_);
     }
    static std::unique_ptr<CsoundWebKit> create(CSOUND *csound_, int rpc_channel_) {
        std::unique_ptr<CsoundWebKit> result(new CsoundWebKit(csound_, rpc_channel_));
        return result;
    }
    virtual int open(const char *window_title, int width, int height) {
        int result = OK;
        main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size(GTK_WINDOW(main_window), width, height);
        gtk_window_set_title(GTK_WINDOW(main_window), window_title);
        web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
        webkit_settings = webkit_web_view_get_settings(web_view);
        webkit_settings_set_enable_javascript(webkit_settings, true);
        webkit_settings_set_enable_webgl(webkit_settings, true);
        webkit_settings_set_enable_webaudio(webkit_settings, true);
        webkit_settings_set_media_playback_requires_user_gesture(webkit_settings, false);
        webkit_settings_set_enable_write_console_messages_to_stdout(webkit_settings, true);
        webkit_settings_set_enable_media_stream(webkit_settings, true);
        webkit_settings_set_enable_mediasource(webkit_settings, true);
        webkit_settings_set_enable_media_capabilities(webkit_settings, true);
        webkit_settings_set_enable_developer_extras(webkit_settings, true);
        // TODO: See if these two are really a good idea.
        webkit_settings_set_allow_file_access_from_file_urls(webkit_settings, true);
        webkit_settings_set_allow_universal_access_from_file_urls(webkit_settings, true);
        webkit_context = webkit_web_view_get_context(web_view);        
        gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(web_view));
        // Make sure that when the browser area becomes visible, it will get mouse
        // and keyboard events
        gtk_widget_grab_focus(GTK_WIDGET(web_view));
        // Make sure the main window and all its contents are visible
        gtk_widget_show_all(main_window);
        return result;
    }
    virtual void load_uri(const char *uri) {
        webkit_web_view_load_uri(web_view, uri);
    }
    virtual void load_html(const char *content, const char *base_uri) {
        webkit_web_view_load_html(web_view, content, base_uri);
    }
    virtual MYFLT run_javascript(int page_id, const std::string &javascript_code, bool asynchronous) {
        MYFLT result = OK;
        return result;
    }
    virtual void handle_events() {
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
    }
};    

static std::map<int, std::shared_ptr<CsoundWebKit> > browsers_for_handles;

class WebKitCreate : public csound::OpcodeBase<WebKitCreate>
{
public:
    // OUTPUTS
    MYFLT *i_browser_handle;
    // INPUTS
    MYFLT *i_rpc_port;
    int init(CSOUND *csound) {
        int result = OK;
        int rpc_port = *i_rpc_port;
        std::shared_ptr<CsoundWebKit> browser = CsoundWebKit::create(csound, rpc_port);
        int handle = browsers_for_handles.size();
        browsers_for_handles[handle] = browser;
        *i_browser_handle = handle;
        return result;
    }
};

class WebKitOpenUri : public csound::OpcodeBase<WebKitOpenUri>
{
public:
    // OUTPUTS
    // INPUTS
    MYFLT *i_browser_handle_;
    STRINGDAT *S_window_title_;
    STRINGDAT *S_uri_;
    MYFLT *i_width_;
    MYFLT *i_height_;
    // STATE
    std::shared_ptr<CsoundWebKit> browser;
    int init(CSOUND *csound) {
        log(csound, "WebKitOpenUri::init: this: %p\n", this);
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        char *S_window_title = S_window_title_->data;
        char *S_uri = S_uri_->data;
        int i_width = *i_width_;
        int i_height = *i_height_;
        browser = browsers_for_handles[i_browser_handle];
        browser->open(S_window_title, i_width, i_height);
        log(csound, "WebKitOpenUri::init: uri: %s\n", S_uri);
        browser->load_uri(S_uri);
        return result;
    }
    int kontrol(CSOUND *csound) {
        int result = OK;
        browser->handle_events();
        return OK;
    }
};

class WebKitOpenHtml : public csound::OpcodeBase<WebKitOpenHtml>
{
public:
    // OUTPUTS
    // INPUTS
    MYFLT *i_browser_handle_;
    STRINGDAT *S_window_title_;
    STRINGDAT *S_html_;
    STRINGDAT *S_base_uri_;
    MYFLT *i_width_;
    MYFLT *i_height_;
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
        browser = browsers_for_handles[i_browser_handle];
        browser->open(S_window_title, i_width, i_height);
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
 * i_webkit_handle webkit_create [, i_rpc_port]
 * webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height
 * webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height
 * webkit_run_javascript i_webkit_handle, S_javascript_code [, i_asynchronous]
 *
 * In addition, each Web page opened by these opcodes has a JavaScript 
 * interface to the invoking instance of Csound.
 */
extern "C" {

    PUBLIC int csoundModuleInit_webkit_opcodes(CSOUND *csound)
    {
        std::fprintf(stderr, "csoundModuleInit_webkit_opcodes...\n");
        int status = csound->AppendOpcode(csound,
                                          (char *)"webkit_create",
                                          sizeof(webkit_opcodes::WebKitCreate),
                                          0,
                                          1,
                                          (char *)"i",
                                          (char *)"j",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitCreate::init_,
                                          (int (*)(CSOUND*,void*)) 0,
                                          (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                                          (char *)"webkit_open_uri",
                                          sizeof(webkit_opcodes::WebKitOpenUri),
                                          0,
                                          3,
                                          (char *)"",
                                          (char *)"iSSii",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitOpenUri::init_,
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitOpenUri::kontrol_,
                                          (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                                          (char *)"webkit_open_html",
                                          sizeof(webkit_opcodes::WebKitOpenHtml),
                                          0,
                                          3,
                                          (char *)"",
                                          (char *)"iSSSii",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitOpenHtml::init_,
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitOpenHtml::kontrol_,
                                          (int (*)(CSOUND*,void*)) 0);
        return status;
    }

    PUBLIC int csoundModuleDestroy_webkit_opcodes(CSOUND *csound)
    {
         return 0;
    }

#ifndef INIT_STATIC_MODULES
    PUBLIC int csoundModuleCreate(CSOUND *csound)
    {
        return 0;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound)
    {
        return csoundModuleInit_webkit_opcodes(csound);
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound)
    {
        return csoundModuleDestroy_webkit_opcodes(csound);
    }
#endif
}

