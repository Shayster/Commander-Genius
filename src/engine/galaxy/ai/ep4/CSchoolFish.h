/*
 * CSchoolFish.h
 *
 *  Created on: 01.08.2011
 *      Author: gerstrong
 *
 *  Innocent small fish creatures which just follow Keen
 */

#ifndef CSCHOOLFISH_H_
#define CSCHOOLFISH_H_

#include "common/CObject.h"

namespace galaxy
{

class CSchoolFish : public CObject {
public:
	CSchoolFish(CMap *pmap, Uint32 x, Uint32 y);

	bool isNearby(CObject &theObject);

	void process();

private:
	int m_moveSpeed;
};

} /* namespace galaxy */
#endif /* CSCHOOLFISH_H_ */
