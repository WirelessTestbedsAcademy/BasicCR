/**
 * \file lib/generic/utility/NetworkingHelper.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * This Class provides helper methods related to networking code.
 *
 * This Class provides helper methods for accessing the network stack of
 * the operating systems as well as easier handling of network packets.
 * This includes getting the current address of a networking device as
 * well as low-level parsing of network packets.
 *
 */

#ifndef NETWORKINGHELPER_H
#define NETWORKINGHELPER_H

#include <boost/shared_ptr.hpp>
#include <irisapi/StackDataBuffer.h>
#include <boost/format.hpp>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

namespace iris
{

class NetworkingHelper
{
public:
    /**
     * \brief Parse StackDataSet and retrieve Ethernet source and destionation address.
     *
     * This function parses a StackDataSet, interprets it as an Ethernet frame and tries
     * to retrieve the source and destionation address of it. It doesn't change anything
     * if the given StackDataSet is not an Ethernet frame.
     *
     * \param packet the StackDataSet
     * \param source reference to the source address to be filled
     * \param destination reference to the destination address to be filled
     * \return true - if successful, false otherwise.
     */
    static bool getAddressFromEthernetFrame(boost::shared_ptr<StackDataSet> packet, std::string &source, std::string &destination)
    {
        unsigned char packetBuffer[sizeof(struct ether_header)];
        const struct ether_header* ethernetHeader;
        boost::format destination_fmt("%02x%02x%02x%02x%02x%02x");
        boost::format source_fmt("%02x%02x%02x%02x%02x%02x");

        // serialize ethernet header
        for (int i = 0; i < sizeof(struct ether_header); i++)
            packetBuffer[i] = packet->data[i];

        // check if it's an IP or ARP frame
        ethernetHeader = (struct ether_header*)packetBuffer;
        if ((ntohs(ethernetHeader->ether_type) != ETHERTYPE_IP) &&
            (ntohs(ethernetHeader->ether_type) != ETHERTYPE_ARP))
        {
            return false;
        }

        for (size_t i = 0; i < 6; i++) {
            destination_fmt % (int(ethernetHeader->ether_dhost[i]) & 0xFF);
            source_fmt % (int(ethernetHeader->ether_shost[i]) & 0xFF);
        }

        destination = destination_fmt.str();
        source = source_fmt.str();
        return true;
    }


    /**
     * \brief Retrieve MAC address from given network interface
     *
     * This function tries to retrieve the MAC address of a given network interface.
     *
     * \param interface name of the network interface
     * \param address reference to the address to be filled
     * \return true - if successful, false otherwise.
     */
    static bool getLocalAddress(const std::string interface, std::string &address)
    {
        boost::format fmt("%02x%02x%02x%02x%02x%02x");
        struct ifreq buffer;

        int s = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&buffer, 0x00, sizeof(buffer));
        strcpy(buffer.ifr_name, interface.c_str());
        if (ioctl(s, SIOCGIFHWADDR, &buffer) != 0) {
            //LOG(LERROR) << "Error while reading MAC address from device " << interface;
            return false;
        }
        close(s);

        for (size_t i = 0; i < 6; i++)
            fmt % (int(buffer.ifr_hwaddr.sa_data[i]) & 0xFF);

        address = fmt.str();
        return true;
    }
};

} // end of iris namespace

#endif // NETWORKINGHELPER_H
