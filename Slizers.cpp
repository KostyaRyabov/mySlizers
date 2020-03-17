#include "ConsoleGameEngine.h"
#include <map>
#include <stack> 
#include <random>

std::random_device dev;
std::mt19937 rng(dev());
int GetRandom(int MIN, int MAX) {
	std::uniform_int_distribution<std::mt19937::result_type> RandNum(MIN, MAX);

	return RandNum(rng);
}

enum GameObjectsID{
	SPACE	= 0,
	FOOD	= 255				//size = 1
};
enum GameProperties {
	MapHeight		= 200,
	MapWidth		= 200,
	WindowHeight	= 100,
	WindowWidth		= 100,

	RespawnArea		= 3,
	RadiusCheck		= 20,
	DistForAtack	= 5,
	CheckStap		= 2,

	resolution_X	= 4,
	resolution_Y	= 4,
	
	StartSize		= 5,
	SnakeMaxCount	= 10,
	MaxFoodCount	= SnakeMaxCount * StartSize,

	TimeOfFoodEmersion		= 200,		// milliseconds
	TimeOfFrameChange		= 50
};
enum DirectionMove {
	Right		= 1,
	UpRight		= 2,
	Up			= 3,
	UpLeft		= 4,
	Left		= 5,
	DownLeft	= 6,
	Down		= 7,
	DownRight	= 8,

	Idle		= 0
};
enum Keys {
	LeftKey		= 37,
	UpKey		= 38,
	RightKey	= 39,
	DownKey		= 40,

	SpaceBar	= 32,

	ESC = 27
};

int Dist(POINT A, POINT B) {
	//return std::sqrt(std::pow(A.x - (A.x > MapWidth/2)? MapWidth / 2 : 0 - B.x - (B.x > MapWidth / 2) ? MapWidth / 2 : 0 ,2) + std::pow(A.y - (A.y > MapHeight / 2) ? MapHeight / 2 : 0 - B.y - (B.y > MapHeight / 2) ? MapHeight / 2 : 0,2));
	return std::sqrt(std::pow(A.x - B.x,2) + std::pow(A.y - B.y,2));
}
POINT SUM_point(POINT &A, POINT &B) {
	return { A.x + B.x, A.y + B.y};
}

byte* Map = new byte[MapWidth * MapHeight];
bool EndGame = false;

void RandomFoodGenerator() {
	while (!EndGame) {
		Map[MapWidth * GetRandom(0, MapHeight) + GetRandom(0, MapWidth)] = FOOD;

		Sleep(TimeOfFoodEmersion);
	}
}

class Snake
{
public:
	byte id;
	bool boosted = false;

	byte direction;

	std::list<POINT> path;
private:
	POINT offset, tmp_point;

	std::list<POINT>::reverse_iterator it;
	byte direction_current, counter, rotator, tmp_byte, thickness, MAX_thickness, distance, newDirection, d, ray_dir, Rotor_atack, target;
	char atack;
	char Ray_count;

	void Analise();

	void LeaveFood() {
		offset = { 0, 0 };

		if (tmp_point.x - thickness < 0) offset.x = - tmp_point.x + thickness;
		if (tmp_point.y - thickness < 0) offset.y = - tmp_point.y + thickness;

		if (tmp_point.x + thickness >= MapWidth) offset.x = MapWidth - tmp_point.x - thickness;
		if (tmp_point.y + thickness >= MapHeight) offset.y = MapHeight - tmp_point.y - thickness;

		 Map[MapWidth * GetRandom(tmp_point.y - thickness + offset.y, tmp_point.y + thickness + offset.y - 1) + GetRandom(tmp_point.x - thickness + offset.x, tmp_point.x + thickness + offset.x)] = FOOD;
	}

	void ClearOldPath() {
		thickness+=3;

		while (!path.empty())
		{
			tmp_point = path.front();
			
			offset = { -thickness,-thickness };
			while (offset.y <= thickness)
			{
				if (Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth] == id)
					Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth] = SPACE;

				if (++offset.x > thickness) {
					offset.y++;
					offset.x = -thickness;
				}
			}

			if (path.size() % 2 == 1) LeaveFood();

			path.pop_front();
		}
	};

	byte NumCollision(POINT host) {
		offset = { -RespawnArea, -RespawnArea };

		byte test;

		while (offset.y < RespawnArea) {
			if (host.y + offset.y < 0) host.y = RespawnArea;
			else if (host.y + offset.y > MapHeight) host.y = MapHeight - RespawnArea;

			if (host.x + offset.x < 0) host.x = RespawnArea;
			else if (host.x + offset.x > MapWidth) host.x = MapWidth - RespawnArea;

			test = Map[MapWidth * (host.y + offset.y) + (host.x + offset.x)];
			if (test != id || test != FOOD || test != SPACE) {
				return false;
			}

			if (++offset.x > RespawnArea) { offset.x = -RespawnArea; ++offset.y; };
		}

		return true;
	};

	bool Stap(bool checker = true) {
		if (checker) {
			thickness = std::pow(path.size(), 0.25f);
			offset = { -thickness,-thickness };

			if (boosted) {
				if (MAX_thickness < thickness + 1) MAX_thickness = thickness + 1;
			} else MAX_thickness = thickness + 1;
			
			while (offset.y <= thickness)
			{
				if ((std::abs(offset.x) + std::abs(offset.y)) <= thickness * 1.5f) {
					tmp_byte = Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth];
					
					if (tmp_byte == FOOD)
						path.push_back(path.back());
					else if (tmp_byte != id) {
						if (tmp_byte != SPACE){
							Stap(false);
							Respawn();
							return false;
						}
					}

					Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth] = id;
				}

				if (++offset.x > thickness) {
					offset.y++;
					offset.x = -thickness;
				}
			}
		}
		else {
			offset = { -MAX_thickness,-MAX_thickness };
			
			while (offset.y <= MAX_thickness)
			{
				for (it = path.rbegin(); it != path.rend(); it++)
					if (Dist(*it, SUM_point(tmp_point, offset)) < MAX_thickness) break;

				if (it == path.rend())				
					if ((std::abs(offset.x) + std::abs(offset.y)) <= MAX_thickness * 1.5f)
						if (Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth] == id)
							Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth] = SPACE;

				if (++offset.x > MAX_thickness) {
					offset.y++;
					offset.x = -MAX_thickness;
				}
			}
		}

		return true;
	}

	char OnSameDirectionWith(Snake &snake) {
		for (d = direction_current - 1; d <= direction_current + 1; d++) {
			if (d + (d < 1)?8:0 == snake.direction_current) return 0;
		}

		if ((snake.direction_current - direction_current) < (8 + direction_current - snake.direction_current)) return 1;
		else return -1;
	}

	void FollowBy(byte id);
public:	
	Snake(byte ID) {
		id = ID;
		Respawn();
	}

	bool Move() {
		tmp_point = path.back();
		path.pop_back();

		Stap(false);
		
		tmp_point = path.front();

		if (direction != Idle) {
			if (direction < direction_current)
			{
				if (++rotator > thickness) {
					rotator = 0;
					if ((direction_current - direction) > (8 + direction - direction_current)) direction_current++;
					else direction_current--;
				}
			}
			else if (direction > direction_current)
			{
				if (++rotator > thickness) {
					rotator = 0;
					if ((direction - direction_current) < (8 + direction_current - direction)) direction_current++;
					else direction_current--;
				}
			}

			if (direction_current > 8) direction_current = 1;
			else if (direction_current < 1) direction_current = 8;
		}

		switch (direction_current)
		{
		case UpRight:
			tmp_point.y--;
		case Right:
			tmp_point.x++;
			break;
		case UpLeft:
			tmp_point.x--;
		case Up:
			tmp_point.y--;
			break;
		case DownLeft:
			tmp_point.y++;
		case Left:
			tmp_point.x--;
			break;
		case DownRight:
			tmp_point.x++;
		case Down:
			tmp_point.y++;
			break;
		}

		if (tmp_point.x >= MapWidth) tmp_point.x = 0;
		if (tmp_point.y >= MapHeight) tmp_point.y = 0;
		if (tmp_point.x < 0) tmp_point.x = MapWidth - 1;
		if (tmp_point.y < 0) tmp_point.y = MapHeight - 1;

		if (!Stap()) return false;

		path.push_front(tmp_point);

		if (boosted) {
			if (path.size() > StartSize) {
				switch (direction_current)
				{
				case UpRight:
					tmp_point.y--;
				case Right:
					tmp_point.x++;
					break;
				case UpLeft:
					tmp_point.x--;
				case Up:
					tmp_point.y--;
					break;
				case DownLeft:
					tmp_point.y++;
				case Left:
					tmp_point.x--;
					break;
				case DownRight:
					tmp_point.x++;
				case Down:
					tmp_point.y++;
					break;
				}

				if (tmp_point.x >= MapWidth) tmp_point.x = 0;
				if (tmp_point.x < 0) tmp_point.x = MapWidth - 1;
				if (tmp_point.y >= MapHeight) tmp_point.y = 0;
				if (tmp_point.y < 0) tmp_point.y = MapHeight - 1;

				if (!Stap()) return false;

				path.push_front(tmp_point);
				if (!Stap()) return false;

				tmp_point = path.back();
				path.pop_back();
				Stap(false);

				if (++counter > thickness)
				{
					tmp_point = path.back();
					path.pop_back();
					Stap(false);

					LeaveFood();
					counter = 0;
				}
			}
		}

		return true;
	}

	void AutoMove() {
		//direction = GetRandom(1, 8);
		
		Analise();

		Move();
	}

	void Respawn();

	~Snake() {
		
	}

};

std::vector<Snake> Players;



void AddSnake() {
	int lastID = 1;

	//getting a new ID

	if (!Players.empty())	lastID = Players.back().id + 1;

	Snake snake(lastID);
	Players.push_back(snake);
};
void Snake::Respawn() {
	ClearOldPath();
	
	newDirection = 0;
	target = 255;
	rotator = 0;
	counter = 0;
	atack = 0;
	direction_current = GetRandom(1,8);

	POINT respPoint;

	do {
		respPoint = { GetRandom(RadiusCheck, MapWidth - RadiusCheck - 1), GetRandom(RadiusCheck, MapHeight - RadiusCheck - 1)};
	} while (NumCollision(respPoint));

	while (path.size() < StartSize) {
		path.push_back({ respPoint.x,respPoint.y});
	}
}
void Snake::FollowBy(byte id) {
	tmp_point = { Players[id].path.front().x - path.front().x, Players[id].path.front().y - path.front().y };

	if (std::abs(tmp_point.x) > MapWidth / 2) tmp_point.x *= -1;
	if (std::abs(tmp_point.y) > MapHeight / 2) tmp_point.y *= -1;


	if (tmp_point.x == 0) {
		if (tmp_point.y > 0) direction = Down;
		else direction = Up;
	}
	else {
		if (tmp_point.y == 0) {
			if (tmp_point.x > 0) direction = Right;
			else direction = Left;
		}
		else {
			if (tmp_point.y > 0) {
				if (tmp_point.x > 0) direction = DownRight;
				else direction = DownLeft;
			}
			else {
				if (tmp_point.x > 0) direction = UpRight;
				else direction = UpLeft;
			}
		}
	}
}
void Snake::Analise() {
	if (target != 255) {
		if (atack == 0) FollowBy(target);
		else {
			if (Rotor_atack++ > 10) {
				if (Rotor_atack % 4 == 0) {
					direction += atack - 1;

					if (direction > 8) direction = 1;
					else if (direction < 1) direction = 8;
				}

				if (Rotor_atack > 15) {
					atack = 0;
					boosted = false;
					target = 255;
				}
			}
		}
	}
	else {
		direction += GetRandom(0, 2) - 1;

		if (direction > 8) direction = 1;
		else if (direction < 1) direction = 8;
	}

	tmp_point = path.front();

	for (Ray_count = -2; Ray_count <= 2; Ray_count++) {
		offset = { 0,0 };

		ray_dir = direction_current + Ray_count + ((direction_current + Ray_count > 8) ? -8 : (direction_current + Ray_count < 1) ? 8 : 0);


		for (distance = 1; distance <= RadiusCheck; distance += CheckStap) {
			switch (ray_dir)
			{
			case UpRight:
				offset.y -= CheckStap;
			case Right:
				offset.x += CheckStap;
				break;
			case UpLeft:
				offset.x -= CheckStap;
			case Up:
				offset.y -= CheckStap;
				break;
			case DownLeft:
				offset.y += CheckStap;
			case Left:
				offset.x -= CheckStap;
				break;
			case DownRight:
				offset.x += CheckStap;
			case Down:
				offset.y += CheckStap;
				break;
			}

			tmp_byte = Map[MapWidth * ((tmp_point.y + offset.y + (((tmp_point.y + offset.y) < 0) ? MapHeight : 0)) % MapHeight) + (tmp_point.x + offset.x + (((tmp_point.x + offset.x) < 0) ? MapWidth : 0)) % MapWidth];
			if (tmp_byte != FOOD && tmp_byte != id && tmp_byte != SPACE) {
				if (target == 255) {
					target = tmp_byte;
				}
					
				if (target == tmp_byte) {
					if (atack == 0)
						if (Dist(Players[target].path.front(),tmp_point) < DistForAtack && path.size() > StartSize + DistForAtack) {
							boosted = true;
							atack = OnSameDirectionWith(Players[target])+1;
							direction -= 3 * (atack - 1) + ((direction - 3 * (atack - 1) > 8) ? -8 : (direction - 3 * (atack - 1) < 1) ? 8 : 0);
							Rotor_atack = 0;
							break;
						}
				}

				if (distance < thickness + Players[target].thickness + 10) {
					if (Ray_count != 0)	newDirection -= Ray_count / std::abs(Ray_count);
				}

				break;
			}
		}
	}

	direction += newDirection + ((direction + newDirection > 8) ? -8 : (direction + newDirection < 1) ? 8 : 0);
	newDirection = 0;
}

class GameEngine : public olcConsoleGameEngine
{
private:
	int x, y, i;
	float timer = 0;
	std::string nums[11];
	POINT offset;

	void DrowNum(int NUMBER) {
		for (i = 0; NUMBER > 0; i++, NUMBER /= 10)
			for (y = 0; y < 5; y++)
				for (x = 0; x < 4; x++)
					if (nums[NUMBER%10][4*y+x]=='.')
						Draw(WindowWidth - 5 + x - i*4, WindowHeight - 6 + y, PIXEL_THREEQUARTERS, FG_WHITE);
	}
public:
	GameEngine() {
		m_sAppName = L"Slizers";
		
		memset(Map, 0, MapWidth * MapHeight * sizeof(byte));

		nums[0].append("....");
		nums[0].append(".  .");
		nums[0].append(".  .");
		nums[0].append(".  .");
		nums[0].append("....");

		nums[1].append("  . ");
		nums[1].append(" .. ");
		nums[1].append(". . ");
		nums[1].append("  . ");
		nums[1].append("....");

		nums[2].append(" .. ");
		nums[2].append(".  .");
		nums[2].append("  . ");
		nums[2].append(" .  ");
		nums[2].append("....");

		nums[3].append("... ");
		nums[3].append("   .");
		nums[3].append(" .. ");
		nums[3].append("   .");
		nums[3].append("... ");

		nums[4].append(".  .");
		nums[4].append(".  .");
		nums[4].append("....");
		nums[4].append("   .");
		nums[4].append("   .");

		nums[5].append("....");
		nums[5].append(".   ");
		nums[5].append("... ");
		nums[5].append("   .");
		nums[5].append("... ");

		nums[6].append(" ...");
		nums[6].append(".   ");
		nums[6].append("... ");
		nums[6].append(".  .");
		nums[6].append(" .. ");

		nums[7].append("....");
		nums[7].append("   .");
		nums[7].append("  . ");
		nums[7].append(" .  ");
		nums[7].append(" .  ");

		nums[8].append(" .. ");
		nums[8].append(".  .");
		nums[8].append(" .. ");
		nums[8].append(".  .");
		nums[8].append(" .. ");

		nums[9].append(" .. ");
		nums[9].append(".  .");
		nums[9].append(" ...");
		nums[9].append("   .");
		nums[9].append(" .. ");
	}
	~GameEngine() {
	}
protected:
	virtual bool OnUserCreate() {
		for (x = 0; x < MaxFoodCount; x++) Map[MapWidth * GetRandom(0, MapHeight) + GetRandom(0, MapWidth)] = FOOD;
		for (x = 0; x < SnakeMaxCount; x++)	AddSnake();

		std::thread t(RandomFoodGenerator);
		t.detach();

		return true;
	}
	virtual bool OnUserUpdate(float fElapsedTime) {
		timer += fElapsedTime;
		
		if (timer > TimeOfFrameChange) {
			timer = 0;
			//player control
			Players[0].direction = Idle;

			Players[0].boosted = m_keys[SpaceBar].bHeld;

			if (m_keys[UpKey].bHeld) {
				if (m_keys[RightKey].bHeld) Players[0].direction = UpRight;
				else if (m_keys[LeftKey].bHeld) Players[0].direction = UpLeft;
				else Players[0].direction = Up;
			}
			else if (m_keys[DownKey].bHeld) {
				if (m_keys[RightKey].bHeld) Players[0].direction = DownRight;
				else if (m_keys[LeftKey].bHeld) Players[0].direction = DownLeft;
				else Players[0].direction = Down;
			}
			else if (m_keys[LeftKey].bHeld) Players[0].direction = Left;
			else if (m_keys[RightKey].bHeld) Players[0].direction = Right;


			if (m_keys[ESC].bHeld) exit(1);


			//logical

			Players[0].Move();
			for (x = 1; x < SnakeMaxCount; x++) Players[x].AutoMove();

			//drowing

			offset = { Players[0].path.front().x - WindowWidth / 2, Players[0].path.front().y - WindowHeight / 2 };

			if (offset.x < 0) offset.x = MapWidth + offset.x;
			if (offset.y < 0) offset.y = MapHeight + offset.y;

			for (x = 0; x < WindowWidth; x++)
				for (y = 0; y < WindowHeight; y++)
					Draw(x, y, PIXEL_SOLID, Map[MapWidth * ((y + offset.y) % MapHeight) + (x + offset.x) % MapWidth] % 16);

			DrowNum(Players[0].path.size());

			return true;
		}
	}
};

int main()
{
	GameEngine Game;
	Game.ConstructConsole(WindowWidth, WindowHeight, resolution_X, resolution_Y);
	Game.Start();
}