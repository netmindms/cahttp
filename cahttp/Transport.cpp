/*
 * Transport.cpp
 *
 *  Created on: May 7, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG

#include "Transport.h"
#include "flog.h"

using namespace std;

namespace cahttp {

Transport::Transport() {
	// TODO Auto-generated constructor stub

}

Transport::~Transport() {
	// TODO Auto-generated destructor stub
}

void Transport::init(std::shared_ptr<SimpleCnn> pcnn) {
	mpCnn = pcnn;
	pcnn->setOnListener([this](SimpleCnn::CH_E evt) ->int {
		alv("rx ch event=%d", (int)evt);
		if(evt == SimpleCnn::CH_E::CH_MSG) {
			mLis(kOnMsg);
			return 0;
		} else if(evt == SimpleCnn::CH_E::CH_DATA) {
			mLis(kOnData);
		} else if(evt == SimpleCnn::CH_E::CH_CLOSED) {
			mLis(kOnClosed);
			return 0;
		} else if(evt == SimpleCnn::CH_E::CH_WRITABLE) {
			auto r = mMsgTx.procOnWritable();
			if(r == MsgSender::kMsgDataNeeded) {
				mLis(kOnWritable);
			}
		} else {
			assert(0);
			return 1;
		}
	});
	mMsgTx.open(*mpCnn);
}

int Transport::sendMsg(BaseMsg& msg) {
	return mMsgTx.sendMsg(msg);
}

void Transport::setOnListener(std::function<void(TE)> lis) {
	mLis = lis;
}

std::unique_ptr<BaseMsg> Transport::fetchMsg() {
	auto pmsg = mpCnn->fetchMsg();
	return unique_ptr<BaseMsg>(pmsg);
}

std::string Transport::fetchData() {
	return move(mpCnn->fetchData());
}

} /* namespace cahttp */
