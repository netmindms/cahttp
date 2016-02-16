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

#include "BaseConnection.h"
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

	BaseConnection::CnnIf* getCnnIf() {
		return &mCnnIf;
	}
	std::vector<std::string>& getPathParams();
private:
	class cnnif: public BaseConnection::CnnIf {
	public:
		cnnif(ReUrlCtrl* pctrl) {
			mpCtrl = pctrl;
		}
		virtual ~cnnif() {
			;
		}
		virtual int OnWritable();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg);
		virtual int OnData(std::string&& data);
		virtual int OnCnn(int cnnstatus);
		ReUrlCtrl* mpCtrl;
	};
	cnnif mCnnIf;
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
