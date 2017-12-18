#pragma once
#include "GameWorld.h"

class Unit : public BaseEntity{
public:
	
	Unit(
		XMFLOAT2 position,
		XMFLOAT2 vStart,
		XMFLOAT2 headingStart,
		float _velocityMax,
		float _forceMax,
		float _mass,
		GameWorld* _scene,
		Shape* _shape
	) :
		velocityMax(_velocityMax),
		forceMax(_forceMax),
		mass(_mass),
		BaseEntity(position, headingStart, _shape, _scene),
		detection(100), 
		isWander(false),
		isSeek(false), 
		isArrive(false) {
		force = { 10.0f, 10.0f, 0.0f };
		velocity = vStart; 
		heading = headingStart; 
		XMStoreFloat2(&acceleration, zero); 
		
		wanderRadius = shape->GetRadius(); 
		XMStoreFloat2(&wanderTarget, XMLoadFloat2(&position) + wanderRadius * XMLoadFloat2(&heading) * 3);
		XMStoreFloat2(&wanderCenter, XMLoadFloat2(&position) + wanderRadius * XMLoadFloat2(&heading) * 2);
		RandomRange = wanderRadius / 3;

		scene->add(this);
	}
	
	/*game loop*/
	void Update(float dt);
	void Render(); 

	/* interface */
	float VCeil() { return velocityMax; }
	float GetWanderRadius() { return wanderRadius; }
	float GetRandomRange() { return RandomRange; }
	float GetMass() { return mass; }
	float GetDetection() { return detection; }
	XMFLOAT2 Velocity() { return velocity; }
	XMFLOAT2 GetWanderTarget() { return wanderTarget; }
	XMFLOAT2 GetWanderCenter() { return wanderCenter; }

	/* update */
	void StoreWanderTarget(FXMVECTOR nextPos) {	XMStoreFloat2(&wanderTarget, nextPos); }
	bool Accumulate(FXMVECTOR f);
	void Calc(float dt);

	/* state */ 
	void ArriveOn(FXMVECTOR pos); 
	void ArriveOff(); 
	void WanderOn();

protected:
	/* attribute */ 
	XMFLOAT2 velocity;
	XMFLOAT2 acceleration;
	XMFLOAT2 wanderTarget;
	XMFLOAT2 wanderCenter;
	XMVECTOR force; 
	float velocityMax; 
	float forceMax;
	float mass;
	float wanderRadius; 
	float RandomRange;
	float prevDis;
	float detection;

	/* state */ 
	bool isWander;
	bool isSeek;
	XMFLOAT2 seekPos; 
	bool isArrive;
	XMFLOAT2 arrivePos; 
};

class BaseBehaviours {
public:

	BaseBehaviours() {}

	static BaseBehaviours* Behaviours() {
		if (Instance == NULL) {
			Instance = new BaseBehaviours();
		}
		return Instance;
	}

	~BaseBehaviours() {
		if (Instance) delete Instance;
	};


	XMVECTOR Seek(Unit* master, FXMVECTOR pos);
	XMVECTOR Arrive(Unit* master, FXMVECTOR pos, float dt);
	XMVECTOR Flee(Unit* master, FXMVECTOR pos);
	XMVECTOR Wander(Unit* master, float dt);
	XMVECTOR Avoid(Unit* master, float dt);					



private:
	static BaseBehaviours* Instance;

};

class Obstacle : public BaseEntity {
public:
	Obstacle(
		XMFLOAT2 pos,
		XMFLOAT2 headingStart,
		GameWorld* _scene,
		Shape* shape
	) : BaseEntity(pos, headingStart, shape, _scene) {
		scene->add(this);
	}

	void Update(float dt) {}
	void Render() {
		shape->Draw(
			XMLoadFloat2(&heading),
			XMLoadFloat2(&tangent),
			XMLoadFloat2(&position),
			scene->GetRender(),
			scene->GetRatio(),
			0
		);
	}
};


/*
#state
0 wander
#state
1 wander
#state
2 wander
#state
3 wander
*/
