/***************************************************************************
 *
 *            netsim/include/tcp_receiver.hh
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
