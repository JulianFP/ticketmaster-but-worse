#include "server-connection.hpp"
#include "ticket.hpp"
#include <cstdint>
#include <httplib.h>

bool ServerConnection::is_new_error(std::string &old_error_msg, const std::string &error_msg) const {
    if (old_error_msg == error_msg) {
        return false;
    }
    else {
        old_error_msg = error_msg;
        return true;
    }
}

json ServerConnection::post(const std::string &route, const httplib::Params &form_params, const httplib::Headers &headers) {
    json json_object;
    bool error = false;
    std::string error_msg;

    for(int i = 0; i < retry_counter; ++i){
        if (auto res = cli.Post(route, headers, form_params)){
            try{
                json_object = json::parse(res->body);
            }
            catch (const json::parse_error& e) {
                error = true;
                if (is_new_error(error_msg, "Server response is not valid json")){
                    i = 0;
                }
            }
            json_object["status"] = res->status; //include the status code in the json object too for easy access
            break; //no error
        }
        else {
            error = true;
            if (is_new_error(error_msg, httplib::to_string(res.error()))){
                i = 0;
            }
        }
    }
    if (error) {
        throw CommunicationError(error_msg);
    }
    return json_object;
}

json ServerConnection::get(const std::string &route, const httplib::Params &query_params, const httplib::Headers &headers) {
    json json_object;
    bool error = false;
    std::string error_msg;

    for(int i = 0; i < retry_counter; ++i){
        if (auto res = cli.Get(route, query_params, headers)){
            try{
                json_object = json::parse(res->body);
            }
            catch (const json::parse_error& e) {
                error = true;
                if (is_new_error(error_msg, "Server response is not valid json")){
                    i = 0;
                }
            }
            json_object["status"] = res->status; //include the status code in the json object too for easy access
            break; //no error
        }
        else {
            error = true;
            if (is_new_error(error_msg, httplib::to_string(res.error()))){
                i = 0;
            }
        }
    }
    if (error) {
        throw CommunicationError(error_msg);
    }
    return json_object;
}

json ServerConnection::post_logged_in(const std::string &route, const httplib::Params &form_params) {
    if (access_token.length() == 0) {
        throw CommunicationError("Please login first before calling post_logged_in!");
    }
    else {
        httplib::Headers headers = {
            { "Authorization", "Bearer " + access_token }
        };
        return post(route, form_params, headers);
    }
}

json ServerConnection::get_logged_in(const std::string &route, const httplib::Params &query_params) {
    if (access_token.length() == 0) {
        throw CommunicationError("Please login first before calling post_logged_in!");
    }
    else {
        httplib::Headers headers = {
            { "Authorization", "Bearer " + access_token }
        };
        return get(route, query_params, headers);
    }
}

ServerConnection::ServerConnection(std::string server_address, uint16_t server_port, std::string server_ca_bundle_path)
    : cli(httplib::SSLClient(server_address, server_port))
    , access_token("")
    {
        cli.set_ca_cert_path(server_ca_bundle_path);
    }

void ServerConnection::login() {
    httplib::Params params{
        { "username", "defaultUser" },
        { "password", "password1234"}
    };
    json response = post("/login", params);

    access_token = response["client_token"];
}

void ServerConnection::get_tickets(std::vector<Ticket> &tickets) {
    json response = get_logged_in("/get_tickets");

    for (json ticket_obj : response["tickets"]) {
        tickets.push_back(Ticket(ticket_obj["token"], ticket_obj["totp_secret"]));
    }
}

bool ServerConnection::check_ticket_validity(const Ticket &ticket) {
    httplib::Params params{
        { "ticket", ticket.get_ticket_string() }
    };
    json response = get("/check_ticket_validity", params);

    if (response["status"] == 200) return true;
    else return false;
}
