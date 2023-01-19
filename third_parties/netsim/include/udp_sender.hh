#ifndef UDP_SENDER_HH
#define UDP_SENDER_HH

#include <scnsl.hh>
#include <systemc>

class Udp_Sender: public Scnsl::Protocols::Network_Lv4::NetworkAPI_Task_if_t
{
public:
    Udp_Sender(
        const sc_core::sc_module_name modulename,
        const task_id_t id,
        Scnsl::Core::Node_t * n,
        const size_t proxies,
        std::string input_file);

    virtual ~Udp_Sender();

    void waitForTime();

private:
    void main() override;

    std::string _input_file;
    bool _file_valid;
};

#endif
