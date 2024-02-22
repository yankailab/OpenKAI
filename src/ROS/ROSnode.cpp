/*
 * ROSnode.cpp
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#include "ROSnode.h"

namespace kai
{
    bool ROSnode::createSubscriptions(void)
    {
        m_pScTopic = this->create_subscription<std_msgs::msg::String>(
            "topic",
            10,
            std::bind(&ROSnode::cbTopic, this, _1));

        return true;
    }

    void ROSnode::cbTopic(const std_msgs::msg::String &msg)
    {
        LOG_(msg.data.c_str());
    }

}
