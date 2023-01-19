#include "../include/tcp_receiver.hh"
#include "../include/tcp_sender.hh"
#include "../include/udp_receiver.hh"
#include "../include/udp_sender.hh"

#include <exception>
#include <scnsl.hh>
#include <sstream>
#include <systemc>
#include <systemc.h>
#include <tlm.h>
#include <cstddef>

using namespace Scnsl::Setup;
using namespace Scnsl::BuiltinPlugin;
using namespace Scnsl::Protocols::Network_Lv4;
using Scnsl::Tracing::Traceable_base_t;

Scnsl::Setup::Scnsl_t * scnsl;
Scnsl::Core::Node_t * n0;
Scnsl::Core::Node_t * n1;
std::string filename;
std::string outfile;
double error_percent;
Scnsl::Core::Channel_if_t * ch;
unsigned int bitrate = 1000000000;

void tcp_setup()
{
    const Scnsl::Core::task_id_t id0 = 0;
    const Scnsl::Core::task_id_t id1 = 1;

    Tcp_Sender sender("Sender", id0, n0, 1, filename);
    Tcp_Receiver p1("Pit", id1, n1, 1, outfile, filename);

    // Creating the protocol Tcp.
    auto tcp0 = new Scnsl::Protocols::Network_Lv4::Lv4Communicator_t("Tcp0", true);
    tcp0->setExtraHeaderSize(14 + 20);
    tcp0->setSegmentSize(
        MAX_ETH_SEGMENT - TCP_BASIC_HEADER_LENGTH - IP_HEADER_MIN_SIZE);
    tcp0->setRto(sc_core::sc_time(200, sc_core::SC_MS));
    tcp0->setSendAckAfterFullSegment(false);
    tcp0->setFastAckCount(25);
    tcp0->set_TCP_NO_DELAY(1);

    auto tcp1 = new Scnsl::Protocols::Network_Lv4::Lv4Communicator_t("Tcp1", true);
    tcp1->setExtraHeaderSize(14 + 20);
    tcp1->setSegmentSize(
        MAX_ETH_SEGMENT - TCP_BASIC_HEADER_LENGTH - IP_HEADER_MIN_SIZE);
    tcp1->setRto(sc_core::sc_time(200, sc_core::SC_MS));
    tcp1->setSendAckAfterFullSegment(false);
    tcp1->setFastAckCount(25);
    tcp0->set_TCP_NO_DELAY(1);

    std::queue<Lv4Saboteur_channel_down_infos> timeouts;

    auto sab1 = new Scnsl::Protocols::Network_Lv4::Lv4ByteSaboteur_t("Sab_1",
                                                                     error_percent,
                                                                     true,
                                                                     false,
                                                                     timeouts,
                                                                     false);

    sab1->avoid_delete_retransmitted_packets(true);
    tcp1->stackDown(sab1);
    sab1->stackUp(tcp1);
    Communicator_if_t * stack1 = new Scnsl::Utils::CommunicatorStack_t(tcp1, sab1);

    // Adding tracing features.
    CoreTracingSetup_t cts;
    cts.extensionId = "core";

    // Setting the formatter.
    cts.formatterExtensionId = "core";
    cts.formatterName = "basic";

    // Setting the filter.
    cts.filterExtensionId = "core";
    cts.filterName = "basic";

    // Core formatter specific option, printing also the type of trace.
    cts.print_trace_type = true;
    cts.print_trace_timestamp = true;

    // Setting to trace only user-like infos.
    cts.info = 5;
    cts.debug = 0;
    cts.log = 5;
    cts.error = 0;
    cts.warning = 0;
    cts.fatal = 0;

    // Creating.
    Scnsl_t::Tracer_t * tracer1 = scnsl->createTracer(cts);

    // Setting the output stream.
    tracer1->addOutput(&std::cout);
    tracer1->trace(ch);
    tracer1->trace(dynamic_cast<Traceable_base_t *>(tcp0));
    tracer1->trace(dynamic_cast<Traceable_base_t *>(tcp1));
    tracer1->trace(dynamic_cast<Traceable_base_t *>(sab1));

    // Setting to trace backend-like infos.
    cts.info = 0;
    cts.debug = 5;
    cts.log = 0;
    cts.error = 5;
    cts.warning = 5;
    cts.fatal = 5;

    // Creating.
    Scnsl_t::Tracer_t * tracer2 = scnsl->createTracer(cts);

    // Setting the output stream.
    tracer2->addOutput(&std::cerr);
    tracer2->trace(ch);
    tracer2->trace(Scnsl::Utils::EventsQueue_t::get_instance());
    tracer2->trace(dynamic_cast<Traceable_base_t *>(tcp0));
    tracer2->trace(dynamic_cast<Traceable_base_t *>(tcp1));
    tracer2->trace(dynamic_cast<Traceable_base_t *>(sab1));

    // Binding.
    BindSetup_base_t bsb0;
    bsb0.extensionId = "core";
    bsb0.destinationNode = n1;
    bsb0.node_binding.x = 0;
    bsb0.node_binding.y = 0;
    bsb0.node_binding.z = 0;
    bsb0.node_binding.bitrate = bitrate;
    bsb0.node_binding.transmission_power = 1000;
    bsb0.node_binding.receiving_threshold = 1;

    bsb0.socket_binding.socket_active = true;
    bsb0.socket_binding.source_ip = SocketMap::getIP("192.168.0.5");
    bsb0.socket_binding.source_port = 2020;
    bsb0.socket_binding.dest_ip = SocketMap::getIP("192.168.0.6");
    bsb0.socket_binding.dest_port = 5050;

    scnsl->bind(n0, ch, bsb0);
    scnsl->bind(&sender, &p1, ch, bsb0, tcp0);

    BindSetup_base_t bsb1;
    bsb1.extensionId = "core";
    bsb1.destinationNode = n0;
    bsb1.node_binding.x = 1;
    bsb1.node_binding.y = 1;
    bsb1.node_binding.z = 1;
    bsb1.node_binding.bitrate = bitrate;
    bsb1.node_binding.transmission_power = 1000;
    bsb1.node_binding.receiving_threshold = 1;

    bsb1.socket_binding.socket_active = true;
    bsb1.socket_binding.source_ip = SocketMap::getIP("192.168.0.6");
    bsb1.socket_binding.source_port = 5050;
    bsb1.socket_binding.dest_ip = SocketMap::getIP("192.168.0.5");
    bsb1.socket_binding.dest_port = 2020;

    scnsl->bind(n1, ch, bsb1);
    scnsl->bind(&p1, &sender, ch, bsb1, stack1);

    sc_core::sc_start(sc_core::sc_time(5000, sc_core::SC_SEC));
    sc_core::sc_stop();
}

void udp_setup()
{
    const Scnsl::Core::task_id_t id0 = 0;
    const Scnsl::Core::task_id_t id1 = 1;

    Udp_Sender ts("TaskSender", id0, n0, 1, filename);
    Udp_Receiver tr("TaskReceiver", id1, n1, 1, outfile, filename);

    // Creating the protocol Udp.
    auto udp0 = new Scnsl::Protocols::Network_Lv4::Lv4Communicator_t("Udp0", true);
    udp0->setExtraHeaderSize(20, 14);

    auto udp1 = new Scnsl::Protocols::Network_Lv4::Lv4Communicator_t("Udp1", true);
    udp1->setExtraHeaderSize(20, 14);

    std::queue<Lv4Saboteur_channel_down_infos> timeouts;

    auto sab1 = new Scnsl::Protocols::Network_Lv4::Lv4ByteSaboteur_t("Sab_1",
                                                                     error_percent,
                                                                     true,
                                                                     false,
                                                                     timeouts,
                                                                     false);
    sab1->avoid_delete_retransmitted_packets(true);
    udp1->stackDown(sab1);
    sab1->stackUp(udp1);
    Communicator_if_t * stack1 = new Scnsl::Utils::CommunicatorStack_t(udp1, sab1);

    // Adding tracing features.
    CoreTracingSetup_t cts;
    cts.extensionId = "core";

    // Setting the formatter.
    cts.formatterExtensionId = "core";
    cts.formatterName = "basic";

    // Setting the filter.
    cts.filterExtensionId = "core";
    cts.filterName = "basic";

    // Core formatter specific option, printing also the type of trace.
    cts.print_trace_type = true;
    cts.print_trace_timestamp = true;

    // Setting to trace only user-like infos.
    cts.info = 5;
    cts.debug = 0;
    cts.log = 5;
    cts.error = 0;
    cts.warning = 0;
    cts.fatal = 0;

    // Creating.
    Scnsl_t::Tracer_t * tracer1 = scnsl->createTracer(cts);

    // Setting the output stream.
    tracer1->addOutput(&std::cout);
    tracer1->trace(ch);
    tracer1->trace(dynamic_cast<Traceable_base_t *>(udp0));
    tracer1->trace(dynamic_cast<Traceable_base_t *>(udp1));
    tracer1->trace(dynamic_cast<Traceable_base_t *>(sab1));

    // Setting to trace backend-like infos.
    cts.info = 0;
    cts.debug = 5;
    cts.log = 0;
    cts.error = 5;
    cts.warning = 5;
    cts.fatal = 5;

    // Creating.
    Scnsl_t::Tracer_t * tracer2 = scnsl->createTracer(cts);

    // Setting the output stream.
    tracer2->addOutput(&std::cerr);

    // Adding to trace.
    tracer2->trace(ch);
    tracer2->trace(Scnsl::Utils::EventsQueue_t::get_instance());
    tracer2->trace(dynamic_cast<Traceable_base_t *>(udp0));
    tracer2->trace(dynamic_cast<Traceable_base_t *>(udp1));
    tracer2->trace(dynamic_cast<Traceable_base_t *>(sab1));

    // Binding.
    BindSetup_base_t bsb0;
    bsb0.extensionId = "core";
    bsb0.destinationNode = n1;
    bsb0.node_binding.x = 0;
    bsb0.node_binding.y = 0;
    bsb0.node_binding.z = 0;
    bsb0.node_binding.bitrate = bitrate;
    bsb0.node_binding.transmission_power = 1000;
    bsb0.node_binding.receiving_threshold = 1;

    bsb0.socket_binding.socket_active = true;
    bsb0.socket_binding.source_ip = SocketMap::getIP("192.168.0.2");
    bsb0.socket_binding.source_port = 2020;
    bsb0.socket_binding.dest_ip = SocketMap::getIP("192.168.0.1");
    bsb0.socket_binding.dest_port = 5050;

    scnsl->bind(n0, ch, bsb0);
    scnsl->bind(&ts, &tr, ch, bsb0, udp0);

    BindSetup_base_t bsb1;
    bsb1.extensionId = "core";
    bsb1.destinationNode = n0;
    bsb1.node_binding.x = 1;
    bsb1.node_binding.y = 1;
    bsb1.node_binding.z = 1;
    bsb1.node_binding.bitrate = bitrate;
    bsb1.node_binding.transmission_power = 1000;
    bsb1.node_binding.receiving_threshold = 1;

    bsb1.socket_binding.socket_active = true;
    bsb1.socket_binding.source_ip = SocketMap::getIP("192.168.0.1");
    bsb1.socket_binding.source_port = 5050;
    bsb1.socket_binding.dest_ip = SocketMap::getIP("192.168.0.2");
    bsb1.socket_binding.dest_port = 2020;

    scnsl->bind(n1, ch, bsb1);
    scnsl->bind(&tr, &ts, ch, bsb1, stack1);

    sc_core::sc_start(sc_core::sc_time(5000, sc_core::SC_SEC));
    sc_core::sc_stop();
}

int sc_main(int argc, char * argv[])
{
    // Singleton.
    scnsl = Scnsl::Setup::Scnsl_t::get_instance();

    // Nodes creation.
    n0 = scnsl->createNode();
    n1 = scnsl->createNode();

    float delayVal;
    std::string protocol;

    if (argc < 5)
    {
        std::cerr << "NOT ENOUGHT ARGUMENT" << std::endl;
        exit(1);
    }
    else
    {
        filename = argv[1];
        outfile = argv[2];
        error_percent = std::stod(argv[3]);
        delayVal = std::stoi(argv[4]);
        protocol = argv[5];
    }

    std::cerr << "Test with " << error_percent << " error rate" << std::endl;

    sc_core::sc_time DELAY(delayVal, sc_core::SC_US);

    std::cerr << "DELAY :" << DELAY << std::endl;

    CoreChannelSetup_t ccs;
    ccs.channel_type = CoreChannelSetup_t::FULL_DUPLEX;
    ccs.capacity = 1000000000;
    ccs.capacity2 = 1000000000;
    ccs.delay = DELAY;
    ccs.extensionId = "core";
    ccs.name = "channel_fullduplex";
    ch = scnsl->createChannel(ccs);


    if(protocol.compare("tcp") == 0)
    {
        tcp_setup();
    }
    else if(protocol.compare("udp") == 0)
    {
        udp_setup();
    }
    else
    {
        std::cerr << "WRONG PROTOCOL ARGUMENT" << std::endl;
    }

    return 0;
}
