/*
 * Transport.h
 *
 *  Created on: May 7, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_TRANSPORT_H_
#define CAHTTP_TRANSPORT_H_

#include <memory>
#include "BaseMsg.h"
#include "MsgSender.h"

namespace cahttp {

class Transport {
public:
	enum TE {
		kOnMsg,
		kOnData,
		kOnWritable,
		kOnClosed,
	};
	Transport();
	virtual ~Transport();
	void init(std::shared_ptr<SimpleCnn> pcnn);
	int sendMsg(BaseMsg& msg);
	void setOnListener(std::function<void (TE)> lis);
	std::unique_ptr<BaseMsg> fetchMsg();
	std::string fetchData();

private:
	std::shared_ptr<SimpleCnn> mpCnn;
	MsgSender mMsgTx;
	std::function<void (TE)> mLis;
};

} /* namespace cahttp */

#endif /* CAHTTP_TRANSPORT_H_ */
