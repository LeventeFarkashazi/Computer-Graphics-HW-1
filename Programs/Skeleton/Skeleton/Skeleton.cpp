//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Farkasházi Levente
// Neptun : HFDKFC
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers
	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0
	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel
	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram; // vertex and fragment shaders
unsigned int vao;	   // virtual world on the GPU
float p = 0.0f, q = 0.0f;

class Atom {
public:
	int mass;		//1 -> 10
	int charge;		// -10 -> +10 (elemi töltése)

	float radius;
	vec4 center;

	const int nVertices = 50;
	std::vector<vec4> points;

	Atom(float x, float y, int c) {
		center.x = x;
		center.y = y;
		charge = c;
		mass = rand() % 6 + 1;
		radius = (float)mass / 100;
	}

	void calculateVerices() {		
		for (int i = 0; i < nVertices; i++) {
			float phi = (float)i * 2.0f * (float)M_PI / (float)nVertices;
			points.push_back(vec4(cosf(phi) * radius + center.x, sinf(phi) * radius + center.y , 0, 1));
		}
	}

	void drawAtom() {
		calculateVerices();
		
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		if(charge < 0)
		glUniform3f(location, 0.0f, 0.0f, (float) -1* charge/10); // 3 floats
		else {
			glUniform3f(location, (float)charge / 10, 0.0f, 0.0f);
		}

		unsigned int vbo;		// vertex buffer object
		glGenBuffers(1, &vbo);	// Generate 1 buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, nVertices * sizeof(vec4), &points[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);				// AttribArray 0
		glVertexAttribPointer(0,					// vbo -> AttribArray 0
			2, GL_FLOAT, GL_FALSE,					// two floats/attrib, not fixed-point
			sizeof(float) * 4, NULL); 				// stride, offset: tightly packed

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
	}

};

class Bond {
public:
	std::vector<vec4> points;

	Bond(Atom a, Atom b) {
		points.push_back(a.center);
		points.push_back(b.center);
	}


	void drawBond() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);	// 3 floats

		unsigned int vbo;							// vertex buffer object
		glGenBuffers(1, &vbo);						// Generate 1 buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec4), &points[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);				// AttribArray 0
		glVertexAttribPointer(0,					// vbo -> AttribArray 0
			2, GL_FLOAT, GL_FALSE,					// two floats/attrib, not fixed-point
			sizeof(float) * 4, NULL); 				// stride, offset: tightly packed

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);

		glDrawArrays(GL_LINES, 0, 2);
	}

};

class Molecule {
public:
	int nAtoms;		//2 -> 8
	int nBonds;		// -10 -> +10 (elemi töltése)
	float atomDistance = 0.1f;

	vec4  massCenter;

	std::vector<Atom> atoms;
	std::vector<Bond> bonds;

	Molecule() {
		nAtoms = rand() % 7 + 2;
		nBonds = nAtoms - 1;

		std::vector<int> charges;

		int randomCharge;
		int chargeSum;
		bool zeroCharge = false;

		while (!zeroCharge) {
			charges.clear();
			chargeSum = 0;

			for (int i = 0; i < nAtoms; i++) {
				randomCharge = rand() % 21 - 10;
				//printf("%d\n", randomCharge);
				chargeSum += randomCharge;
				charges.push_back(randomCharge);
			}

			//printf("------------------------------\n");

			if (chargeSum == 0) {
				zeroCharge = true;
			}
		}

		for (int i = 0; i < nAtoms; i++) {
			atoms.push_back(Atom((float)(rand() % 201 - 100) / 100, (float)(rand() % 201 - 100) / 100, charges.at(i)));
		}
		

		for (int i = 0; i < nBonds; i++) {
			bonds.push_back(Bond(atoms.at(i), atoms.at(i+1)));
		}
	}
		void drawMolecule() {
			for (int i = 0; i < bonds.size(); i++) {
				bonds.at(i).drawBond();
			}


			for (int i = 0; i < nAtoms; i++) {
				atoms.at(i).drawAtom();
			}
		}

};

Molecule m1;
Molecule m2;

// Initialization, create an OpenGL context
void onInitialization() {
	m1 = Molecule();
	m2 = Molecule();

	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);	// get 1 vao id
	glBindVertexArray(vao);		// make it active
	
	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
	
}

// Window has become invalid: Redraw
void onDisplay() {
	m1 = Molecule();
	m2 = Molecule();

	glClearColor(0.2, 0.2, 0.2, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix, 
							  0, 1, 0, 0,    // row-major!
							  0, 0, 1, 0,
							  q, p, 0, 1 };

	int  location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

	glBindVertexArray(vao);  // Draw call
	//glDrawArrays(GL_TRIANGLES, 0 /*startIdx*/, 3 /*# Elements*/);
	
	/*
	Atom a = Atom(0.3f, 0.3f, -10);

	Atom b = Atom(-0.7f, 0.2f, 10);
	
	Bond c = Bond(a,b);
	
	c.drawBond();
	b.drawAtom();
	a.drawAtom();


	Atom d = Atom(0.3f, -0.3f, -3);

	Atom e = Atom(-0.7f, -0.2f, 3);

	Bond f = Bond(d, e);

	f.drawBond();
	e.drawAtom();
	d.drawAtom();
	*/

	m1.drawMolecule();
	m2.drawMolecule();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
	switch (key) {
	case 'e': p += 0.1f; break;
	case 'x': p -= 0.1f; break;
	case 'd': q += 0.1f; break;
	case 's': q -= 0.1f; break;
	}
	glutPostRedisplay();
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
	float cY = 1.0f - 2.0f * pY / windowHeight;

	char* buttonStat;
	switch (state) {
	case GLUT_DOWN: buttonStat = "pressed"; break;
	case GLUT_UP:   buttonStat = "released"; break;
	}

	switch (button) {
	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}

