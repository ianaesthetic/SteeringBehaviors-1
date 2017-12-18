#pragma

#include "Util.h"
#include "WinClass.h"

class GameWorld; 

class Shape {
public:
	Shape(float _scale) : scale(_scale) {};
	float GetWidth() { return width; }
	float GetRadius() { return scale; }
	float GetBackOffset() { return backOffset; }
	virtual void Draw(FXMVECTOR _heading, FXMVECTOR _tangent, FXMVECTOR _position, Direct2d* render, float ratio, int f) = 0;

protected:
	float scale;
	float width;
	float backOffset; 
};

class Triangle : public Shape {
public:
	Triangle(float _angle, float _scale) :
		Shape(_scale), angle(_angle) {
		width = sinf(angle) * scale * 4;
		backOffset = cos(angle) * scale; 
	}
	void Draw(FXMVECTOR heading, FXMVECTOR tangent, FXMVECTOR position, Direct2d* render, float ratio, int f) {
		XMVECTOR p0, p1, p2;
		p0 = XMVectorSet(scale, 0, 1, 0);
		p1 = XMVectorSet(-cosf(angle) * scale, sinf(angle) * scale, 1, 0);
		p2 = XMVectorSet(-cosf(angle) * scale, -sinf(angle) * scale, 1, 0);
		p0 = LocalToWorld(p0, heading, tangent, position); 
		p1 = LocalToWorld(p1, heading, tangent, position);
		p2 = LocalToWorld(p2, heading, tangent, position); 
		render->DrawTriangle(p0 * ratio, p1 * ratio, p2 * ratio, f);
	}

private:
	float angle;
};

class Round : public Shape {
public:
	Round(float scale) : Shape(scale){
		width = scale;
		backOffset = scale; 
	}
	void Draw(FXMVECTOR heading, FXMVECTOR tangent, FXMVECTOR position, Direct2d* render, float ratio, int f) {
		render->DrawRound(position * ratio, scale * ratio, f);
	}
private:
};

class BaseEntity {
public:
	BaseEntity(XMFLOAT2 _pos, XMFLOAT2  _heading, Shape* _shape, GameWorld* _scene) : shape(_shape), scene(_scene) {
		position = _pos;
		heading = _heading; 
		id = ++idNum;
		tangent.x = -heading.y;
		tangent.y = heading.x;
		backPos = XMFLOAT2(
			position.x - heading.x * shape->GetBackOffset(), 
			position.y - heading.y * shape->GetBackOffset() 
		);
	}
	
	/* gameloop */
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;

	/* interface */
	int GetID() { return id; }
	GameWorld* GetGameWorld() { return scene; }
	Shape* GetShape() { return shape; }
	XMFLOAT2 GetPos() { return position; }
	XMFLOAT2 GetHeading() { return heading; }
	XMFLOAT2 GetTangent() { return tangent; }
	XMFLOAT2 GetBackPos() { return backPos; }

protected:
	int id;
	static int idNum;

	XMFLOAT2 position;
	XMFLOAT2 tangent;
	XMFLOAT2 heading;
	XMFLOAT2 backPos; 
	GameWorld* scene;	
	Shape* shape;
};


class GameWorld {
public:

	GameWorld(int _width, int _height, float _ratio, Direct2d* _render);
	
	/* gameloop */
	void Update(float dt);
	void Render();
	void RectangleCollision(BaseEntity* t, float length);
	void RoundCollision(BaseEntity* t, float radius);

	/* interface */
	Direct2d* GetRender();
	float GetSceneWidth();
	float GetSceneHeight();
	float GetRatio();

	/* iterator */	
	void add(BaseEntity* s);
	void Reset();
	BaseEntity* GetNext();

private:
	int width; 
	int height;
	float ratio; 

	int iterator; 
	std::vector<BaseEntity*> entityArray; 
	std::vector<BaseEntity*> collisionArray; 
	Direct2d* pRender; 
};
