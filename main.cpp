#include "FundLibs/BarakWinHelper/Barak.h"
#include "FundLibs/BarakWinHelper/Win/Keys.h"
#include "FundLibs/ObjectsGL/Shader.h"
#include "Simplex.h"
#include <chrono>
#include <vector>

//glad 4.6

float ff(int g) { return (float)g; }

void prepr(hWindow* g_Win){
	g_Win->rename(L"Graphical panel");
}
#define errvox 0xFFFFFFFD

GLuint* voxels;
size_t XX = 256, YY = 64, ZZ = 256;

GLuint getVox(uint32_t x, uint32_t y, uint32_t z) {
	if (x >= XX || y >= YY || z >= ZZ) return errvox;
	return voxels[(x * YY + y) * ZZ + z];
}

void setVox(uint32_t x, uint32_t y, uint32_t z, uint32_t id) {
	if (x >= XX || y >= YY || z >= ZZ) return;
	voxels[(x * YY + y) * ZZ + z] = id;
}

#define water_vox 3
#define mud_vox 2

typedef struct pppttt {
	uint32_t voxs[10];
	uint32_t x, y, z;
	bool is_live = true;
	uint8_t staticcc = 0;
	pppttt() {
		voxs[0] = 0;
		x = rand() % XX;
		y = YY - 1;
		z = rand() % ZZ;
	}
	void update() {
		bool down_vox = getVox(x, y - 1, z);
		if (!down_vox) {
			y--;
			staticcc = 0;
		} else {
			staticcc++;
			uint8_t u = !bool(getVox(x + 1, y, z));
			uint8_t d = !bool(getVox(x - 1, y, z));
			uint8_t r = !bool(getVox(x, y, z + 1));
			uint8_t l = !bool(getVox(x, y, z - 1));

			std::vector<uint8_t> vel;
			if (u) vel.push_back(u);
			if (d) vel.push_back(d * 2);
			if (r) vel.push_back(r * 4);
			if (l) vel.push_back(l * 8);

			uint8_t fff = vel.size();
			if (!fff || staticcc > 64 || y == 0) {
				is_live = false;
				if (down_vox == mud_vox || voxs[0] != 0)
					setVox(x, y, z, water_vox);
				else 
					setVox(x, y, z, mud_vox);

				return;
			}
			if (!(getVox(x + 1, y-1, z) && getVox(x - 1, y - 1, z) && getVox(x, y - 1, z+1) && getVox(x, y - 1, z-1)) && down_vox != mud_vox && down_vox != water_vox && down_vox != errvox && !(rand() % 32)) (voxs[0] = down_vox), setVox(x, y-1, z, 0);

			fff = rand() % fff;
			fff = vel[fff];
			vel.clear();

			switch (fff) {
			case 1:
				x++;
				break;
			case 2:
				x--;
				break;
			case 4:
				z++;
				break;
			case 8:
				z--;
				break;
			default:
				break;
			}
		}
		setVox(x, y, z, water_vox);
	}
}pppttt;

bool dfgh = true;
void upd_th() {
	std::vector<pppttt> points;
	for (uint32_t i(1000); i--;)
		points.emplace_back();
	while (dfgh) {
		
		for (uint32_t i(points.size()); i--;) {
			setVox(points[i].x, points[i].y, points[i].z, 0);
			points[i].update();
			if (!points[i].is_live) points[i] = pppttt();
		}

		//Sleep(10);
	}
}

int main() {
	srand(time(0));

	program prog;
	prog.setShaderFVG("Shaders/main.vert.glsl", 0);
	prog.setShaderFVG("Shaders/main.frag.glsl", 1);
	prog.create();
	prog.use();

	voxels = new GLuint[XX * YY * ZZ];

	SimplexNoise nois;

	//loads data to voxel array
	for (size_t i(XX); i--;)
		for (size_t j(YY); j--;)
			for (size_t k(ZZ); k--;)
				voxels[(i * YY + j) * ZZ + k] = (j < 30 + 30*nois.noise(i/32., j/32., k/32.))?1 : 0;// rand() % 4;

	std::thread u_th(upd_th);

	//set size on GPU
	glUniform1ui(1, XX);
	glUniform1ui(2, YY);
	glUniform1ui(3, ZZ);

	//creates and load voxel array to GPU
	GLuint voxels_buffer;
	glGenBuffers(1, &voxels_buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxels_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint)*XX*YY*ZZ, voxels, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxels_buffer);

	float speed = 30;
	float angSpeed = 1;
	char vel[3];
	float pos[3] = { 0, 50, 0 };
	char angVel[2] = { 0, 0 };
	float ang[2] = { 0, 0 };
	float dt;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (threadIsLive()) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * XX * YY * ZZ, voxels, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxels_buffer);

		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> dTime = tp2 - tp1;
		tp1 = tp2;
		float dt = dTime.count();
		
		KeyUpdate();
		if (GetKey(VK_ESCAPE).bHeld)
			break;
		angVel[0] = 0; angVel[1] = 0;
		if (GetKey(VK_LEFT).bHeld)
			angVel[0] -= 1;
		if (GetKey(VK_RIGHT).bHeld)
			angVel[0] += 1;
		ang[0] += ((float)angVel[0]) * angSpeed * dt;
		if (GetKey(VK_UP).bHeld && ang[1] < 1.57079632f)
			angVel[1] += 1;
		if (GetKey(VK_DOWN).bHeld && ang[1] > -1.57079632f)
			angVel[1] -= 1;
		ang[1] += ((float)angVel[1]) * angSpeed * dt;

		if (ang[0] > 3.14159265f)
			ang[0] -= 6.2832853f;
		if (ang[0] < -3.14159265f)
			ang[0] += 6.2832853f;
		vel[0] = 0;
		vel[1] = 0;
		vel[2] = 0;
		if (GetKey('W').bHeld)
			vel[2] += 1;
		if (GetKey('A').bHeld)
			vel[0] -= 1;
		if (GetKey('S').bHeld)
			vel[2] -= 1;
		if (GetKey('D').bHeld)
			vel[0] += 1;
		if (GetKey(VK_SHIFT).bHeld)
			vel[1] -= 1;
		if (GetKey(VK_SPACE).bHeld)
			vel[1] += 1;
		pos[0] += (((float)vel[0])*cos(ang[0]) + ((float)vel[2])*sin(ang[0]))*speed*dt;
		pos[1] += ((float)vel[1])*speed*dt;
		pos[2] += (((float)vel[2])*cos(ang[0]) - ((float)vel[0])*sin(ang[0]))*speed*dt;
		//sets position and angles of camera on GPU
		glUniform3f(4, pos[0], pos[1], pos[2]);
		glUniform2f(5, ang[0], ang[1]);

		//clears color buffer
		glClear(GL_COLOR_BUFFER_BIT);

		//sets aspect area of screen
		glUniform1f(0, ff(getWind().getW())/ff(getWind().getH()));
		//draws canvas
		glDrawArrays(GL_TRIANGLES, 0, 3);

		SwapBuffers();
	}
	dfgh = false;
	u_th.join();
	return 1;
}