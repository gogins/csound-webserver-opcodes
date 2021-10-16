#include <gtk/gtk.h>
#include <csound/csound.hpp>
#include <csound/OpcodeBase.hpp>
#include <cstdio>
#include <string>
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

static void destroy_window_callback(GtkWidget* widget, GtkWidget* window)
{
    std::fprintf(stderr, "destroy_window_callback...\n");
}

static gboolean close_webview_callback(WebKitWebView* webView, GtkWidget* window)
{
    std::fprintf(stderr, "close_webview_callback...\n");
    return true;
}

/**
 * This class implements the abstract skeleton of the Csound proxy using an 
 * instance of Csound and a network connector provided by the opcodes.
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
    double GetControlChannel(const std::string& channel_name) override {
        MYFLT value = csound->GetControlChannel(channel_name.c_str());
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
    double GetScoreTime() override {
        MYFLT value = csound->GetScoreTime();
        return value;
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
    int SetScorePending(double score_time) override {
        int result = OK;
        csound->SetScorePending(score_time);
        return result;
    }
};

struct CsoundWebKit {
    std::shared_ptr<Csound> csound;
    std::shared_ptr<jsonrpc::HttpServer> network_server;
    std::shared_ptr<CsoundServer> csound_server;
    GtkWidget *main_window;
    WebKitWebView *web_view;
    WebKitSettings *webkit_settings;
    WebKitWebContext *webkit_context;
    WebKitWebInspector *webkit_inspector;
    JSCContext *jsc_context = nullptr;
    JSCClass *csound_class = nullptr;
    JSCValue *csound_constructor = nullptr;
    JSCValue *csound_message_callback = nullptr;
    bool diagnostics_enabled;
    std::string return_channel;
    int rpc_port = 8383;
    CsoundWebKit(CSOUND *csound_, const char *return_channel_) {
        auto temp_csound = new Csound(csound_);
        csound = std::shared_ptr<Csound>(temp_csound);
        return_channel = return_channel_;
        // Initialize GTK+
        gtk_init(nullptr, nullptr);
        diagnostics_enabled = true;
        auto temp_server = new jsonrpc::HttpServer(rpc_port);
        network_server = std::shared_ptr<jsonrpc::HttpServer>(temp_server);
        csound_server = std::shared_ptr<CsoundServer>(new CsoundServer(csound, *temp_server));
        std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: Starting to listen on port: %d\n", rpc_port);
        network_server->StartListening();
        std::fprintf(stderr, "CsoundWebKit::CsoundWebKit: Now listening...\n");
    }
    static std::unique_ptr<CsoundWebKit> create(CSOUND *csound_, const char *return_channel_) {
        std::unique_ptr<CsoundWebKit> result(new CsoundWebKit(csound_, return_channel_));
        return result;
    }
    virtual void inject_csound() {
        static bool initialized = false;
        if (initialized == true) {
            return;
        }
        
        initialized = true;
        std::fprintf(stderr, "CsoundWebKit::inject_csound...");
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
        // Set up callbacks so that if either the main window or the browser instance is
        // closed, the program will exit
        g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);
        g_signal_connect(web_view, "close", G_CALLBACK(close_webview_callback), main_window);
        ///  world = webkit_script_world_get_default();
        // Called at the right time to inject native code into the JavaScript context.
        ///g_signal_connect(world, "window-object-cleared", G_CALLBACK(window_object_cleared_callback_), this);
        ///g_signal_connect(webkit_context, "initialize-web-extensions", G_CALLBACK(close_webview_callback), main_window);
        // Make sure that when the browser area becomes visible, it will get mouse
        // and keyboard events
        gtk_widget_grab_focus(GTK_WIDGET(web_view));
        // Make sure the main window and all its contents are visible
        gtk_widget_show_all(main_window);
        // Run the main GTK+ event loop
        ///gtk_main();
        return result;
    }
    virtual void set_visible(bool visible) {
        if (visible) {
            gtk_widget_show(main_window);
        } else {
            gtk_widget_hide(main_window);
        }
    }
    virtual void load_uri(const char *uri) {
        webkit_web_view_load_uri(web_view, uri);
     }
    virtual void load_html(const char *content, const char *base_uri) {
        webkit_web_view_load_html(web_view, content, base_uri);
     }
    void run_javascript_callback(GObject *object, GAsyncResult *result) {
        std::fprintf(stderr, "run_javascript_callback...\n");
        WebKitJavascriptResult *js_result;
        JSCValue *value;
        GError *error = nullptr;
        js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW(object), result, &error);
        if (!js_result) {
            g_warning ("Error running javascript: %s", error->message);
            g_error_free (error);
            return;
        }
        value = webkit_javascript_result_get_js_value (js_result);
        jsc_context = jsc_value_get_context(value);
        std::fprintf(stderr, "run_javascript_callback: jsc_context: %p\n", jsc_context);
        gchar *str_value = jsc_value_to_string(value);
        g_print ("Script result: %s\n", str_value);
        webkit_javascript_result_unref(js_result);
    }
    static void run_javascript_callback_(GObject *object, GAsyncResult *result, gpointer user_data) {
        ((CsoundWebKit *)user_data)->run_javascript_callback(object, result);
    }
    virtual int run_javascript(const char *javascript_code) {
        //auto code = g_strdup(javascript_code);
        std::fprintf(stderr, "run_javascript %s\n", javascript_code);
        int result = OK;
        webkit_web_view_run_javascript(web_view, javascript_code, nullptr, run_javascript_callback_, this);
        //g_free(code);
        return result;
    }
    virtual void open_inspector() {
        webkit_inspector = webkit_web_view_get_inspector(web_view);
        webkit_web_inspector_show(webkit_inspector);
    }
};    

static std::map<int, std::shared_ptr<CsoundWebKit> > browsers_for_handles;

class WebKitCreate : public csound::OpcodeBase<WebKitCreate>
{
public:
    // OUTPUTS
    MYFLT *i_browser_handle;
    // INPUTS
    STRINGDAT *S_return_channel_;
    int init(CSOUND *csound) {
        int result = OK;
        std::shared_ptr<CsoundWebKit> browser = CsoundWebKit::create(csound, S_return_channel_->data);
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
    int init(CSOUND *csound) {
        log(csound, "WebKitOpenUri::init: this: %p\n", this);
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        char *S_window_title = S_window_title_->data;
        char *S_uri = S_uri_->data;
        int i_width = *i_width_;
        int i_height = *i_height_;
        std::shared_ptr<CsoundWebKit> browser = browsers_for_handles[i_browser_handle];
        browser->open(S_window_title, i_width, i_height);
        log(csound, "WebKitOpenUri::init: uri: %s\n", S_uri);
        browser->load_uri(S_uri);
        browser->run_javascript("let x=\"hello\";console.log(x);x;");
        return result;
    }
    int kontrol(CSOUND *csound) {
        int result = OK;
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
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
    int init(CSOUND *csound) {
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        char *S_window_title = S_window_title_->data;
        char *S_html = S_html_->data;
        char *S_base_uri = S_base_uri_->data;
        int i_width = *i_width_;
        int i_height = *i_height_;
        std::shared_ptr<CsoundWebKit> browser = browsers_for_handles[i_browser_handle];
        browser->open(S_window_title, i_width, i_height);
        browser->load_html(S_html, S_base_uri);
        browser->run_javascript("\"hack\";");
        return result;
    }
    int kontrol(CSOUND *csound) {
        int result = OK;
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
        return OK;
    }
};

class WebKitVisibility : public csound::OpcodeBase<WebKitVisibility>
{
public:
    // OUTPUTS
    // INPUTS
    MYFLT *i_browser_handle_;
    MYFLT *i_visible_;
    int init(CSOUND *csound) {
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        std::shared_ptr<CsoundWebKit> browser = browsers_for_handles[i_browser_handle];
        bool visible = *i_visible_;
        browser->set_visible(visible);        
        return result;
    }
};

class WebKitRunJavaScript : public csound::OpcodeBase<WebKitRunJavaScript>
{
public:
    // OUTPUTS
    // INPUTS
    MYFLT *i_browser_handle_;
    STRINGDAT *S_javascript_code_;
    // STATE
    std::shared_ptr<CsoundWebKit> browser;
    int init(CSOUND *csound) {
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        browser = browsers_for_handles[i_browser_handle];
        browser->run_javascript(S_javascript_code_->data);        
        return result;
    }
};

class WebKitInspector : public csound::OpcodeBase<WebKitInspector>
{
public:
    // OUTPUTS
    // INPUTS
    MYFLT *i_browser_handle_;
    int init(CSOUND *csound) {
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        std::shared_ptr<CsoundWebKit> browser = browsers_for_handles[i_browser_handle];
        browser->open_inspector();        
        return result;
    }
};

};

#if 0

static JSCContext *jsc_context = nullptr;
static JSCClass *csound_class = nullptr;
static JSCValue *csound_constructor = nullptr;
static JSCValue *csound_message_callback = nullptr;
static char browser_csound_callback_name[] = "csound_message_callback";

static JSCValue *csound_constructor_callback(gpointer data) {
    std::printf("csound_constructor_callback: data: %p\n", data);
    JSCValue *new_instance = jsc_value_new_object(jsc_context, NULL, csound_class);
    return new_instance;
}

static int Csound_Cleanup(JSCValue *instance, gpointer user_data) {
    int result = csound_->Cleanup();
    std::printf("Csound_Cleanup: result: %d\n", result);
    return result;
}

static int Csound_CompileCsd(JSCValue *instance, char *csd_filepath, gpointer user_data) {
    // std::printf("Csound_CompileCsd: csd_filepath: %s\n", csd_filepath);
    int result = csound_->CompileCsd(csd_filepath);
    std::printf("Csound_CompileCsd: result: %d\n", result);
    return result;
}

static int Csound_CompileCsdText(JSCValue *instance, char *csd_text, gpointer user_data) {
    // std::printf("Csound_CompileCsdText: csd_text: %s\n", csd_text);
    int result = csound_->CompileCsdText(csd_text);
    std::printf("Csound_CompileCsdText: result: %d\n", result);
    return result;
}

static void Csound_CreateMessageBuffer(JSCValue *instance, int to_stdout, gpointer user_data) {
    std::printf("Csound_CreateMessageBuffer: to_stdout: %d\n", to_stdout);
    csound_->CreateMessageBuffer(to_stdout);
}

static int Csound_GetMessageCnt(JSCValue *instance, gpointer user_data) {
    int result = csound_->GetMessageCnt();
    std::printf("Csound_GetMessageCnt: result: %d\n", result);
    return result;
}

static int Csound_GetVersion(JSCValue *instance, gpointer user_data) {
    int result = csound_->GetVersion();
    std::printf("Csound_GetVersion: result: %d\n", result);
    return result;
}

static JSCValue *Csound_GetFirstMessage(JSCValue *instance, gpointer user_data) {
    const char *message = csound_->GetFirstMessage();
    std::printf("Csound_GetFirstMessage: message: %s\n", message);
    JSCValue *jsc_message = jsc_value_new_string(jsc_context, message);
    g_object_ref_sink(jsc_message);
    return jsc_message;
}

static void Csound_Perform(JSCValue *instance, gpointer user_data) {
    std::printf("Csound_Perform\n");
    csound_->Perform();
}

static void Csound_PopFirstMessage(JSCValue *instance, gpointer user_data) {
    std::printf("Csound_PopFirstMessage\n");
    csound_->PopFirstMessage();
}

static int Csound_Render(JSCValue *instance, char *csd_text, gpointer user_data) {
    // std::printf("Csound_Render: csd_text: %s\n", csd_text);
    int result = csound_->CompileCsdText(csd_text);
    result = result + csound_->Start();
    result = result + csound_->Perform();
    std::printf("Csound_Render: result: %d\n", result);
    return result;
}

static void Csound_Reset(JSCValue *instance, gpointer user_data) {
    std::printf("Csound_Reset\n");
    csound_->Reset();
}

static void Csound_SetControlChannel(JSCValue *instance, char *name, double value, gpointer user_data) {
    // std::printf("Csound_SetControlChannel: name: %s value: %f\n", name, value);
    csound_->SetControlChannel(name, value);
}

static void Csound_SetMessageCallback(JSCValue *instance, JSCValue *csound_message_callback_, gpointer user_data) {
    std::printf("Csound_SetMessageCallback: csound_message_callback: %p\n", csound_message_callback_);
    csound_message_callback = csound_message_callback_;
    g_object_ref(csound_message_callback);
    std::printf("Csound_SetMessageCallback: csound_message_callback type: %s %s\n", G_OBJECT_TYPE_NAME(csound_message_callback), jsc_value_to_string(csound_message_callback));
    csound_->SetMessageCallback(native_csound_message_callback);
}

static void Csound_Start(JSCValue *instance, gpointer user_data) {
    std::printf("Csound_Start\n");
    csound_->Start();
}

static void Csound_Stop(JSCValue *instance, gpointer user_data) {
    std::printf("Csound_Stop");
    csound_->Stop();
    csound_->Join();
}

/**
* Apparently, only at this time is it possible for WebExtensions to
* inject native code into WebKit's JavaScript context.
*/
static void
window_object_cleared_callback (WebKitScriptWorld *world,
                                WebKitWebPage     *web_page,
                                WebKitFrame       *frame,
                                GVariant          *user_data)
{
    jsc_context = webkit_frame_get_jsc_context_for_script_world(frame, world);
    JSCValue *js_global_object = jsc_context_get_global_object(jsc_context);
    csound_.reset(new CsoundThreaded());
    JSCValue *gjs_csound_hello = jsc_value_new_function(jsc_context,
                                 NULL,
                                 G_CALLBACK(on_gjs_csound_hello),
                                 NULL,
                                 NULL,
                                 G_TYPE_NONE,
                                 0);
    jsc_context_set_value (jsc_context,
                           "gjs_csound_hello",
                           gjs_csound_hello);
    // First the class must be registered.
    csound_class = jsc_context_register_class(jsc_context, "Csound", NULL, NULL, NULL);
    std::printf("window_object_cleared_callback: defined Csound class: %p name: %s\n",
                csound_class, jsc_class_get_name(csound_class));
    // Then its constructor must be defined.
    csound_constructor = jsc_class_add_constructor (csound_class,
                         "Csound",
                         GCallback(csound_constructor_callback),
                         user_data,
                         destroy_notify_,
                         G_TYPE_OBJECT,
                         0);
    // jsc_class_add_method (JSCClass *jsc_class,
    // const char *name,
    // GCallback callback,
    // gpointer user_data,
    // GDestroyNotify destroy_notify,
    // GType return_type,
    // guint n_params,
    // ...);
    // NOTE: Elementary types can be returned as such.
    jsc_class_add_method(csound_class,
                         "GetVersion",
                         G_CALLBACK(Csound_GetVersion),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         0);
    jsc_class_add_method(csound_class,
                         "Cleanup",
                         G_CALLBACK(Csound_Cleanup),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         0);
    jsc_class_add_method(csound_class,
                         "CompileCsd",
                         G_CALLBACK(Csound_CompileCsd),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         1,
                         G_TYPE_STRING);
    jsc_class_add_method(csound_class,
                         "CompileCsdText",
                         G_CALLBACK(Csound_CompileCsdText),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         1,
                         G_TYPE_STRING);
    jsc_class_add_method(csound_class,
                         "CreateMessageBuffer",
                         G_CALLBACK(Csound_CreateMessageBuffer),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         1,
                         G_TYPE_INT);
    jsc_class_add_method(csound_class,
                         "GetFirstMessage",
                         G_CALLBACK(Csound_GetFirstMessage),
                         user_data,
                         NULL,
                         JSC_TYPE_VALUE,
                         0);
    jsc_class_add_method(csound_class,
                         "GetMessageCnt",
                         G_CALLBACK(Csound_GetMessageCnt),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         0);
    jsc_class_add_method(csound_class,
                         "Perform",
                         G_CALLBACK(Csound_Perform),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         0);
    jsc_class_add_method(csound_class,
                         "PopFirstMessage",
                         G_CALLBACK(Csound_PopFirstMessage),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         0);
    jsc_class_add_method(csound_class,
                         "Render",
                         G_CALLBACK(Csound_Render),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         1,
                         G_TYPE_STRING);
    jsc_class_add_method(csound_class,
                         "Reset",
                         G_CALLBACK(Csound_Reset),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         0);
    jsc_class_add_method(csound_class,
                         "SetControlChannel",
                         G_CALLBACK(Csound_SetControlChannel),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         2,
                         G_TYPE_STRING,
                         G_TYPE_DOUBLE);
    jsc_class_add_method(csound_class,
                         "SetMessageCallback",
                         G_CALLBACK(Csound_SetMessageCallback),
                         user_data,
                         NULL,
                         G_TYPE_NONE,
                         1,
                         JSC_TYPE_VALUE);
    jsc_class_add_method(csound_class,
                         "Start",
                         G_CALLBACK(Csound_Start),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         0);
    jsc_class_add_method(csound_class,
                         "Stop",
                         G_CALLBACK(Csound_Stop),
                         user_data,
                         NULL,
                         G_TYPE_INT,
                         1,
                         G_TYPE_STRING);

    // Then the constructor must be added to the window object.
    jsc_context_set_value (jsc_context,
                           "Csound",
                           csound_constructor);
}

/**
 * Loads this extension when the page is created; this is required before it is 
 * possible to inject native code into the page's JavaScript context.
 */
void webkit_web_extension_initialize_with_user_data(WebKitWebExtension *extension,
        GVariant *user_data)
{
    g_variant_ref_sink(user_data);
    std::fprintf(stderr, "webkit_web_extension_initialize_with_user_data: %p user_data: %s (0x%lx)\n",
                extension,
                g_variant_print(user_data, TRUE),
                g_variant_get_uint64(user_data));
    g_signal_connect (webkit_script_world_get_default (),
                      "window-object-cleared",
                      G_CALLBACK (window_object_cleared_callback),
                      user_data);
}

#endif

/** 
 * Here is the syntax for all the WebKit opcodes:
 *
 * i_webkit_handle webkit_create S_return_channel
 * webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height
 * webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height
 * webkit_visibility i_webkit_handle, i_visible
 * webkit_run_javascript i_webkit_handle, S_javascript_code
 * webkit_inspector i_webkit_handle
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
                                          (char *)"S",
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
        status += csound->AppendOpcode(csound,
                                          (char *)"webkit_visibility",
                                          sizeof(webkit_opcodes::WebKitVisibility),
                                          0,
                                          1,
                                          (char *)"",
                                          (char *)"ii",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitVisibility::init_,
                                          (int (*)(CSOUND*,void*)) 0,
                                          (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                                          (char *)"webkit_run_javascript",
                                          sizeof(webkit_opcodes::WebKitRunJavaScript),
                                          0,
                                          1,
                                          (char *)"",
                                          (char *)"ii",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitRunJavaScript::init_,
                                          (int (*)(CSOUND*,void*)) 0,
                                          (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                                          (char *)"webkit_inspector",
                                          sizeof(webkit_opcodes::WebKitInspector),
                                          0,
                                          1,
                                          (char *)"",
                                          (char *)"i",
                                          (int (*)(CSOUND*,void*)) webkit_opcodes::WebKitInspector::init_,
                                          (int (*)(CSOUND*,void*)) 0,
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

#if 1

/**
 * For developing and testing...
 */
int main(int argc, char* argv[])
{
    Csound csound;
    std::fprintf(stderr, "main...\n");
    auto browser = std::move(webkit_opcodes::CsoundWebKit::create(csound.GetCsound(), "webkit_returns"));
    browser->open("Fake Title", 1000, 600);
    browser->load_uri("file:///home/mkg/csound-examples/docs/message.html");
    bool blocking = false;
    browser->run_javascript("let x = \"boo\";x;");
    while (true) {
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }
    }
    std::fprintf(stderr, "Exiting...\n");
    return 0;
}

#endif
