#include <future>
#include <chrono>
#include <csound.hpp>
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
#include <csound_runtime_interface.hpp>

#define XSTR(x) STR(x)
#define STR(x) #x

#pragma message "__cplusplus is defined as: " XSTR(__cplusplus) 

namespace csound_webserver {
        
    static std::mutex &get_mutex() {
        static std::mutex mutex_;
        return mutex_;
    }
    
    static bool diagnostics_enabled = true;

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
        csound::CsoundRuntimeInterface Csound;
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
        virtual void create_(CSOUND *csound_, const std::string &base_directory_, int port_) {
            csound = csound_;
            Csound.initialize(csound);
            base_directory = base_directory_;
            if(port != -1) {
                port = port_;
            } else {
                port = 8080;
            }
            if (diagnostics_enabled) {
                server.set_logger([] (const auto& req, const auto& res) {
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
            server.Post("/CompileCsdText", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileCsdText...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto csd_text = json_request["params"]["csd_text"].get<std::string>();
                auto result = Csound.CompileCsdText(csd_text.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileCsdText: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/CompileOrc", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileOrc...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto orc_code = json_request["params"]["orc_code"].get<std::string>();
                auto result = Csound.CompileOrc(orc_code.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/CompileOrc: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/EvalCode", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto orc_code = json_request["params"]["orc_code"].get<std::string>();
                auto result = Csound.EvalCode(orc_code.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/Get0dBFS", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/Get0dBFS...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.Get0dBFS();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/Get0dBFS: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetAudioChannel", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetAudioChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto ksmps = csound->GetKsmps(csound);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();
                std::vector<MYFLT> result;
                result.resize(ksmps);
                MYFLT *buffer = &result.front();
                Csound.GetAudioChannel(channel_name.c_str(), buffer);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetAudioChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetControlChannel", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();    
                int err;          
                auto result = Csound.GetControlChannel(channel_name.c_str(), &err);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetControlChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                if (err == OK) {
                    response.status = 201;
                } else {
                    response.status = 404;
                }
            });
            server.Post("/GetDebug", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetDebug...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetDebug();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/EvalCode: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetKsmps", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetKsmps...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetKsmps();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetKsmps: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetNchnls", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnls...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetNchnls();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnls: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetNchnlsInput", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnlsInput...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetNchnls_input();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetNchnlsInput: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetScoreOffsetSeconds", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreOffsetSeconds...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetScoreOffsetSeconds();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreOffsetSeconds: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetScoreTime", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreTime...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetScoreTime();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetScoreTime: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/GetSr", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetSr...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.GetSr();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetSr: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            /*
            server.Post("/GetStringChannel", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/GetStringChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();     
                char buffer[0x500];     
                Csound.GetStringChannel(channel_name.c_str(), &buffer[0]);
                create_json_response(json_request, response, buffer);
                if (diagnostics_enabled) std::fprintf(stderr, "/GetStringChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            */
            server.Post("/InputMessage", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/InputMessage...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto sco_code = json_request["params"]["sco_code"].get<std::string>();     
                Csound.InputMessage(sco_code.c_str());
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/InputMessage: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/IsScorePending", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/IsScorePending...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto result = Csound.IsScorePending();
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/IsScorePending: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/Message", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/Message...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto message = json_request["params"]["message"].get<std::string>();     
                Csound.Message(message.c_str());
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/Message: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
             server.Post("/ReadScore", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/ReadScore...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto sco_code = json_request["params"]["sco_code"].get<std::string>();     
                auto result = Csound.ReadScore(sco_code.c_str());
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/ReadScore: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
             server.Post("/RewindScore", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/RewindScore...\n");
                auto json_request = nlohmann::json::parse(request.body);
                Csound.RewindScore();
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/RewindScore: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/ScoreEvent", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/ScoreEvent...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto opcode_code = json_request["params"]["opcode_code"].get<char>();     
                auto pfields = json_request["params"]["pfields"].get<std::vector<MYFLT> >(); 
                Csound.ScoreEvent(opcode_code, pfields);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/ScoreEvent: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/SetControlChannel", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();     
                auto channel_value = json_request["params"]["channel_value"].get<MYFLT>(); 
                Csound.SetControlChannel(channel_name.c_str(), channel_value);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/SetDebug", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetDebug...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto enabled = json_request["params"]["enabled"].get<int>();     
                Csound.SetDebug(enabled);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetDebug: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/SetScoreOffsetSeconds", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScoreOffsetSeconds...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto score_time = json_request["params"]["score_time"].get<MYFLT>();     
                Csound.SetScoreOffsetSeconds(score_time);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScoreOffsetSeconds: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/SetScorePending", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScorePending...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto pending = json_request["params"]["pending"].get<int>();     
                Csound.SetScorePending(pending);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetScorePending: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/SetStringChannel", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/SetControlChannel...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto channel_name = json_request["params"]["channel_name"].get<std::string>();     
                auto channel_value = json_request["params"]["channel_value"].get<std::string>();
                Csound.SetStringChannel(channel_name.c_str(), const_cast<char *>(channel_value.c_str()));
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/SetStringChannel: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/TableGet", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableGet...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();     
                auto index = json_request["params"]["index"].get<int>();
                MYFLT result = Csound.TableGet(table_number, index);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableGet: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/TableLength", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableLength...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();     
                int result = Csound.TableLength(table_number);
                create_json_response(json_request, response, result);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableLength: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
            });
            server.Post("/TableSet", [&](const httplib::Request &request, httplib::Response &response) {
                if (diagnostics_enabled) std::fprintf(stderr, "/TableSet...\n");
                auto json_request = nlohmann::json::parse(request.body);
                auto table_number = json_request["params"]["table_number"].get<int>();     
                auto index = json_request["params"]["table_number"].get<int>();     
                auto  value = json_request["params"]["table_number"].get<MYFLT>();     
                Csound.TableSet(table_number, index, value);
                create_json_response(json_request, response, OK);
                if (diagnostics_enabled) std::fprintf(stderr, "/TableSet: response: %s\n", response.body.c_str());
                // This is the HTTP result code.
                response.status = 201;
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
                std::snprintf(html_path, 0x500, "csound_webserver_%x.html", mersenne_twister());
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
    };
    
    typedef csound::heap_object_manager_t<csound_webserver::CsoundWebServer> webservers;
    
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
                auto server = CsoundWebServer::create(csound, base_uri_, port);
                int handle = webservers::instance().handle_for_object(csound, server);
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
                log(csound, "csound_webserver_open_resource::init: this: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string resource = S_resource->data;
                std::string browser = S_browser->data;
                server = webservers::instance().object_for_handle(csound, i_server_handle);
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
                server = webservers::instance().object_for_handle(csound, i_server_handle);
                server->open_html(html_text, browser);
                log(csound, "csound_webserver_open_html::init.\n");
                return result;
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

