/*
 * test_msg.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/HttpReq.h"
#include "../cahttp/flog.h"
#include "../cahttp/CaHttpCommon.h"
#include "testutil.h"
#include "test_common.h"
#include "../cahttp/BaseMsg.h"

using namespace cahttp;

TEST(msg, content) {
	BaseMsg msg;
	// init value test
	ASSERT_EQ(msg.getContentLen(), 0);
	ASSERT_EQ(msg.getTransferEncoding(), false);

	msg.setContentLen(100);
	ASSERT_EQ(msg.getContentLen(), 100);
	msg.setTransferEncoding(true);
	ASSERT_EQ(msg.getTransferEncoding(), true);
	ASSERT_EQ(msg.getContentLen(), 0);
	msg.setTransferEncoding(false);
	ASSERT_EQ(msg.getTransferEncoding(), false);

	msg.setContentLen(200);
	ASSERT_EQ(msg.getContentLen(), 200);
	ASSERT_EQ(msg.getTransferEncoding(), false);

	msg.addHdr("x-user_hdr", "my header");
	ASSERT_EQ(msg.getContentLen(), 200);
	msg.setTransferEncoding(true);
	msg.addHdr("x-user_hdr", "my value");
	ASSERT_EQ(msg.getTransferEncoding(), true);
}



