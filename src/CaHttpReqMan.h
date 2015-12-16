/*
 * CaHttpReqMan.h
 *
 *  Created on: Dec 4, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPREQMAN_H_
#define SRC_CAHTTPREQMAN_H_
#include <list>
#include <stdint.h>

#include "HttpCnn.h"
namespace cahttp {
class CaHttpReqMan {
	friend class CaHttpReq;
public:
	struct cnninfo_t {
		uint32_t ip;
		int port;
		HttpCnn cnn;
	};
	CaHttpReqMan();
	virtual ~CaHttpReqMan();
	CaHttpReq* newRequest();
	void close();
private:
	std::list<cnninfo_t> mCnnList;
	std::list<CaHttpReq*> mReqList;
	std::list<CaHttpReq*> mDummyReqList;

	HttpCnn* getConnection(uint32_t ip, int port);
	void gotoDummyReq(CaHttpReq* preq);
	void clearDummyReq();
	void freeRequest(CaHttpReq* preq);
};
}
#endif /* SRC_CAHTTPREQMAN_H_ */
