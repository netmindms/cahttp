/*
 * NotFoundUrl.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#include "NotFoundUrl.h"

namespace cahttp {

NotFoundUrl::NotFoundUrl() {
	// TODO Auto-generated constructor stub

}

NotFoundUrl::~NotFoundUrl() {
	// TODO Auto-generated destructor stub
}

void NotFoundUrl::OnHttpReqMsg(BaseMsg& msg) {
	response(404);
}

} /* namespace cahttp */
