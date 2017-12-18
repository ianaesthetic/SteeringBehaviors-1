#include "GameWorld.h"

GameWorld::GameWorld(int _width, int _height, float _ratio, Direct2d* _render) {
	width = _width;
	height = _height;
	ratio = _ratio;
	pRender = _render;
	entityArray.clear();
}

void GameWorld::RectangleCollision(BaseEntity* t, float length) {
	collisionArray.clear();
	XMVECTOR p[4];
	p[0] = XMVectorSet(0, -t->GetShape()->GetWidth() / 2.0f, 1, 0);
	p[1] = p[0] + XMVectorSet(0, t->GetShape()->GetWidth(), 0, 0);
	p[3] = p[0] + XMVectorSet(length, 0, 0, 0);
	p[2] = p[1] + XMVectorSet(length, 0, 0, 0);

	for (int i = 0; i < 4; ++i) {
		p[i] = LocalToWorld(
			p[i],
			XMLoadFloat2(&t->GetHeading()),
			XMLoadFloat2(&t->GetTangent()),
			XMLoadFloat2(&t->GetBackPos())
		);
	}

	for (int i = 0; i < (int)entityArray.size(); ++i) {
		if (entityArray[i]->GetID() == t->GetID()) continue;
		bool isIn = true;
		for (int j = 0; j < 4; ++j) {
			if (InterSect(p[j], p[(j + 1) % 4], XMLoadFloat2(&entityArray[i]->GetPos()), entityArray[i]->GetShape()->GetRadius())) {
				collisionArray.push_back(entityArray[i]);
				isIn = false;
				break;
			}
		}
	}
}

void GameWorld::RoundCollision(BaseEntity* t, float length) {
	collisionArray.clear();
	for (int i = 0; i < (int)entityArray.size(); ++i) {
		if (entityArray[i]->GetID() == t->GetID()) continue; 
		if (Distance(XMLoadFloat2(&t->GetPos()), XMLoadFloat2(&entityArray[i]->GetPos())) < length * length) {
			collisionArray.push_back(entityArray[i]); 
		}
	}
}

void GameWorld::add(BaseEntity* s) {
	entityArray.push_back(s);
}
void GameWorld::Update(float dt) {
	for (int i = 0; i < (int)entityArray.size(); ++i) {
		entityArray[i]->Update(dt);
	}
}
void GameWorld::Render() {
	for (int i = 0; i < (int)entityArray.size(); ++i) {
		entityArray[i]->Render();
	}
}
float GameWorld::GetRatio() {
	return ratio;
}
Direct2d* GameWorld::GetRender() {
	return pRender;
}

float GameWorld::GetSceneWidth() {
	return static_cast<float>(width);
}

float GameWorld::GetSceneHeight() {
	return static_cast<float>(height);
}

void GameWorld::Reset() {
	iterator = 0;
}

BaseEntity* GameWorld::GetNext() {
	if (iterator >= (int)collisionArray.size()) return NULL;
	else return collisionArray[iterator++];
}