#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <csound/csound.hpp>
#include <csound/OpcodeBase.hpp>
#include <cstdio>
#include <string>
#include <map>
#include <memory>

// From: https://wiki.gnome.org/Projects/WebKitGtk/ProgrammingGuide/Tutorial

namespace webkit_opcodes {
  
static void destroy_window_callback(GtkWidget* widget, GtkWidget* window)
{
    std::fprintf(stderr, "destroy_window_callback...\n");
}

static gboolean close_webview_callback(WebKitWebView* webView, GtkWidget* window)
{
    std::fprintf(stderr, "close_webview_callback...\n");
    return true;
}

struct CsoundWebKit {
    CSOUND *csound;
    GtkWidget *main_window;
    WebKitWebView *web_view;
    WebKitSettings *webkit_settings;
    WebKitWebInspector *webkit_inspector;
    bool diagnostics_enabled;
    CsoundWebKit(CSOUND *csound_) {
        csound = csound_;
        // Initialize GTK+
        gtk_init(nullptr, nullptr);
        diagnostics_enabled = true;
    }
    static std::unique_ptr<CsoundWebKit> create(CSOUND *csound_) {
        std::unique_ptr<CsoundWebKit> result(new CsoundWebKit(csound_));
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
        webkit_settings_set_media_playback_requires_user_gesture(webkit_settings, true);
        webkit_settings_set_enable_write_console_messages_to_stdout(webkit_settings, true);
        webkit_settings_set_enable_media_stream(webkit_settings, true);
        webkit_settings_set_enable_mediasource(webkit_settings, true);
        webkit_settings_set_enable_media_capabilities(webkit_settings, true);
        // TODO: See if these two are really a good idea.
        webkit_settings_set_allow_file_access_from_file_urls(webkit_settings, true);
        webkit_settings_set_allow_universal_access_from_file_urls(webkit_settings, true);
        gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(web_view));
        // Set up callbacks so that if either the main window or the browser instance is
        // closed, the program will exit
        g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);
        g_signal_connect(web_view, "close", G_CALLBACK(close_webview_callback), main_window);
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
    static void run_javascript_callback(GObject *object, GAsyncResult *result, gpointer user_data) {
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
        if (jsc_value_is_string (value)) {
            JSCException *exception;
            gchar *str_value = jsc_value_to_string(value);
            exception = jsc_context_get_exception(jsc_value_get_context(value));
            if (exception) {
                g_warning("Error running javascript: %s", jsc_exception_get_message(exception));
            } else {
                g_print ("Script result: %s\n", str_value);
            }
            g_free(str_value);
        } else {
            g_warning ("Error running javascript: unexpected return value");
        }
        webkit_javascript_result_unref(js_result);
    }
    virtual int run_javascript(const char *javascript_code) {
        //auto code = g_strdup(javascript_code);
        std::fprintf(stderr, "run_javascript %s\n", javascript_code);
        int result = OK;
        webkit_web_view_run_javascript(web_view, javascript_code, nullptr, run_javascript_callback, this);
        //g_free(code);
        return result;
    }
    virtual void open_inspector() {
        webkit_inspector = webkit_web_view_get_inspector(web_view);
        webkit_web_inspector_show(webkit_inspector);
    }
};    

static std::map<int, std::shared_ptr<CsoundWebKit> > browsers_for_handles;

/** 
 * Here is the syntax for all the WebKit opcodes:
 *
 * i_webkit_handle webkit_create
 * webkit_open_uri i_webkit_handle, S_window_title, S_uri, i_width, i_height
 * webkit_open_html i_webkit_handle, S_window_title, S_html, S_base_uri, i_width, i_height
 * webkit_visibility i_webkit_handle, i_visible
 * webkit_run_javascript i_webkit_handle, S_javascript_code, S_return_channel
 * webkit_inspector i_webkit_handle
 *
 * In addition, each Web page opened by these opcodes has a JavaScript 
 * interface to the invoking instance of Csound.
 */

class WebKitCreate : public csound::OpcodeBase<WebKitCreate>
{
public:
    // OUTPUTS
    MYFLT *i_browser_handle;
    int init(CSOUND *csound) {
        int result = OK;
        std::shared_ptr<CsoundWebKit> browser = CsoundWebKit::create(csound);
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
        int result = OK;
        int i_browser_handle = *i_browser_handle_;
        char *S_window_title = S_window_title_->data;
        char *S_uri = S_uri_->data;
        int i_width = *i_width_;
        int i_height = *i_height_;
        std::shared_ptr<CsoundWebKit> browser = browsers_for_handles[i_browser_handle];
        browser->open(S_window_title, i_width, i_height);
        browser->load_uri(S_uri);
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

class WebKitOpenInspector : public csound::OpcodeBase<WebKitOpenInspector>
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

int main(int argc, char* argv[])
{
    Csound csound;
    std::fprintf(stderr, "main...\n");
    auto browser = std::move(webkit_opcodes::CsoundWebKit::create(csound.GetCsound()));
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
