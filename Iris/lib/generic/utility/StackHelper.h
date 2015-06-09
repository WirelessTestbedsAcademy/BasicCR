/**
 * \file lib/generic/Utility/StackHelper.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2011-2013 Andre Puschmann <andre.puschmann@tu-ilmenau.de>
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
 * This Class provides some common methods for Stack components.
 *
 * This Class provides some common methods for Stack components like
 * displaying the content of a StackDataSet for debugging purposes
 * as well as serialization/deserialization for StackDataSet/Protobuf
 * objects.
 *
 */

#ifndef STACKHELPER_H
#define STACKHELPER_H

#include <boost/shared_ptr.hpp>
#include <irisapi/StackDataBuffer.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <google/protobuf/message.h>


namespace iris
{

class StackHelper
{
public:  
    /**
     * \brief Tries to deserialize a StackDataSet into ProtoBuf object.
     *
     * This function first copies the StackDataSet into a continous data
     * structure and then tries to desierialize it into a given ProtoBuf
     * object. If the StackDataSet does not contain a proper Protobuf
     * object the function returns false. If yes, and the object contains
     * any payload, only the payload of the packet remains inside the
     * StackDataSet for further processing.
     *
     * \param frame the StackDataSet
     * \param protobuf the ProtocolBuffer data structure
     * \return true - if successful, false otherwise.
     */
    template<typename T>
    static bool deserializeAndStripDataset(boost::shared_ptr<StackDataSet> frame, T &protobuf)
    {
        // copy frame into vector first
        std::vector<uint8_t> buffer(frame->data.begin(), frame->data.end());

        if (!protobuf.ParseFromArray((void*)&buffer.front(), buffer.size())) {
            return false;
        }

        // retrieve payload from packet which is the first element in RepeatedPtrField
        if (protobuf.mutable_payload()->size() == 1) {
            std::string payload = protobuf.mutable_payload()->Get(0);
            frame->data.clear();
            frame->data.insert(frame->data.end(), payload.c_str(), payload.c_str() + payload.size());
        }
        return true;
    }


    /**
     * \brief Merges a StackDataSet/ProtoBuf object and serializes it.
     *
     * This function appends the data of a given StackDataSet as payload
     * to a protobuf object which is then serialized. The StackDataSet is
     * then filled with the resulting binary data.
     *
     * \param frame the StackDataSet
     * \param protobuf the ProtocolBuffer data structure
     * \return void
     */
    template<typename T>
    static void mergeAndSerializeDataset(boost::shared_ptr<StackDataSet> frame, T &protobuf)
    {
        // construct vector from deque and add to protobuf packet
        std::vector<uint8_t> payload(frame->data.begin(), frame->data.end());
        protobuf.add_payload(&payload.front(), payload.size());

        // serialize whole packet into "old" payload vector
        payload.clear();
        payload.resize(protobuf.ByteSize());
        protobuf.SerializeWithCachedSizesToArray(&payload.front());

        // clear "old" Iris frame and construct new one from payload
        frame->data.clear();
        frame->data.insert(frame->data.begin(), payload.begin(), payload.end());
    }


    /**
     * \brief Print content of a StackDataSet to console.
     *
     * This function simply writes the content of a StackDataSet in hexadecimal
     * representation to the console.
     *
     * \param frame the StackDataSet
     * \param comment Comment to be displayed along with data
     * \return void
     */
    static void printDataset(const boost::shared_ptr<StackDataSet> frame, const char *comment)
    {
        size_t outputCounter = 0;

        std::cout << boost::format("--------") << std::endl;
        std::cout << boost::format("Comment: %s") % (comment) << std::endl;
        std::cout << boost::format("Total length: %d") % (frame->data.size()) << std::endl;

        typedef std::deque<uint8_t>::iterator deqIterator;
        for (deqIterator it = frame->data.begin(); it != frame->data.end(); it++)
        {
            // int cast is neccessary, std::hex doesn't know uint8_t
            std::cout << boost::format("%02X ") % (static_cast<int>(*it));

            outputCounter++;
            if (outputCounter % 8 == 0)
                std::cout << std::endl;
        }
        if (outputCounter % 8 != 0)
            std::cout << std::endl;
        std::cout << boost::format("--------") << std::endl;
    }
};

} // end of iris namespace
#endif // STACKHELPER_H
