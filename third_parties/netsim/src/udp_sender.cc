#include "udp_sender.hh"

#include <chrono>
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

Udp_Sender::Udp_Sender(const sc_core::sc_module_name modulename,
                        const task_id_t id, Scnsl::Core::Node_t * n,
                        const size_t proxies, std::string input_file):
    // Parents:
    NetworkAPI_Task_if_t(modulename, id, n, proxies, DEFAULT_WMEM),
    _input_file(input_file)
{
    if (input_file == "NONE")
        _file_valid = false;
    else
        _file_valid = true;
}

Udp_Sender::~Udp_Sender()
{
    // Nothing to do.
}

void Udp_Sender::waitForTime(){}

// ////////////////////////////////////////////////////////////////
// Processes.
// ////////////////////////////////////////////////////////////////

void Udp_Sender::main()
{
    initTime();
    int sockfd, portno;
    socklen_t clilen;
    char buffer[1024], read[1024];;
    int length, arraylength, number;
    std::ifstream in;
    rapidjson::Document document;
    std::string encoding;
    struct sockaddr recv_addr;
    wait(1, sc_core::SC_MS);
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
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) throw std::runtime_error("ERROR opening socket");

    inet_pton(AF_INET, "192.168.0.1", &recv_addr.sin_addr);
    recv_addr.sin_port = 5050;

    std::cerr << "UDP" << std::endl;

    encoding = document["encoding"].GetString();
    arraylength = document["array"].Size();

    //--------------------------  LOOP  -------------------------------------------------
    if (encoding == "file")
    {
        std::string file = document["file"].GetString();
        in.open(file, std::ifstream::binary);

        for (SizeType i = 0; i < arraylength; i++)
        {
            wait(document["array"][i]["time"].GetDouble()-sc_time_stamp().to_seconds(), SC_SEC);

            // UDP SEQUENCE
            bzero(buffer, 1024);
            char hex[5] = {'0','0','0','0'};
            char seq_hex[7] = {'0','0','0','0','0','0'};
            sprintf(seq_hex, "%06X", i);
            strcat (buffer, seq_hex);

            // JSON NUMBER
            number= document["array"][i]["number"].GetInt();
            sprintf(hex, "%04X", number);
            strcat (buffer,hex);

            // LENGTH
            length= document["array"][i]["size"].GetInt();
            sprintf(hex, "%04X", length);
            strcat (buffer,hex);
            

            in.read(read, length);
            strcat (buffer,read);

            n = Scnsl::Syscalls::sendto(sockfd, (byte_t *)(buffer), length+14,
                        0, &recv_addr, sizeof(recv_addr));
            
            if (n < 0) 
            throw std::runtime_error("ERROR writing to socket");
        }
    }

    //in.close();
    close(sockfd);

    std::cerr << "-----------------------TIME SEND: " << sc_time_stamp().to_seconds() << "s"  << std::endl;
}
