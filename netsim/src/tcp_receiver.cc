/***************************************************************************
 *
 *            netsim/src/tcp_receiver.hh
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

#include "tcp_receiver.hh"

#include <fstream>
#include <sstream>
#include <systemc>
#include <systemc.h>
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
//#include "base64pp/base64pp.h"

// ////////////////////////////////////////////////////////////////
// Constructor and destructor.
// ////////////////////////////////////////////////////////////////

using namespace Scnsl::Protocols::Network_Lv4;
using namespace Scnsl::Syscalls;
using namespace rapidjson;
using Scnsl::Syscalls::send;

Tcp_Receiver::Tcp_Receiver(const sc_core::sc_module_name modulename, 
                            const task_id_t id, Scnsl::Core::Node_t * n,
                            const size_t proxies, std::string output_file, std::string input_file):
    // Parents:
    NetworkAPI_Task_if_t(modulename, id, n, proxies, DEFAULT_WMEM),
    _output_file(output_file), _input_file(input_file)
{

}

Tcp_Receiver::~Tcp_Receiver()
{
    // Nothing to do.
}

void Tcp_Receiver::waitForTime(){}

// ////////////////////////////////////////////////////////////////
// Processes.
// ////////////////////////////////////////////////////////////////

void Tcp_Receiver::main()
{
    initTime();
    int sockfd, portno, n, arraylen, framelength, number, flen;
    struct sockaddr serv_addr;
    byte_t buffer[1024];
    std::string encoding, file;
    rapidjson::Document document;

    std::ofstream out_file(_output_file);

    if (!out_file.is_open()) 
        throw std::runtime_error("Error opening file");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    portno = 2020;
    serv_addr.sin_port = portno;
    inet_pton(AF_INET, "192.168.0.5", &serv_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("ERROR connecting");    

    FILE *fin = fopen(_input_file.c_str(), "r");
    char buf[0XFFFF];
 
    rapidjson::FileReadStream input(fin, buf, sizeof(buf));
    document.ParseStream(input);
    fclose(fin);

    encoding = document["encoding"].GetString();
    arraylen = document["array"].Size();

    Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
    Value array(kArrayType);
    Value val(kObjectType);

    //------------------------  LOOP  ----------------------------
    for (SizeType i = 0; i < arraylen; i++){
        Value obj(kObjectType);
        std::string s="";
        char a[4];
        char * pEnd;
        int received;

        bzero(buffer, sizeof(int));
        n = recv(sockfd, buffer, sizeof(int), 0);
        if (n < sizeof(int)){
            received = n;
            do
            {
                n = recv(sockfd, buffer+received, sizeof(int)-received, 0);
                if (n < 0) throw std::runtime_error("ERROR reading to socket");
                received = received + n;
            } while (received != sizeof(int));              
        }
        memcpy(a, buffer, sizeof(int));
        number = strtol(a, &pEnd, 16);
        s.append(a, sizeof(int));
        obj.AddMember("number", number, allocator);
        std::cerr << "RECEIVER: number: " << number << std::endl;

        bzero(buffer, sizeof(int));
        n = recv(sockfd, buffer, sizeof(int), 0);
        if (n < sizeof(int)){
            received = n;
            do
            {
                n = recv(sockfd, buffer+received, sizeof(int)-received, 0);
                if (n < 0) throw std::runtime_error("ERROR reading to socket");
                received = received + n;
            } while (received != sizeof(int));              
        }
        memcpy(a, buffer, sizeof(int));
        flen = strtol(a, &pEnd, 16);
        s.append(a, sizeof(int));
        std::cerr << "RECEIVER: flen: " << flen << std::endl;

        if (encoding == "file"){
            file = document["file"].GetString();
            bzero(buffer, flen);
            n = recv(sockfd, buffer, flen, 0);
            received = n;
            if (n < 0) throw std::runtime_error("ERROR reading to socket");
            if (n<flen){
                do
                {
                    n = recv(sockfd, buffer+received, flen-received, 0);
                    if (n < 0) throw std::runtime_error("ERROR reading to socket");
                    received += n;
                } while (received != flen);              
            }
            obj.AddMember("time", sc_time_stamp().to_seconds(), allocator);
            obj.AddMember("size", received, allocator);
            obj.AddMember("data", "none", allocator);
        }

        array.PushBack(obj, allocator);
    }
    
    Value sequence(kObjectType);
    val.SetString(encoding.c_str(), static_cast<SizeType>(encoding.length()), allocator);
    d.AddMember("encoding", val, allocator);
    val.SetString(file.c_str(), static_cast<SizeType>(file.length()), allocator);
    d.AddMember("file", val, allocator);
    d.AddMember("array", array, allocator);

    //d.AddMember("sequence", sequence, allocator);

    FILE* fp = std::fopen(_output_file.c_str(), "wb");

    // writer to file (through a provided buffer)
    char writeBuffer[100000];
    rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

    d.Accept(writer);
    std::fclose(fp);

    std::cerr << "RECV: Closing" << std::endl;
    close(sockfd);

    std::cerr << "--------------------TIME RECV: " << sc_time_stamp().to_seconds() << " s" << std::endl;
}
