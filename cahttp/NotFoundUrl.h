/*
 * NotFoundUrl.h
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_NOTFOUNDURL_H_
#define CAHTTP_NOTFOUNDURL_H_
#include "ReUrlCtrl.h"

namespace cahttp {

class NotFoundUrl : public ReUrlCtrl {
public:
	NotFoundUrl();
	virtual ~NotFoundUrl();
	void OnHttpReqMsg(BaseMsg &msg) override;
};

} /* namespace cahttp */

#endif /* CAHTTP_NOTFOUNDURL_H_ */
