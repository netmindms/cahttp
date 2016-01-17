/*
 * CaHttp404UrlCtrl.cpp
 *
 *  Created on: Nov 12, 2015
 *      Author: root
 */

#include "CaHttp404UrlCtrl.h"

namespace cahttp {
CaHttp404UrlCtrl::CaHttp404UrlCtrl() {
	// TODO Auto-generated constructor stub

}

CaHttp404UrlCtrl::~CaHttp404UrlCtrl() {
	// TODO Auto-generated destructor stub
}

void CaHttp404UrlCtrl::OnHttpReqMsg() {
	response(404);
}
}
