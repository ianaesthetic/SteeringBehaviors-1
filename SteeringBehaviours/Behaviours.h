#pragma once
#include "Unit.h"

class BaseBehaviours {
public:

	static BaseBehaviours* Behaviours() {
		if (Instance == NULL) {
			Instance = Behaviours();
		}
		return Instance; 
	}

	~BaseBehaviours() {
		if (Instance) delete Instance;
	};


	XMVECTOR Seek(Unit* master, FXMVECTOR pos); 
	XMVECTOR Arrive(Unit* master, FXMVECTOR pos); 
	XMVECTOR Flee(Unit* master, FXMVECTOR pos);
	XMVECTOR Hide(Unit* master, Unit* target);
	XMVECTOR Pursuit(Unit* master, Unit* target);
	XMVECTOR Wander(Unit* master);

	XMVECTOR Avoid(Unit* master);					 // state 0 
	XMVECTOR WallDetection(Unit* master);			 // state 1
	XMVECTOR Seperation(Unit* master);				 // state 2
	XMVECTOR Alignment(Unit* master);				 // state 3
	XMVECTOR Cohesion(Unit* master);				 // state 4 

private:
	static BaseBehaviours* Instance;
};
