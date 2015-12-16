/*
 * CaHttp404UrlCtrl.h
 *
 *  Created on: Nov 12, 2015
 *      Author: root
 */

#ifndef EXTERNAL_CAHTTP_CAHTTP404URLCTRL_H_
#define EXTERNAL_CAHTTP_CAHTTP404URLCTRL_H_

#include "CaHttpUrlCtrl.h"
namespace cahttp {
class CaHttp404UrlCtrl : public CaHttpUrlCtrl {
public:
	CaHttp404UrlCtrl();
	virtual ~CaHttp404UrlCtrl();
	virtual void OnHttpReqMsg() override ;
};
}
#endif /* EXTERNAL_CAHTTP_CAHTTP404URLCTRL_H_ */
