#pragma once

#include <iostream>
#include <string>
#include "Util.hpp"
#include "mongoose.h"
#include "mysql.h"

#define IM_DB "im_db"
#define MY_PORT 3306

struct mg_serve_http_opts s_http_server_opts;

//model
class MysqlClient{
    private:
        MYSQL *my;

        bool ConnectMysql()
        {
            my = mysql_init(NULL);
            mysql_set_character_set(my, "utf8");
            if(!mysql_real_connect(my, "localhost",\
                        "root", "", IM_DB, MY_PORT, NULL, 0)){
                cerr << "connect mysql error" << endl;
                return false;
            }
            cout << "connect mysql success" << endl;

            return true;
        }
    public:
        MysqlClient(){
        }
        bool InsertUser(string name, string passwd)
        {
            ConnectMysql();
            string sql = "INSERT INTO user (name, passwd) values(\"";
            sql += name;
            sql += "\",\"";
            sql += passwd;
            sql += "\")";
            cout << sql << endl;
            if(0 == mysql_query(my, sql.c_str())){
                return true;
            }
            mysql_close(my);
            return false;

        }
        bool SelectUser(string name, string passwd)
        {
            ConnectMysql();
            string sql = "SELECT * FROM user WHERE name=\"";
            sql += name;
            sql += "\" AND passwd=\"";
            sql += passwd;
            sql += "\"";
            cout << sql << endl;
            if(0 != mysql_query(my, sql.c_str())){
                //return false;
            }
            cout << "select done" << endl;
            //judge result not empty
            mysql_close(my);
            return true;
        }
        ~MysqlClient(){
        }
};

//MVC
class ImServer{
    private:
        string port;
        struct mg_mgr mgr;
        struct mg_connection *nc;
        volatile bool quit;
        static MysqlClient mc;
    public:
        ImServer(string _port = "8080"):port(_port),quit(false)
        {
        }
        static void Broadcast(struct mg_connection *nc, string msg)
        {
            struct mg_connection *c;
            for (c = mg_next(nc->mgr, NULL); \
                    c != NULL; c = mg_next(nc->mgr, c)) {
                //if (c == nc) continue; /* Don't send to the sender. */
                mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT,\
                        msg.c_str(), msg.size());
            }
        }
        static void RegisterHandler(mg_connection *nc, int ev, void *data)
        {

        }
        static void LoginHandler(mg_connection *nc, int ev, void *data)
        {
            struct http_message *hm = (struct http_message*)data;
            string method = Util::mgStrToString(&(hm->method));
            if(method == "POST"){
                string body = Util::mgStrToString(&hm->body);
                std::cout << "login handler: " << body << std::endl;
                string name, passwd;
                if(Util::GetNameAndPasswd(body, name, passwd)){
                    if(mc.SelectUser(name, passwd)){
                        string test = "{\"result\":12}";
                        nc->flags |= MG_F_SEND_AND_CLOSE; //相应完毕，完毕链接
                        mg_printf(nc, "HTTP/1.1 200 OK\r\n");
                        mg_printf(nc, "Content-Length: %d\r\n\r\n", test.size());
                        mg_printf(nc, test.c_str());
                    }
                }
                //mg_serve_http(nc, hm, s_http_server_opts);
            }
            else{
                mg_serve_http(nc, hm, s_http_server_opts);
            }
        }
        static void EventHandler(mg_connection *nc, int ev, void *data)
        {
            switch(ev){
                case MG_EV_HTTP_REQUEST:{
                        struct http_message *hm = (struct http_message*)data;
                        mg_serve_http(nc, hm, s_http_server_opts);
                        //nc->flags |= MG_F_SEND_AND_CLOSE;
                    }
                    break;
                case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
                        Broadcast(nc, "some body join...");
                    }
                    break;
                case MG_EV_WEBSOCKET_FRAME:{
                        struct websocket_message *wm = (struct websocket_message*)data;
                        struct mg_str ms = {(const char*)wm->data, wm->size};
                        string msg = Util::mgStrToString(&ms);
                        Broadcast(nc, msg);
                    }
                    break;
                case MG_EV_CLOSE:
                    std::cout << "close link" << std::endl;
                    break;
                default:
                    cout << "other ev: " << ev << endl;
                    break;
            }
        }
        void InitServer()
        {
            mg_mgr_init(&mgr, NULL);
            nc = mg_bind(&mgr, port.c_str(), EventHandler);

            mg_register_http_endpoint(nc, "/login", LoginHandler);
            mg_register_http_endpoint(nc, "/register", RegisterHandler);

            mg_set_protocol_http_websocket(nc);
            s_http_server_opts.document_root = "web";
        }
        void Start()
        {
            int timeout = 1000000;
            while(!quit){
                mg_mgr_poll(&mgr, timeout);
                cout << "time out ..." << endl;
            }
        }
        ~ImServer()
        {
            mg_mgr_free(&mgr);
        }
};
MysqlClient ImServer::mc;



