/*
 * CBounder.h
 *
 *  Created on: 02.07.2011
 *      Author: gerstrong
 */

#ifndef CBOUNDER_H_
#define CBOUNDER_H_

#include "engine/galaxy/ai/CStunnable.h"
#include "engine/galaxy/ai/platform/CPlatform.h"

namespace galaxy {

class CBounder : public CStunnable, public CPlatform
{
public:
	CBounder(CMap *pmap, Uint32 x, Uint32 y);

	void getTouchedBy(CObject &theObject);
	void processBounce();

	void movePlayerUp(const int amnt);
	void movePlayerDown(const int amnt);

	void process();

private:
	int bounceboost;
};

};

#endif /* CBOUNDER_H_ */
