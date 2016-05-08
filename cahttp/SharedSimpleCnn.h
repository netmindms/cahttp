/*
 * SharedBaseCnn.h
 *
 *  Created on: Apr 13, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_SHAREDSIMPLECNN_H_
#define CAHTTP_SHAREDSIMPLECNN_H_

#include "SimpleCnn.h"

namespace cahttp {

class SharedBaseCnn: public SimpleCnn {
public:
	SharedBaseCnn();
	virtual ~SharedBaseCnn();

};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDSIMPLECNN_H_ */
