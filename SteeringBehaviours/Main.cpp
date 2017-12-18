#include "GameTimer.h"
#include "Unit.h"
#include "filein.h"

#pragma warning(disable:4316)

int BaseEntity::idNum = 0;

class MainWindow : public BaseWindow<MainWindow>
{
public:
	PCWSTR  ClassName() const { return L"Game Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(m_hwnd, &ps);
	}
	return 0;

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
	return TRUE;
}

class Main {
public:
	Main() :
		world(100, 100, 9, &canvas) {
		width = 900;
		height = 900;
	}
	~Main() {
		for (int i = 0; i < (int)unit.size(); ++i) {
			if (unit[i]) delete unit[i]; 
		}
		for (int i = 0; i < (int)obstacle.size(); ++i) {
			if (obstacle[i]) delete obstacle[i];
		}
	}

	bool Init(int nCmdShow) {
		srand((int)time(0));
		if (!win.Create(L"Steering Behaviour", WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, CW_USEDEFAULT, width, height))
		{
			return 0;
		}

		ShowWindow(win.Window(), nCmdShow);

		if (!canvas.Init(win.Window())) {
			MessageBox(NULL, L"Canvas Error!", 0, 0);
			return 0; 
		}
		return 1; 
	}

	void Update(float dt);
	void Render();
	
	/* interface */

	GameWorld* GetCurrentWorld() {
		return &world;
	}
	GameTimer* GetTimer() {
		return &timer;
	}
	void SetFileIn(std::string t) {
		file.Init(t); 
	}
	void ReadIn();

private:
	MainWindow win;
	Direct2d canvas;
	GameWorld world; 
	GameTimer timer;	
	FileIn file; 
	std::vector<Round> round; 
	std::vector<Triangle> triangle; 
	std::vector<Unit*> unit;
	std::vector<Obstacle*> obstacle;
	int width;
	int height;
};

void Main::Update(float dt) {
	canvas.CountReset(); 
	canvas.TimeSpanAdd(dt);
	world.Update(dt); 
}

void Main::Render() {
	canvas.Begin();
	canvas.DisplayInfo();
	world.Render();
	canvas.End();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	BaseBehaviours::Behaviours();
	Main main;
	if (!main.Init(nCmdShow)) return 0;
	
	main.SetFileIn(std::string("data.txt"));
	main.ReadIn(); 

	MSG msg = {};
	main.GetTimer()->Reset();
	GetMessage(&msg, NULL, 0, 0);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			main.GetTimer()->Tick();
			main.Update(main.GetTimer()->GetDeltaTime());
			main.Render();
		}
	}
	return 0;
}

void Main::ReadIn() {
	file.cin = (char)getchar(); 
	std::string t; 
	while (file.cin != EOF) {
		if (file.IsSkip(file.cin)) {
			file.cin = (char)getchar();
			continue;
		}
		
		if (file.cin == '#') {
			t = file.StringIn(); 

			if (t == std::string("round")) {
				float radius; 
				radius = file.FloatIn();
				round.push_back(Round(radius));
			}

			else if (t == std::string("triangle")) {
				float angle = pi / file.FloatIn();
				float scale = file.FloatIn();
				triangle.push_back(Triangle(angle, scale));
			}

			else if (t == std::string("unit")) {
				XMFLOAT2 position; 
				position.x = file.FloatIn();
				position.y = file.FloatIn();
				XMFLOAT2 vStart; 
				vStart.x = file.FloatIn(); 
				vStart.y = file.FloatIn(); 
				XMFLOAT2 heading; 
				heading.x = vStart.x / sqrt(vStart.x * vStart.x + vStart.y * vStart.y);
				heading.y = vStart.y / sqrt(vStart.x * vStart.x + vStart.y * vStart.y);
				float vCeil = file.FloatIn(); 
				float fCeil = file.FloatIn(); 
				float mass = file.FloatIn(); 
				int flag = file.IntIn(); 
			
				unit.push_back(new Unit(
					position,
					vStart, 
					heading,
					vCeil,
					fCeil,
					mass,
					this->GetCurrentWorld(),
					&triangle[flag]
				));
			}

			else if (t == std::string("obstacle")) {
				XMFLOAT2 position;
				position.x = file.FloatIn();
				position.y = file.FloatIn();
				int flag = file.IntIn(); 

				obstacle.push_back(new Obstacle(
					position,
					XMFLOAT2(1, 0), 
					this->GetCurrentWorld(),
					&round[flag]
				));
			}
			
			else if (t == std::string("state")) {
				int id = file.IntIn(); 
				std::string state = file.StringIn();
				if (state == std::string("wander")) unit[id]->WanderOn(); 
			}
		}
		file.cin = (char)getchar(); 
	}
	fclose(stdin);
}
