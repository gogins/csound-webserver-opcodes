#include <future>
#include <atomic>
#include <chrono>
#include <OpcodeBase.hpp>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <queue>
#include <thread>
#include <map>
#include <memory>
#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>

extern "C" {
    typedef int (*csoundCompileCsdText_t)(CSOUND *, const char *);
    typedef int (*csoundCompileOrc_t)(CSOUND *, const char *);
    typedef MYFLT(*csoundEvalCode_t)(CSOUND *, const char *);
    typedef MYFLT(*csoundGet0dBFS_t)(CSOUND *);
    typedef void (*csoundGetAudioChannel_t)(CSOUND *, const char *, MYFLT *);
    typedef MYFLT(*csoundGetControlChannel_t)(CSOUND *, const char *, int *);
    typedef int (*csoundGetDebug_t)(CSOUND *);
    typedef uint32_t (*csoundGetKsmps_t)(CSOUND *);
    typedef uint32_t (*csoundGetNchnls_t)(CSOUND *);
    typedef uint32_t (*csoundGetNchnlsInput_t)(CSOUND *);
    typedef MYFLT(*csoundGetScoreOffsetSeconds_t)(CSOUND *);
    typedef MYFLT(*csoundGetScoreTime_t)(CSOUND *);
    typedef MYFLT(*csoundGetSr_t)(CSOUND *);
    typedef void (*csoundGetStringChannel_t)(CSOUND *, const char *, char *);
    typedef void (*csoundInputMessage_t)(CSOUND *, const char *);
    typedef int (*csoundIsScorePending_t)(CSOUND *);
    typedef void (*csoundMessage_t)(CSOUND *, const char *, ...);
    typedef int (*csoundReadScore_t)(CSOUND *, const char *);
    typedef void (*csoundRewindScore_t)(CSOUND *);
    typedef int (*csoundScoreEvent_t)(CSOUND *,char, const MYFLT *, long);
    typedef void (*csoundSetControlChannel_t)(CSOUND *, const char *, MYFLT);
    typedef void (*csoundSetDebug_t)(CSOUND *, int);
    typedef void (*csoundSetMessageCallback_t)(CSOUND *, void (*)(CSOUND *, int, const char *, va_list));
    typedef void (*csoundSetScoreOffsetSeconds_t)(CSOUND *, MYFLT);
    typedef void (*csoundSetScorePending_t)(CSOUND *, int);
    typedef void (*csoundSetStringChannel_t)(CSOUND *, const char *, char *);
    typedef MYFLT(*csoundTableGet_t)(CSOUND *, int, int);
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
    csoundSetMessageCallback_t csoundSetMessageCallback_;
    csoundSetScoreOffsetSeconds_t csoundSetScoreOffsetSeconds_;
    csoundSetScorePending_t csoundSetScorePending_;
    csoundSetStringChannel_t csoundSetStringChannel_;
    csoundTableGet_t csoundTableGet_;
    csoundTableLength_t csoundTableLength_;
    csoundTableSet_t csoundTableSet_;
};

static bool diagnostics_enabled = true;

namespace csound_webserver {


    static std::mutex &get_mutex() {
        static std::mutex mutex_;
        return mutex_;
    }

    class CsoundWebServer;

    static std::string default_browser() {
#if defined(_WIN64)
        return "start";
#endif
#if defined(__linux__)
        return "xdg-open";
#endif
#if defined(macintosh)
        return "open";
#endif
        return "Shell open command for browser is undefined.\n";
    }

    template<typename O> class heep_object_manager_t {
        private:
            std::map<CSOUND *, std::vector<O*>> objects_;
            std::recursive_mutex mutex;
            heep_object_manager_t() {};
            ~heep_object_manager_t() {};
        public:
            static heep_object_manager_t &instance() {
                static heep_object_manager_t singleton;
                return singleton;
            }
            std::map<CSOUND *, std::vector<O*>> &objects() {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                return objects_;
            }
            /**
             * Returns a list of pointers to all objects allocated for this
             * instance of Csound.
             */
            std::vector<O*> &objects_for_csound(CSOUND *csound) {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                auto &objects_for_csound_ = objects()[csound];
                return objects_for_csound_;
            }
            /**
             * Returns the handle for the object; if the object has not yet been
             * stored, inserts it into the list of object pointers for this
             * instance of Csound; otherwise, returns the handle of the stored
             * object pointer.
             */
            int handle_for_object(CSOUND *csound, O *object) {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                auto &objects_for_csound_ = objects_for_csound(csound);
                auto iterator = std::find(objects_for_csound_.begin(), objects_for_csound_.end(), object);
                if (iterator == objects_for_csound_.end()) {
                    int handle = objects_for_csound_.size();
                    objects_for_csound_.push_back(object);
                    if (diagnostics_enabled) std::fprintf(stderr, "heep_object_manager_t::handle_for_object %p: new object handle: %d (of %ld)\n", object, handle, objects_for_csound_.size());
                    return handle;
                } else {
                    int handle = static_cast<int>(iterator - objects_for_csound_.begin());
                    if (diagnostics_enabled) std::fprintf(stderr, "heep_object_manager_t::handle_for_object: existing object handle: %d\n", handle);
                    return handle;
                }
            }
            /**
             * Returns the object pointer for the handle;
             * if the object pointer has not been stored by
             * handle, returns a null pointer.
             */
            O *object_for_handle(CSOUND *csound, int handle) {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                auto &objects_for_csound_ = objects_for_csound(csound);
                if (handle >= objects_for_csound_.size()) {
                    return nullptr;
                }
                O *object = objects_for_csound_[handle];
                if (diagnostics_enabled) std::fprintf(stderr, "heep_object_manager_t::object_for_handle: %p %d (of %ld)\n", object, handle, objects_for_csound_.size());
                return object;
            }
            /**
             * First destroys all objects created by the calling
             * instance of Csound, then destroys the list of
             * object pointers for this instance of Csound.
             */
            void module_destroy(CSOUND *csound) {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                auto &objects_for_csound_ = objects_for_csound(csound);
                for(int i = 0, n = objects_for_csound_.size(); i < n; ++i) {
                    delete objects_for_csound_[i];
                    objects_for_csound_[i] = nullptr;
                }
                objects_for_csound_.clear();
                objects().erase(csound);
            }
    };

    /**
     * Enqueues messages for a server-sent event channel.
     */
    class EventDispatcher {
        public:
            EventDispatcher(const std::string &channel_name_) : channel_name(channel_name_) {
            }
            void wait_event(httplib::DataSink *sink) {
                std::unique_lock<std::mutex> lk(m_);
                int id = id_;
                cv_.wait(lk, [&] { return cid_ == id; });
                if (sink->is_writable()) {
                    sink->write(message_.data(), message_.size());
                }
            }
            void send_event(const std::string &message) {
                std::lock_guard<std::mutex> lk(m_);
                cid_ = id_++;
                message_ = message;
                cv_.notify_all();
            }
        private:
            std::string channel_name;
            std::mutex m_;
            std::condition_variable cv_;
            std::atomic_int id_{0};
            std::atomic_int cid_{-1};
            std::string message_;
    };

    /**
    * A thread-safe queue, or first-in first-out (FIFO) queue, implemented using
    * only the standard C++11 library. The Data should be a simple type, such as
    * a pointer.
    */
    template<typename Data>
    class concurrent_queue {
        private:
            std::queue<Data> queue_;
            std::mutex mutex_;
            std::condition_variable condition_variable_;
        public:
            void push(Data const& data) {
                std::unique_lock<std::mutex> lock(mutex_);
                queue_.push(data);
                lock.unlock();
                condition_variable_.notify_one();
            }
            bool empty() const {
                std::unique_lock<std::mutex> lock(mutex_);
                return queue_.empty();
            }
            bool try_pop(Data& popped_value) {
                std::unique_lock<std::mutex> lock(mutex_);
                if (queue_.empty()) {
                    return false;
                }
                popped_value = queue_.front();
                queue_.pop();
                return true;
            }
            void wait_and_pop(Data& popped_value) {
                std::unique_lock<std::mutex> lock(mutex_);
                while(queue_.empty()) {
                    condition_variable_.wait(lock);
                }
                popped_value = queue_.front();
                queue_.pop();
            }
    };

    /**
     * First the Csound method is called, then this function creates the
     * JSON-RPC response from the JSON-RPC request and the return value of the
     * Csound method.
     */
    template<typename T>
    void create_json_response(const nlohmann::json &json_request, httplib::Response &response, const T &return_value_) {
        nlohmann::json json_response;
        json_response["jsonrpc"] = "2.0";
        json_response["id"] = json_request["id"];
        json_response["method"] = json_request["method"];
        if (diagnostics_enabled) std::fprintf(stderr, "json_request: %s\n", json_request.dump().c_str());
        nlohmann::json return_value(return_value_);
        json_response["result"] = return_value;
        if (diagnostics_enabled) std::fprintf(stderr, "json_response: %s\n", json_response.dump().c_str());
        response.set_content(json_response.dump(), "application/json");
    }

    struct CsoundWebServer {
        // All resources are served relative to the server's base directory.
        std::string base_directory;
        // All resources are identified by appending their path to the
        // origin.
        std::string origin;
        int port;
        httplib::Server server;
        std::thread *listener_thread;
        CSOUND *csound;
        void *library_handle;
        std::string csound_message_callback_channel;
        std::map<std::string, concurrent_queue<char *>> event_queues_for_event_channels;
        CsoundWebServer() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer...\n");
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer.\n");
        }
        virtual ~CsoundWebServer() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer...\n");
            server.stop();
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer.\n");
        }
        virtual void listen() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen...\n");
            // Was "0.0.0.0" for all interfaces -- for security we limit this
            // to localhost.
            server.listen("localhost", port);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen.\n");
        }
        bool &interface_initialized() {
            static bool interface_initialized_ = false;
            return interface_initialized_;
        }
        virtual void create_(CSOUND *csound_, const std::string &base_directory_, int port_) {
            csound = csound_;
            // A null library path indicates that symbols should be resolved
            // from the running program (Csound usually) and its loaded
            // dependencies (which must include the Csound library).
            int result = csound->OpenLibrary(&library_handle, nullptr);
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
                csoundSetMessageCallback_ = (csoundSetMessageCallback_t) csound_->GetLibrarySymbol(library_handle, "csoundSetMessageCallback");
                csoundSetScoreOffsetSeconds_ = (csoundSetScoreOffsetSeconds_t) csound_->GetLibrarySymbol(library_handle, "csoundSetScoreOffsetSeconds");
                csoundSetScorePending_ = (csoundSetScorePending_t) csound_->GetLibrarySymbol(library_handle, "csoundSetScorePending");
                csoundTableGet_ = (csoundTableGet_t) csound_->GetLibrarySymbol(library_handle, "csoundTableGet");
                csoundTableLength_ = (csoundTableLength_t) csound_->GetLibrarySymbol(library_handle, "csoundTableLength");
                csoundTableSet_ = (csoundTableSet_t) csound_->GetLibrarySymbol(library_handle, "csoundTableSet");
                interface_initialized() = true;
            }
            base_directory = base_directory_;
            if (port != -1) {
                port = port_;
            } else {
                port = 8080;
            }
            if (diagnostics_enabled) {
                server.set_logger([](const auto& req, const auto& res) {
                    std::fprintf(stderr, "Request:  method: %s path: %s body: %s\n", req.method.c_str(), req.path.c_str(), req.body.c_str());
                    std::fprintf(stderr, "Response: reason: %s body: %s\n", res.reason.c_str(), res.body.c_str());
                });
            }
            server.set_base_dir(base_directory.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create: base_directory: %s\n", base_directory.c_str());
            origin = "http://localhost:" + std::to_string(port);
            csound->Message(csound_, "CsoundWebServer: origin: %s\n", origin.c_str());
            // Add JSON-RPC skeletons... these are just HTTP APIs that follow
            // the JSON-RPC 2.0 wire protocol, so there is no need for a
            // second port to carry the RPCs.
            server.Post("/CompileCsdText", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileCsdText...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto csd_text = json_request["params"]["csd_text"].get<std::string>();
                auto result = csoundCompileCsdText_(csound, csd_text.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileCsdText: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/CompileOrc", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileOrc...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto orc_code = json_request["params"]["orc_code"].get<std::string>();
                auto result = csoundCompileOrc_(csound, orc_code.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileOrc: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/EvalCode", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode...\n");
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: request.body: %s\n", request.body.c_str());
                auto json_request = nlohmann::json::parse(request.body);
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: json_request: %s\n", json_request.dump().c_str());
                auto orc_code = json_request["params"]["orc_code"].get<std::string>();
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: orc_code: %s\n", orc_code.c_str());
                auto result = csoundEvalCode_(csound, orc_code.c_str());
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: result: %f\n", result);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/Get0dBFS", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/Get0dBFS...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGet0dBFS_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/Get0dBFS: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetAudioChannel", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetAudioChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto ksmps = csound->GetKsmps(csound);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();
                std::vector<MYFLT> result;
                result.resize(ksmps);
                MYFLT *buffer = &result.front();
                csoundGetAudioChannel_(csound, channel_name.c_str(), buffer);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetAudioChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetControlChannel", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();
                int err;
                auto result = csoundGetControlChannel_(csound, channel_name.c_str(), &err);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetControlChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                if (err == OK) {
                    response.status = 200;
                } else {
                    response.status = 404;
                }
            });
            server.Post("/GetDebug", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetDebug...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetDebug_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetKsmps", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetKsmps...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetKsmps_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetKsmps: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetNchnls", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnls...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetNchnls_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnls: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetNchnlsInput", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnlsInput...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetNchnlsInput_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnlsInput: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetScoreOffsetSeconds", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreOffsetSeconds...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetScoreOffsetSeconds_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreOffsetSeconds: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetScoreTime", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreTime...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetScoreTime_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreTime: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/GetSr", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetSr...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundGetSr_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetSr: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/InputMessage", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/InputMessage...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto sco_code = json_request["params"]["sco_code"].get<std::string>();
                csoundInputMessage_(csound, sco_code.c_str());
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/InputMessage: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/IsScorePending", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/IsScorePending...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = csoundIsScorePending_(csound);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/IsScorePending: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/Message", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/Message...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto message = json_request["params"]["message"].get<std::string>();
                csoundMessage_(csound, message.c_str());
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/Message: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/ReadScore", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/ReadScore...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto sco_code = json_request["params"]["sco_code"].get<std::string>();
                auto result = csoundReadScore_(csound, sco_code.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/ReadScore: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/RewindScore", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/RewindScore...\n");
                auto json_request = nlohmann::json::parse(request.body);
                csoundRewindScore_(csound);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/RewindScore: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/ScoreEvent", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/ScoreEvent...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto opcode_code = json_request["params"]["opcode_code"].get<char>();
                auto pfields = json_request["params"]["pfields"].get<std::vector<MYFLT> >();
                csoundScoreEvent_(csound, opcode_code, &pfields.front(), pfields.size());
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/ScoreEvent: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/SetControlChannel", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();
                auto channel_value = json_request["params"]["channel_value"].get<MYFLT>();
                csoundSetControlChannel_(csound, channel_name.c_str(), channel_value);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/SetDebug", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetDebug...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto enabled = json_request["params"]["enabled"].get<int>();
                csoundSetDebug_(csound, enabled);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetDebug: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/SetScoreOffsetSeconds", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScoreOffsetSeconds...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto score_time = json_request["params"]["score_time"].get<MYFLT>();
                csoundSetScoreOffsetSeconds_(csound, score_time);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScoreOffsetSeconds: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/SetScorePending", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScorePending...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto pending = json_request["params"]["pending"].get<int>();
                csoundSetScorePending_(csound, pending);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScorePending: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/SetStringChannel", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();
                auto channel_value = json_request["params"]["channel_value"].get<std::string>();
                csoundSetStringChannel_(csound, channel_name.c_str(), const_cast<char *>(channel_value.c_str()));
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetStringChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/TableGet", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableGet...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();
                auto index = json_request["params"]["index"].get<int>();
                MYFLT result = csoundTableGet_(csound, table_number, index);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableGet: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/TableLength", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableLength...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();
                int result = csoundTableLength_(csound, table_number);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableLength: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            server.Post("/TableSet", [=](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableSet...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();
                auto index = json_request["params"]["table_number"].get<int>();
                auto  value = json_request["params"]["table_number"].get<MYFLT>();
                csoundTableSet_(csound, table_number, index, value);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableSet: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 200;
            });
            // ...and start listening in a separate thread.
            listener_thread = new std::thread(&CsoundWebServer::listen, this);
        }
        static CsoundWebServer *create(CSOUND *csound_, const std::string &base_directory_, int port_) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create...\n");
            auto webserver = new CsoundWebServer();
            webserver->create_(csound_, base_directory_, port_);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create.\n");
            return webserver;
        }
        virtual void open_resource(const std::string &resource, const std::string &browser) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_resource...\n");
            // Origin:
            // `http://host:port`
            // Complete URL:
            // `http://host:port[[/resource_path][.extension]]`
            // NOTE: The _filesystem_ base directory is mapped to: `origin/`.
            std::string url;
            if (resource.find("http://") == 0) {
                url = resource;
            } else if (resource.find("https://") == 0) {
                url = resource;
            } else {
                url = origin + "/" + resource;
            }
            std::string command = browser + " " + url;
            std::system(command.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_resource: command: %s\n", command.c_str());
        }
        virtual void open_html(const std::string &html_text, const std::string &browser) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_html...\n");
            // Origin:
            // `http://host:port`
            // Complete URL:
            // `http://host:port[[/resource_path][.extension]]`
            // NOTE: The _filesystem_ base directory is mapped to: `origin/`.
            // Creaate a temporary .html file from the html text in the base directory.
            char html_path[0x500];
            {
                std::lock_guard lock(get_mutex());
                std::mt19937 mersenne_twister;
                unsigned int seed_ = std::time(nullptr);
                mersenne_twister.seed(seed_);
                std::snprintf(html_path, 0x500, "csound_webserver_%lx.html", mersenne_twister());
                std::string filepath = base_directory + "/" + html_path;
                auto file_ = fopen(filepath.c_str(), "w+");
                std::fwrite(html_text.c_str(), html_text.length(), sizeof(html_text[0]), file_);
                std::fclose(file_);
            }
            std::string url = origin + "/" + html_path;
            std::string command = browser + " " + url;
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::html_text: command: %s\n", command.c_str());
            std::system(command.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::html_text.\n");
        }
        virtual void send_message(const std::string &channel_name, const std::string &message) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::send_message...\n");
            event_queues_for_event_channels[channel_name].push(strdup(message.c_str()));
            auto &event_queue = event_queues_for_event_channels.at(channel_name);
            server.Get("/" + channel_name, 
                [&event_queue, channel_name](const httplib::Request & /*request*/, httplib::Response &response) {
                    if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::send_message Get: channel_name: %s\n", channel_name.c_str());
                    response.set_chunked_content_provider("text/event-stream",
                    [&event_queue](size_t /*offset*/, httplib::DataSink &sink) {
                        char *message = nullptr;
                        event_queue.wait_and_pop(message);
                        if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::send_message: popped: %s\n", message);
                        if (sink.is_writable()) {
                            int result = 0;
                            char header[] = "event: message\ndata:";
                            result = sink.write(header, std::strlen(header));
                            result = sink.write(message, std::strlen(message));
                            char end_of_data[] = "\n\n";
                            result = sink.write(end_of_data, std::strlen(end_of_data));
                            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::send_message: wrote: %s result: %d\n", message, result);
                            std::free(message);
                        }
                        return true;
                    });          
                    response.status = 200;
                });
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::send_message.\n");
        }
        virtual void message_callback(CSOUND *csound, int attr, const char *format, va_list valist) {
            char message[0x2000];
            std::vsnprintf(message, 0x2000, format, valist);
            send_message(csound_message_callback_channel, message);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::message_callback: %s\n", message);
        }
        static void message_callback_(CSOUND *csound, int attr, const char *format, va_list valist) {
            CsoundWebServer *web_server_ptr = nullptr;
            web_server_ptr = (CsoundWebServer *)csound::QueryGlobalPointer(csound, "CsoundWebServer", web_server_ptr);
            web_server_ptr->message_callback(csound, attr, format, valist);
        }
        virtual void set_message_callback(const std::string &channel_name) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::set_message_callback: csound: %p channel_name: %s\n", csound, channel_name.c_str());
            csound_message_callback_channel = channel_name;
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::set_message_callback: CreateGlobalPointer...\n");
            csound::CreateGlobalPointer(csound, "CsoundWebServer", this);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::set_message_callback: CreateGlobalPointer.\n");
            csoundSetMessageCallback_(csound, &CsoundWebServer::message_callback_);
            send_message(channel_name, channel_name);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::set_message_callback: this: %p message_callback_: %p\n", this, &CsoundWebServer::message_callback_);
        }
    };

    class csound_webserver_create : public csound::OpcodeBase<csound_webserver_create> {
        public:
            // OUTPUTS
            MYFLT *i_server_handle;
            // INPUTS
            STRINGDAT *S_base_uri;
            MYFLT *i_port;
            MYFLT *i_diagnostics_enabled;
            int init(CSOUND *csound) {
                int result = OK;
                std::string base_uri_ = S_base_uri->data;
                int port = *i_port;
                diagnostics_enabled = *i_diagnostics_enabled;
                if (diagnostics_enabled) {
                    std::setbuf(stderr, nullptr);
                    csound->Message(csound, "csound_webserver_create: diagnostics have been enabled.\n");
                }
                auto server = CsoundWebServer::create(csound, base_uri_, port);
                int handle = heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().handle_for_object(csound, server);
                *i_server_handle = static_cast<MYFLT>(handle);
                return result;
            }
    };

    class csound_webserver_open_resource : public csound::OpcodeBase<csound_webserver_open_resource> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_resource;
            STRINGDAT *S_browser;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_resource::init: this: %p csound: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string resource = S_resource->data;
                std::string browser = S_browser->data;
                server = heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().object_for_handle(csound, i_server_handle);
                server->open_resource(resource, browser);
                log(csound, "csound_webserver_open_resource::init.\n");
                return result;
            }
    };

    class csound_webserver_open_html : public csound::OpcodeBase<csound_webserver_open_html> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_html_text_;
            STRINGDAT *S_browser;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_html::init: this: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string html_text = S_html_text_->data;
                std::string browser = S_browser->data;
                server = heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().object_for_handle(csound, i_server_handle);
                server->open_html(html_text, browser);
                log(csound, "csound_webserver_open_html::init.\n");
                return result;
            }
    };

    class csound_webserver_send : public csound::OpcodeBase<csound_webserver_send> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_channel_name;
            STRINGDAT *S_message;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_send::init: this: %p csound: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string channel_name = S_channel_name->data;
                std::string message = S_message->data;
                server = heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().object_for_handle(csound, i_server_handle);
                server->send_message(channel_name, message);
                log(csound, "csound_webserver_send::init.\n");
                return result;
            }
    };

    class csound_webserver_set_message_callback : public csound::OpcodeBase<csound_webserver_set_message_callback> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_channel_name;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_set_message_callback::init: this: %p csound: %p\n", this, csound);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string channel_name = S_channel_name->data;
                server = heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().object_for_handle(csound, i_server_handle);
                log(csound, "csound_webserver_set_message_callback::init: server: %p channel_name: %s.\n", server, channel_name.c_str());
                server->set_message_callback(channel_name);
                log(csound, "csound_webserver_set_message_callback::init (2): channel_name: %s.\n", channel_name.c_str());
                return result;
            }
    };

};

/**
 * i_webserver_handle webserver_create S_base_uri, i_port [, i_diagnostics_enabled]
 * webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
 * webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
 * webserver_send i_webserver_handle, S_channel_name, S_message
 * webserver_set_message_callback i_webserver_handle, S_channel_name
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
                1,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_open_html",
                sizeof(csound_webserver::csound_webserver_open_html),
                0,
                1,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_html::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_send",
                sizeof(csound_webserver::csound_webserver_send),
                0,
                1,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_send::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_set_message_callback",
                sizeof(csound_webserver::csound_webserver_set_message_callback),
                0,
                1,
                (char *)"",
                (char *)"iS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_set_message_callback::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        return status;
    }

    PUBLIC int csoundModuleDestroy_csound_webserver(CSOUND *csound) {
        csound_webserver::heep_object_manager_t<csound_webserver::CsoundWebServer>::instance().module_destroy(csound);
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


