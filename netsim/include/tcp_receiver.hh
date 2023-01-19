#ifndef SCNSL_TCP_ADDON_TCP_RECEIVER_HH
#define SCNSL_TCP_ADDON_TCP_RECEIVER_HH

#include <scnsl.hh>
#include <systemc>

class Tcp_Receiver: public Scnsl::Protocols::Network_Lv4::NetworkAPI_Task_if_t
{
public:

    Tcp_Receiver(const sc_core::sc_module_name modulename, const task_id_t id,
                    Scnsl::Core::Node_t * n, const size_t proxies,
                    std::string output_file, std::string input_file);

    virtual ~Tcp_Receiver();

    void waitForTime();

private:
    void main() override;
    std::string _output_file;
    std::string _input_file;
};
#endif  // SCNSL_TCP_ADDON_TCP_RECEIVER_HH
