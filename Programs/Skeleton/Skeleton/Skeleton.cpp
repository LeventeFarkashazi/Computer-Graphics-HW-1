//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII
// karaktereket tartalmazhat, BOM kihuzando. Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni
// es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest
// kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual
// Studio-hoz kepesti elteresekrol es a leggyakoribb hibakrol (pl. ideiglenes
// objektumot nem lehet referencia tipusnak ertekul adni) a hazibeado portal ad
// egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az
// oran a feladatkiadasig elhangzottak A keretben nem szereplo GLUT fuggvenyek
// tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Farkasházi Levente
// Neptun : HFDKFC
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen
// segitseget igenybe vettem vagy mas szellemi termeket felhasznaltam, akkor a
// forrast es az atvett reszt kommentekben egyertelmuen jeloltem. A
// forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi,
// illetve a grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban,
// irasban, Interneten, stb.) erkezo minden egyeb informaciora (keplet, program,
// algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is
// ertem, azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok
// azzal, hogy az atvett reszek nem szamitanak a sajat kontribucioba, igy a
// feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik
// dontes. Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese
// eseten a hazifeladatra adhato pontokat negativ elojellel szamoljak el es
// ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line
// characters
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

// 2D camera
class Camera2D {
  vec2 wCenter;  // center in world coordinates
  vec2 wSize;    // width and height in world coordinates
 public:
  Camera2D() : wCenter(0, 0), wSize(200, 200) {}

  mat4 V() { return TranslateMatrix(-wCenter); }
  mat4 P() { return ScaleMatrix(vec2(2 / wSize.x, 2 / wSize.y)); }

  mat4 Vinv() { return TranslateMatrix(wCenter); }
  mat4 Pinv() { return ScaleMatrix(vec2(wSize.x / 2, wSize.y / 2)); }

  void Zoom(float s) { wSize = wSize * s; }
  void Pan(vec2 t) { wCenter = wCenter + t; }
};

Camera2D camera;            // 2D camera
GPUProgram gpuProgram;      // vertex and fragment shaders
int transform = 0;          // for testing

vec4 projectToHyperboloid(vec4 v) {
  if (transform)
    return vec4(v.x, v.y, sqrtf(v.x * v.x + v.y * v.y + 1), v.w);
  else return v;
}

vec4 projectToDisc(vec4 v) {
  if (transform)
    return vec4(v.x/(v.z + 1), v.y/(v.z + 1), 0, v.w);
  else return v;
}

class Atom {
  unsigned int vao, vbo;    // vertex buffer object
  const int nVertices = 50; //the "resolution"

 public:
  int mass;         // between 2 and 6
  int charge;       // between -10 and +10 (hidrogene: 1.60217 * 10^-19 C)
  float radius;     // calculated from mass (between 0.02 and 0.06)
  vec4 center;

  std::vector<vec4> points;

  Atom(float x, float y, int c) {
    center.x = x;
    center.y = y;
    charge = c;
    mass = (rand() % 5) + 2;
    radius = (float)mass / 100;
  }

  void calculateVerices() {
    points.clear();
    for (int i = 0; i < nVertices; i++) {
      float phi = (float)i * 2.0f * (float)M_PI / (float)nVertices;     //phi angle from the unit circle in radian 
      points.push_back(projectToDisc(projectToHyperboloid(vec4(
            cosf(phi) * radius + center.x,                              //unit circle points * radius + center offset
            sinf(phi) * radius + center.y, 0, 1))));
    }
  }

  void create() {
    calculateVerices();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);                  // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

     glBufferData(GL_ARRAY_BUFFER,          // copy to the GPU
                 nVertices * sizeof(vec4),  // number of the vbo in bytes
                 &points[0],                // address of the data array on the CPU
                 GL_STATIC_DRAW);           // copy to that part of the memory which is not modified 

    glEnableVertexAttribArray(0);           // AttribArray 0
    glVertexAttribPointer(
    0,                                      // vbo -> AttribArray 0
    2, GL_FLOAT, GL_FALSE,                  // two floats/attrib, not fixed-point
    sizeof(float) * 4, NULL);               // stride, offset: tightly packed
  }

    void draw() {
    
        int location = glGetUniformLocation(gpuProgram.getId(), "color");
        if (charge < 0)
            glUniform3f(location, 0.0f, 0.0f, (float)-1 * charge / 10); // blue between 0 and 1 (need to be positive...)
        else {
            glUniform3f(location, (float)charge / 10, 0.0f, 0.0f);      // red between 0 and 1
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
    }

};

class Bond {
  unsigned int vao, vbo;        // vertex buffer object
  const int nVertices = 50;     //the "resolution"

 public:
  std::vector<vec4> points;
  std::vector<Atom> atoms;      //2 atoms

  Bond(Atom a, Atom b) {
    atoms.push_back(a);
    atoms.push_back(b);
  }

  void calculateVerices() {
    points.clear();
    for (int i = 0; i <= nVertices; ++i) {                                     //linear interpollation between two known points
      float xCoordinate = atoms.at(0).center.x + ((atoms.at(1).center.x - atoms.at(0).center.x) / (float)nVertices) * (float)i;
      float yCoordinate = atoms.at(0).center.y + ((atoms.at(1).center.y - atoms.at(0).center.y) / (float)nVertices) * (float)i;
      points.push_back(vec4(xCoordinate, yCoordinate, 0, 1));
    }
  }

  void create() {
    calculateVerices();
    
    for (int i = 0; i < nVertices; i++) {
      points.at(i) = projectToDisc(projectToHyperboloid(points.at(i)));
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);              // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, nVertices * sizeof(vec4), &points[0], GL_STATIC_DRAW);
  
      glEnableVertexAttribArray(0);     // AttribArray 0
    glVertexAttribPointer(
        0,                              // vbo -> AttribArray 0
        2, GL_FLOAT, GL_FALSE,          // two floats/attrib, not fixed-point
        sizeof(float) * 4, NULL);       // stride, offset: tightly packed
  }

  void draw() {
    int location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location, 1.0f, 1.0f, 1.0f);  // 3 floats

    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, nVertices);
  }
};

class Molecule {
 public:
  int nAtoms;                   // random between2 and 8
  int nBonds;                   // tree => edges == atoms-1
  float atomDistance = 0.3f;    // between centers
  vec2 wTranslate;              // translation
  float phi;                    // rotation

  vec4 massCenter;

  std::vector<Atom> atoms;
  std::vector<Bond> bonds;

  Molecule() {
    nAtoms = rand() % 7 + 2;
    nBonds = nAtoms - 1;
    
    animate(0);

    std::vector<int> charges;

    int randomCharge;
    int chargeSum;
    bool zeroCharge = false;

    while (!zeroCharge) {   //try until charge is 0
      charges.clear();
      chargeSum = 0;

      for (int i = 0; i < nAtoms; i++) {
        randomCharge = rand() % 21 - 10; //random between -10 and 10;
        chargeSum += randomCharge;
        charges.push_back(randomCharge);
      }

      if (chargeSum == 0) {
        zeroCharge = true;
      }
    }

    for (int i = 0; i < nAtoms; i++) {
      Atom newAtom = Atom(0, 0, 0);
      
      if (!atoms.empty()) {
        Atom pair = atoms.at(rand() % atoms.size());

        int badPosition = 1;
        while (badPosition) {
          float randomPhi = ((float)(rand() % 10001) / 10000) * 2.0f * (float)M_PI;                     // random phi between 0 and (2 * (float)M_PI) radian
          newAtom.center = pair.center + vec4(cosf(randomPhi), sinf(randomPhi), 0, 1) * atomDistance;   // random directional unit vector * distance

          if (-1 < newAtom.center.x && newAtom.center.x < 1 &&      // atoms can't be initializes out of frame
              -1 < newAtom.center.y && newAtom.center.y < 1) {  
            badPosition = 0;
          }
        }

        newAtom.charge = charges.at(i);                             //precalculated charge

        bonds.push_back(Bond(newAtom, pair));
      } else {                                                     //first atom ("root" of tree)
        newAtom.center.x = (float)(rand() % 201 - 100) / 100;
        newAtom.center.y = (float)(rand() % 201 - 100) / 100;
        newAtom.charge = charges.at(i);
      }
      atoms.push_back(newAtom);
    }
  }

  void create() {
    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds.at(i).create();
    }

    for (int i = 0; i < nAtoms; i++) {
      atoms.at(i).create();
    }
  }

  void animate(float t) { 
     phi = t;
     wTranslate = vec2(0, 0);
  }

  mat4 M() {
		mat4 Mrotate(cosf(phi), sinf(phi), 0, 0,
			        -sinf(phi), cosf(phi), 0, 0,
			           0,        0,        1, 0,
			           0,        0,        0, 1); // rotation

		mat4 Mtranslate(1,            0,            0, 0,
			            0,            1,            0, 0,
			            0,            0,            0, 0,
			            wTranslate.x, wTranslate.y, 0, 1); // translation

		return Mrotate * Mtranslate;	// model transformation
	}


  void draw() {
          mat4 MVPTransform = M() * camera.V();
    gpuProgram.setUniform(MVPTransform, "MVP");

    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds.at(i).draw();
    }

    for (int i = 0; i < nAtoms; i++) {
      atoms.at(i).draw();
    }
  }
};

Molecule m1;
Molecule m2;

// Initialization, create an OpenGL context
void onInitialization() {
  glViewport(0, 0, windowWidth, windowHeight);      // Position and size of the photograph on screen

  // Initialize the components of the molecule
  m1.create();
  m2.create();

  // create program for the GPU
  gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// Window has become invalid: Redraw
void onDisplay() {

  glClearColor((GLclampf)0.2, (GLclampf)0.2, (GLclampf)0.2, (GLclampf)0);     // background color
  glClear(GL_COLOR_BUFFER_BIT);    // clear frame buffer
    
  m1.draw();
  m2.draw(); 

  glutSwapBuffers();        // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
  switch (key) {
    case 's':
      camera.Pan(vec2(-0.1, 0.0));
      break;
    case 'd':
      camera.Pan(vec2(+0.1, 0.0));
      break;
    case 'e':
      camera.Pan(vec2(0.0, 0.1));
      break;
    case 'x':
      camera.Pan(vec2(0.0, -0.1));
      break;
    case ' ':
      m1 = Molecule();
      m1.create();
      m2 = Molecule();
      m2.create();
      break;
  }
  glutPostRedisplay(); // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {}

// Move mouse with key pressed
void onMouseMotion(
    int pX, int pY) {  // pX, pY are the pixel coordinates of the cursor in the
                       // coordinate system of the operation system
  // Convert to normalized device space
  float cX = 2.0f * pX / windowWidth - 1;  // flip y axis
  float cY = 1.0f - 2.0f * pY / windowHeight;
  printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX,
             int pY) {  // pX, pY are the pixel coordinates of the cursor in the
                        // coordinate system of the operation system
  // Convert to normalized device space
  float cX = 2.0f * pX / windowWidth - 1;  // flip y axis
  float cY = 1.0f - 2.0f * pY / windowHeight;

  char* buttonStat;
  switch (state) {
    case GLUT_DOWN:
      buttonStat = "pressed";
      break;
    case GLUT_UP:
      buttonStat = "released";
      break;
  }

  switch (button) {
    case GLUT_LEFT_BUTTON:
      printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
      break;
    case GLUT_MIDDLE_BUTTON:
      printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
      break;
    case GLUT_RIGHT_BUTTON:
      printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);
      break;
  }
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
  long time = glutGet(GLUT_ELAPSED_TIME);  // elapsed time since the start of the program
  float sec = time / 1000.0f;  // convert msec to sec
  m1.animate(sec);       // animate the triangle object
  m2.animate(-sec/2);
  glutPostRedisplay();         // redraw the scene
}
