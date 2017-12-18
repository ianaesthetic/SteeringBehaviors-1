#include "Unit.h"

bool Unit::Accumulate(FXMVECTOR f) {
	float remain = forceMax - XMVectorGetX(XMVector2Length(force));
	if (remain < 0.001f) return false; 
	float addMag = XMVectorGetX(XMVector2Length(f)); 
	if (addMag <= remain) force += f; 
	else 
		force += XMVector2Normalize(f) * remain;
	return true;
}

void Unit::Calc(float dt) {
	force = zero;
	if (!Accumulate(BaseBehaviours::Behaviours()->Avoid(this, dt))) return;
	if (isArrive) {
		if (!Accumulate(BaseBehaviours::Behaviours()->Arrive(this, XMLoadFloat2(&arrivePos), dt))) return; 
	}
	if (isWander) {
		if (!Accumulate(BaseBehaviours::Behaviours()->Wander(this, dt))) return;
	}
}

void Unit::Update(float dt) {
	Calc(dt);
	prevDis = XMVectorGetX(XMVector2Length(XMLoadFloat2(&velocity) * dt * detection));
	XMVECTOR tVelocity = force / mass;
	XMVECTOR pos = XMLoadFloat2(&position);
	XMStoreFloat2(&acceleration, tVelocity);
	tVelocity = tVelocity * dt;
	XMStoreFloat2(&velocity, XMLoadFloat2(&velocity) + tVelocity);
	if (velocity.x * velocity.x + velocity.y * velocity.y > velocityMax * velocityMax) {
		velocity.x = velocity.x / (velocity.x * velocity.x + velocity.y * velocity.y) * velocityMax;
		velocity.x = velocity.y / (velocity.x * velocity.x + velocity.y * velocity.y) * velocityMax;
	}
	pos = pos + XMLoadFloat2(&velocity) * dt;
	XMStoreFloat2(&wanderTarget, XMLoadFloat2(&wanderTarget) + XMLoadFloat2(&velocity) * dt);
	XMStoreFloat2(&position, pos);
	position.x = Repeat(position.x, -shape->GetRadius(), scene->GetSceneWidth() + shape->GetRadius()); 
	position.y = Repeat(position.y, -shape->GetRadius(), scene->GetSceneHeight() + shape->GetRadius());

	float tmp = velocity.x * velocity.x + velocity.y * velocity.y; 
	if (tmp > 0.0001) {
		XMStoreFloat2(&heading, XMVector2Normalize(XMLoadFloat2(&velocity)));
		tangent.x = -heading.y; 
		tangent.y = heading.x; 
	}
	backPos.x = position.x - shape->GetBackOffset() * heading.x; 
	backPos.y = position.y - shape->GetBackOffset() * heading.y; 
	XMStoreFloat2(&wanderCenter, pos + XMLoadFloat2(&heading) * wanderRadius * 2);

}

void Unit::Render() {
	shape->Draw(
		XMLoadFloat2(&heading),
		XMLoadFloat2(&tangent),
		XMLoadFloat2(&position),
		scene->GetRender(), 
		scene->GetRatio(),
		0
	);
	
	if (isWander) {
		this->scene->GetRender()->DrawRound(
			XMLoadFloat2(&this->wanderCenter) * this->scene->GetRatio(),
			this->wanderRadius * this->scene->GetRatio(),
			1
		);

		this->scene->GetRender()->DrawRound(
			XMLoadFloat2(&this->wanderTarget) * this->scene->GetRatio(),
			this->scene->GetRatio(),
			1
		);
	}
	
	XMVECTOR p[4];
	p[0] = XMVectorSet(0, -this->GetShape()->GetWidth() / 2.0f, 1, 0);
	p[1] = p[0] + XMVectorSet(0, this->GetShape()->GetWidth(), 0, 0);
	p[2] = p[0] + XMVectorSet(prevDis, 0, 0, 0);
	p[3] = p[1] + XMVectorSet(prevDis, 0, 0, 0);

	for (int i = 0; i < 4; ++i) {
		p[i] = LocalToWorld(
			p[i],
			XMLoadFloat2(&this->GetHeading()),
			XMLoadFloat2(&this->GetTangent()),
			XMLoadFloat2(&this->GetBackPos())
		);
	}

	this->scene->GetRender()->DrawRectangle(
		p[0] * this->scene->GetRatio(), 
		p[1] * this->scene->GetRatio(), 
		p[3] * this->scene->GetRatio(), 
		p[2] * this->scene->GetRatio(),
		2
	);
}

void Unit::ArriveOn(FXMVECTOR pos) {
	isArrive = true; 
	XMStoreFloat2(&arrivePos, pos); 
}

void Unit::ArriveOff() {
	isArrive = false; 
}

void Unit::WanderOn() {
	isWander = true; 
}

BaseBehaviours* BaseBehaviours::Instance = NULL;

XMVECTOR BaseBehaviours::Seek(Unit* master, FXMVECTOR pos) {
	XMVECTOR ret = pos - XMLoadFloat2(&master->GetPos());
	XMVector2Normalize(ret);
	ret *= master->VCeil();
	return ret - XMLoadFloat2(&master->Velocity());
}

XMVECTOR BaseBehaviours::Flee(Unit* master, FXMVECTOR pos) {
	XMVECTOR ret = -pos + XMLoadFloat2(&master->GetPos());
	XMVector2Normalize(ret);
	ret *= master->VCeil();
	return ret - XMLoadFloat2(&master->Velocity());
}

XMVECTOR BaseBehaviours::Arrive(Unit* master, FXMVECTOR pos, float dt) {
	XMVECTOR ret = pos - XMLoadFloat2(&master->GetPos());
	float dist;
	XMStoreFloat(&dist, XMVector2Length(ret));
	float speed = dist * 1.5f;
	speed = min(speed, master->VCeil());
	ret = (XMVector2Normalize(ret) * speed - XMLoadFloat2(&master->Velocity()));
	return ret * master->GetMass() / dt;
}

XMVECTOR BaseBehaviours::Avoid(Unit* master, float dt) {
	float length = master->GetDetection() * dt * XMVectorGetX(XMVector2Length(XMLoadFloat2(&master->Velocity()))); /* length squared */ 
	float width = master->GetShape()->GetWidth();
	master->GetGameWorld()->RectangleCollision(master, length);
	master->GetGameWorld()->Reset();
	BaseEntity* other; 
	XMVECTOR pos;

	XMVECTOR debugPos;

	float closestDis = inf;
	float radius = 0.0f; 
	bool isFind = false; 

	while (other = master->GetGameWorld()->GetNext()) {
		XMVECTOR tPos = XMVectorSet(other->GetPos().x, other->GetPos().y, 1, 0); 
		tPos = WorldToLocal(
			tPos,
			XMLoadFloat2(&master->GetHeading()),
			XMLoadFloat2(&master->GetTangent()), 
			XMLoadFloat2(&master->GetBackPos()) 
		); 
		if (XMVectorGetX(tPos) < 0.0f) continue; 
		float expandRadius = master->GetShape()->GetWidth() / 2.0f + other->GetShape()->GetRadius();
		if (fabs(XMVectorGetY(tPos)) > expandRadius) continue;
		float curDis = sqrt(expandRadius * expandRadius - XMVectorGetY(tPos) * XMVectorGetY(tPos));
		if (XMVectorGetX(tPos) - curDis < 0) curDis = 0;
		else curDis = XMVectorGetX(tPos) - curDis; 
		if (curDis < closestDis) {
			debugPos = XMVectorSet(other->GetPos().x, other->GetPos().y, 1, 0);
			pos = tPos; 
			closestDis = curDis; 
			radius = other->GetShape()-> GetRadius();
			isFind = true; 
		}
	} 
	if (!isFind) return zero;
	XMVECTOR ret = zero; 
	float xTweaker = 0.6f;
	float yTweaker = 1.0f + (length - closestDis) / length * 20;
	ret = XMVectorSetX(ret, -xTweaker * closestDis);
	if(XMVectorGetY(pos) > -0.1) ret = XMVectorSetY(ret, -yTweaker * (width / 2.0f - (XMVectorGetY(pos) - radius))); 
	else ret = XMVectorSetY(ret, +yTweaker * (XMVectorGetY(pos) + radius + width / 2.0f));

	ret = LocalToWorld(ret,
		XMLoadFloat2(&master->GetHeading()),
		XMLoadFloat2(&master->GetTangent()),
		XMVectorSet(0, 0, 0, 0)
	);
	return ret;

}

XMVECTOR BaseBehaviours::Wander(Unit* master, float dt) {
	XMVECTOR nextPos;
	XMVECTOR r = randomVector(master->GetRandomRange());
	XMFLOAT2 t; 
	XMStoreFloat2(&t, r);
	nextPos = XMLoadFloat2(&master->GetWanderTarget()) + r;
	nextPos = XMVector2Normalize(nextPos - XMLoadFloat2(&master->GetWanderCenter())) * master->GetWanderRadius()
		+ XMLoadFloat2(&master->GetWanderCenter());
	master->StoreWanderTarget(nextPos);
	return Arrive(master, nextPos, dt);
}

