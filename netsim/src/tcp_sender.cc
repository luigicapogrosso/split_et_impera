/***************************************************************************
 *
 *            netsim/src/tcp_sender.cc
 *
 ***************************************************************************/

/*
 * MIT License
 *
 * Copyright (c) 2023 Luigi Capogrosso, Federico Cunico, Michele Lora, 
 *                    Marco Cristani, Franco Fummi, and Davide Quaglia.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tcp_sender.hh"

#include <chrono>
#include <cassert>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <systemc.h>
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

// ////////////////////////////////////////////////////////////////
// Constructor and destructor.
// ////////////////////////////////////////////////////////////////

using namespace Scnsl::Protocols::Network_Lv4;
using namespace Scnsl::Syscalls;
using namespace rapidjson;
using Scnsl::Syscalls::send;

Tcp_Sender::Tcp_Sender(const sc_core::sc_module_name modulename, 
                        const task_id_t id, Scnsl::Core::Node_t * n,
                        const size_t proxies, std::string input_file):
// Parents:
NetworkAPI_Task_if_t(modulename, id, n, proxies, DEFAULT_WMEM),
_input_file(input_file)
{
    if (input_file == "NONE")
        _file_valid = false;
    else _file_valid = true;
}

Tcp_Sender::~Tcp_Sender()
{
    // Nothing to do.
}

void Tcp_Sender::waitForTime(){}

// ////////////////////////////////////////////////////////////////
// Processes.
// ////////////////////////////////////////////////////////////////

void Tcp_Sender::main()
{
    initTime();
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[1024], read[1016];
    int length, arraylength, framelength, number;
    std::ifstream in;
    rapidjson::Document document;
    std::string encoding;
    struct sockaddr serv_addr, cli_addr;

    if (_file_valid)
    {
        FILE *fp = fopen(_input_file.c_str(), "r");
        char buf[0XFFFF];
 
        rapidjson::FileReadStream input(fp, buf, sizeof(buf));
        document.ParseStream(input);

        assert(document.HasMember("encoding"));
        assert(document["encoding"].IsString());

        assert(document.HasMember("file"));
        assert(document["file"].IsString());

        assert(document.HasMember("array"));
        fclose(fp);
    }
    else
        exit(1);

    int n;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) throw std::runtime_error("ERROR opening socket");
    portno = 2020;
    inet_pton(AF_INET, "192.168.0.5", &serv_addr.sin_addr);
    serv_addr.sin_port = portno;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("ERROR on binding");
    listen(sockfd, 1);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) throw std::runtime_error("ERROR on accept");

    encoding = document["encoding"].GetString();
    arraylength = document["array"].Size();
    
    //--------------------------  LOOP  -------------------------------------------------
    if (encoding == "file")
    {
        std::string file = document["file"].GetString();
        in.open(file, std::ifstream::binary);

        for (SizeType i = 0; i < arraylength; i++){

            wait(document["array"][i]["time"].GetDouble()-sc_time_stamp().to_seconds(), SC_SEC);

            char hex[5]= {'0','0','0','0'};
            number= document["array"][i]["number"].GetInt();
            sprintf(hex, "%04X", number);
            bzero(buffer, 1024);
            strcat (buffer,hex);

            length= document["array"][i]["size"].GetInt();
            sprintf(hex, "%04X", length);
            strcat (buffer,hex);
            

            in.read(read, length);
            strcat (buffer,read);

            n = Scnsl::Syscalls::send(newsockfd, (byte_t *)buffer, length+8, 0);
            
            if (n < 0) 
            throw std::runtime_error("ERROR writing to socket");
        }
    }

    close(newsockfd);
    close(sockfd);

    std::cerr << "-----------------------TIME SEND: " << sc_time_stamp().to_seconds() << "s"  << std::endl;
}