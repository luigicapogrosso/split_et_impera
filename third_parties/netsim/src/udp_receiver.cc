#include "udp_receiver.hh"

#include <fstream>
#include <sstream>
#include <systemc>
#include <systemc.h>
#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include <time.h>

// ////////////////////////////////////////////////////////////////
// Constructor and destructor.
// ////////////////////////////////////////////////////////////////

using namespace Scnsl::Protocols::Network_Lv4;
using namespace Scnsl::Syscalls;
using namespace rapidjson;
using Scnsl::Syscalls::send;

Udp_Receiver::Udp_Receiver(const sc_core::sc_module_name modulename,
                            const task_id_t id, Scnsl::Core::Node_t * n,
                            const size_t proxies, std::string output_file, std::string input_file):
    // Parents:
    NetworkAPI_Task_if_t(modulename, id, n, proxies, DEFAULT_WMEM),
    _output_file(output_file), _input_file(input_file)
{}

Udp_Receiver::~Udp_Receiver()
{
    // Nothing to do.
}

void Udp_Receiver::waitForTime(){}

// ////////////////////////////////////////////////////////////////
// Processes.
// ////////////////////////////////////////////////////////////////

void Udp_Receiver::main()
{
    initTime();
    int sockfd, portno, n, arraylen, number, flen, udp_seq;
    struct sockaddr recv_addr, src_addr;
    byte_t buffer[1024];
    std::string encoding, file;
    std::string res;
    rapidjson::Document document;

    std::ofstream out_file(_output_file);

    if (!out_file.is_open()) 
        throw std::runtime_error("Error opening file");

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    portno = 5050;
    recv_addr.sin_port = portno;
    inet_pton(AF_INET, "192.168.0.1", &recv_addr.sin_addr);

    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0)
        throw std::runtime_error("ERROR binding");

    FILE *fin = fopen(_input_file.c_str(), "r");
    char buf[0XFFFF];
 
    //FileReadStream(FILE *fp, char *buffer, std::size_t bufferSize)
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

    int prev_seq = -1;
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sockfd, &readSet);
    timeval tv;
    int max_recv_delay=0;
    long prev_recv_time=0.0;


    //------------------------  LOOP  ----------------------------
    while(udp_seq != arraylen - 1)
    {
        Value obj(kObjectType);
        std::string s="";
        char a[5];
        char b[7];
        char * pEnd;
        int received;
        uint addrlen;
        
        tv.tv_sec = 60;
        tv.tv_usec = 200000;
        int ret = select(sockfd + 1, &readSet, NULL, NULL, &tv);
        if (!ret)
        {
            while (prev_seq != arraylen - 1)
            {
                prev_seq++;
                Value obj1(kObjectType);
                obj1.AddMember("number", document["array"][prev_seq]["number"].GetInt(), allocator);
                obj1.AddMember("time", "inf", allocator);
                obj1.AddMember("size", document["array"][prev_seq]["size"].GetInt(), allocator);
                std::string data = document["array"][prev_seq]["data"].GetString();
                val.SetString(data.c_str(), static_cast<SizeType>(data.length()), allocator);
                obj1.AddMember("data", val, allocator);
                array.PushBack(obj1, allocator);
            }
            break;
        }

        // --------------------- Receiving UDP SEQUENCE data ---------------
        bzero(buffer, 1024);

        n = recvfrom(sockfd, buffer, 1024, 0, &src_addr, &addrlen);
        memcpy(b, buffer, sizeof(int) + 2);
        udp_seq = strtol(b, &pEnd, 16);
        std::cerr << "--------RECV: seq " << udp_seq << std::endl;
        
        if (prev_seq != udp_seq - 1)
        {
            prev_seq++;
            while (prev_seq != udp_seq)
            {
                Value obj1(kObjectType);
                obj1.AddMember("number", document["array"][prev_seq]["number"].GetInt(), allocator);
                obj1.AddMember("time", "inf", allocator);
                obj1.AddMember("size", document["array"][prev_seq]["size"].GetInt(), allocator);
                std::string data = document["array"][prev_seq]["data"].GetString();
                val.SetString(data.c_str(), static_cast<SizeType>(data.length()), allocator);
                obj1.AddMember("data", val, allocator);
                array.PushBack(obj1, allocator);
                prev_seq++;
            }
        }
        else
        {
            prev_seq++;
        }

        memcpy(a, buffer+6, sizeof(int));
        number = strtol(a, &pEnd, 16);
        s.append(a, sizeof(int));
        obj.AddMember("number", number, allocator);
        std::cerr << "-------RECV: number " << number << std::endl;

        // --------------------- Receiving FILE LENGTH data ---------------
        memcpy(a, buffer+10, sizeof(int));
        flen = strtol(a, &pEnd, 16);
        s.append(a, sizeof(int));
        std::cerr << "---------RECV: len " << flen << std::endl;

        if (encoding == "file")
        {
            file = document["file"].GetString();

            obj.AddMember("time", sc_time_stamp().to_seconds(), allocator); //act_recv_time
            obj.AddMember("size", n-14, allocator);
            obj.AddMember("data", "none", allocator);

            //if(act_recv_time-prev_recv_time)
            tv.tv_sec=0;
        }

        array.PushBack(obj, allocator);
    }

    //Value sequence(kObjectType);
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
