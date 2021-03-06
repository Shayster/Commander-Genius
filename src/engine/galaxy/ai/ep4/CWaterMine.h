/*
 * CWaterMine.h
 *
 *  Created on: 30.07.2011
 *      Author: gerstrong
 *
 *  The Watermines are normally in the well of wishes
 */

#ifndef CWATERMINE_H_
#define CWATERMINE_H_

#include "common/CObject.h"

namespace galaxy {

class CWaterMine: public CObject {
public:
	CWaterMine(CMap *pmap, Uint32 x, Uint32 y, const bool vertical);

	void process();

	// What happens when some object touches another one
	void getTouchedBy(CObject &theObject);

private:

	void processMove();
	void processExplode();

	void (CWaterMine::*mp_processState)();
};

} /* namespace galaxy */
#endif /* CWATERMINE_H_ */
