/*
 * ReUrlCtrl.h
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REURLCTRL_H_
#define CAHTTP_REURLCTRL_H_

#include <string>
#include <vector>

#include "BaseMsg.h"
namespace cahttp {

class ReUrlCtrl {
	friend class ReHttpServer;
	friend class ReHttpSvrCtx;
public:
	ReUrlCtrl();
	virtual ~ReUrlCtrl();
	virtual void OnMsgHdr(BaseMsg& msg);
	virtual void OnMsg(BaseMsg& msg);
	virtual void OnData(std::string&& data);
	virtual void OnEnd();

	std::vector<std::string>& getPathParams();
private:
	BaseMsg* mpReqMsg;

	std::vector<std::string> mPathParams;
	void setPathParams(std::vector<std::string>&& vs) {
		mPathParams = move(vs);
	}

	void init(upBaseMsg upmsg);

};

typedef std::unique_ptr<ReUrlCtrl> upReUrlCtrl;

} /* namespace cahttp */

#endif /* CAHTTP_REURLCTRL_H_ */
