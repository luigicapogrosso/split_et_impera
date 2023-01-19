#ifndef TCP_SENDER_HH
#define TCP_SENDER_HH

#include <scnsl.hh>
#include <systemc>

class Tcp_Sender: public Scnsl::Protocols::Network_Lv4::NetworkAPI_Task_if_t
{
public:
    Tcp_Sender(const sc_core::sc_module_name modulename, const task_id_t id, 
                Scnsl::Core::Node_t * n, const size_t proxies,
                std::string input_file);

    virtual ~Tcp_Sender();

    void waitForTime();


private:
    void main() override;

    std::string _input_file;
    bool _file_valid;

};

#endif
