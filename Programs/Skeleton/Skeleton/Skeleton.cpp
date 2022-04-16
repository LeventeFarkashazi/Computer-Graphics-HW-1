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
int transform = 1;          // for testing

float coulombConst = 8.988e9f;
float chargeUnit = 1.602e-18f;
float hydrogenMass = 1.674e-17f;      //invalid

vec3 projectToHyperboloid(vec3 v) {
  if (transform)
    return vec3(v.x, v.y, sqrtf(v.x * v.x + v.y * v.y + 1));
  else
    return v;
}

vec3 projectToDisc(vec3 v) {
  if (transform)
    return vec3(v.x / (v.z + 1), v.y / (v.z + 1), 0);
  else
    return v;
}

class Atom {
  unsigned int vao = 0;
  unsigned int vbo = 0;    // vertex buffer object
  const int nVertices = 50; //the "resolution"

 public:
  int mass;         // between 2 and 6
  int charge;       // between -10 and +10 (hidrogene: 1.60217 * 10^-19 C)
  float radius;     // calculated from mass (between 0.02 and 0.06)
  vec3 center;

  std::vector<vec3> points;

  Atom(float x, float y, int c) {
    center.x = x;
    center.y = y;
    charge = c;
    mass = (rand() % 6) + 5;
    radius = (float)mass / 80;
  }

  void calculatePoints() {
    points.clear();
    for (int i = 0; i < nVertices; i++) {
      float phi = (float)i * 2.0f * (float)M_PI / (float)nVertices;     //phi angle from the unit circle in radian 
      points.push_back(vec3(
            cosf(phi) * radius + center.x,                              //unit circle points * radius + center offset
            sinf(phi) * radius + center.y, 0));
    }
  }

  void rotate(vec3 centerOfRotation, float angle) {     //counter clockwise
    float x = cosf(angle) * (center.x - centerOfRotation.x) - sinf(angle) * (center.y - centerOfRotation.y) + centerOfRotation.x;    //formula: https://en.wikipedia.org/wiki/Rotation_of_axes
    float y = sinf(angle) * (center.x - centerOfRotation.x) + cosf(angle) * (center.y - centerOfRotation.y) + centerOfRotation.y;    //modified with translating the point before totation  
    center.x = x;
    center.y = y;
  }

  void translate(vec3 translationVec) { 
      center = center + translationVec;
  }

  void create() {

    glGenVertexArrays(1, &vao);             // Generate 1 array object
    glBindVertexArray(vao);                 // bind array

    glGenBuffers(1, &vbo);                  // Generate 1 buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo);     // bind buffer

    glEnableVertexAttribArray(0);           // AttribArray 0
    glVertexAttribPointer(
    0,                                      // vbo -> AttribArray 0
    2, GL_FLOAT, GL_FALSE,                  // two floats/attrib, not fixed-point
    sizeof(float) * 3, NULL);               // stride, offset: tightly packed
  }

    void draw() {
        //calculate points
        calculatePoints();
        
        //projections all points to hiperboloid and back to disc
        for (unsigned int i = 0; i < points.size(); i++) {
          points[i] = projectToDisc(projectToHyperboloid(points[i]));
        }
        
        //set color (different intensity blue or red)
        int location = glGetUniformLocation(gpuProgram.getId(), "color");   
        if (charge < 0)
            glUniform3f(location, 0.0f, 0.0f, (float)-1 * charge / 10);     // blue between 0 and 1 (need to be positive...)
        else {
            glUniform3f(location, (float)charge / 10, 0.0f, 0.0f);          // red between 0 and 1
        }

        glBindVertexArray(vao);                 //bind vao
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,           // copy to the GPU
                     nVertices * sizeof(vec3),  // number of the vbo in bytes
                     &points[0],                // address of the data array on the CPU
                     GL_STATIC_DRAW);

        glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);
    }
};

class Bond {
  unsigned int vao = 0;
  unsigned int vbo = 0;       // vertex buffer object
  const int nVertices = 50;  // the "resolution"

 public:
  std::vector<vec3> points;
  std::vector<Atom> atoms;

  Bond(Atom a, Atom b) {
    atoms.push_back(a);
    atoms.push_back(b);
  }

  void rotate(vec3 centerOfRotation, float angle) {
    float x0 = cosf(angle) * (atoms[0].center.x - centerOfRotation.x) - sinf(angle) * (atoms[0].center.y - centerOfRotation.y) + centerOfRotation.x;
    float y0 = sinf(angle) * (atoms[0].center.x - centerOfRotation.x) + cosf(angle) * (atoms[0].center.y - centerOfRotation.y) + centerOfRotation.y;
    atoms[0].center.x = x0;
    atoms[0].center.y = y0;

    float x1 = cosf(angle) * (atoms[1].center.x - centerOfRotation.x) - sinf(angle) * (atoms[1].center.y - centerOfRotation.y) + centerOfRotation.x;
    float y1 = sinf(angle) * (atoms[1].center.x - centerOfRotation.x) + cosf(angle) * (atoms[1].center.y - centerOfRotation.y) + centerOfRotation.y;
    atoms[1].center.x = x1;
    atoms[1].center.y = y1;
  }

  void translate(vec3 translationVec) { 
    atoms[0].center = atoms[0].center + translationVec;
    atoms[1].center = atoms[1].center + translationVec;
  }

  void calculatePoints() {
    points.clear();
    for (int i = 0; i <= nVertices; ++i) {  // linear interpollation between two known points 
      float xCoordinate = atoms[0].center.x + ((atoms[1].center.x - atoms[0].center.x) / (float)nVertices) * (float)i; 
      float yCoordinate = atoms[0].center.y + ((atoms[1].center.y - atoms[0].center.y) / (float)nVertices) * (float)i;
      points.push_back(vec3(xCoordinate, yCoordinate, 0));
    }
  }

  void create() {
    calculatePoints();

    glGenVertexArrays(1, &vao);         // Generate 1 array object
    glBindVertexArray(vao);             // bind array

    glGenBuffers(1, &vbo);              // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);  // AttribArray 0
    glVertexAttribPointer(
        0,                         // vbo -> AttribArray 0
        2, GL_FLOAT, GL_FALSE,     // two floats/attrib, not fixed-point
        sizeof(float) * 3, NULL);  // stride, offset: tightly packed
  }

  void draw() {
    // calculate points
    calculatePoints();

    // projections all points to hiperboloid and back to disc
    for (unsigned int i = 0; i < points.size(); i++) {
      points[i] = projectToDisc(projectToHyperboloid(points[i]));
    }

    // set color
    int location = glGetUniformLocation(gpuProgram.getId(), "color");
    glUniform3f(location, 1.0f, 1.0f, 1.0f);// white

    glBindVertexArray(vao);                 // bind vao

    glBindBuffer(GL_ARRAY_BUFFER, vbo);     // bind vbo 
    glBufferData(GL_ARRAY_BUFFER,           // copy to the GPU
                 nVertices * sizeof(vec3),  // number of the vbo in bytes
                 &points[0],                // address of the data array on the CPU
                 GL_STATIC_DRAW);

    glDrawArrays(GL_LINE_STRIP, 0, nVertices);
  }
};

class Molecule {
 public:
  int nAtoms;                 // random between2 and 8
  int nBonds;                 // tree => edges == atoms-1
  float atomDistance = 0.5f;  // between centers
  vec3 wTranslate;            // translation world coordinates  [translation vector/0.01 sec]
  float wRotation;            // rotation world coordinates     [angle/0.01 sec]
 
  vec3 centerOfMass;          // calculated mass center
  float mass;                 // mass in kg
  float momentOfInertia;

  std::vector<vec3> coulombForces;

  std::vector<Atom> atoms;
  std::vector<Bond> bonds;

  Molecule() {
    nAtoms = rand() % 7 + 2;  // random between2 and 8
    nBonds = nAtoms - 1;      // tree => edges == atoms-1

    wTranslate = vec3(0,0,0);
    wRotation = 0;

    std::vector<int> charges;

    int randomCharge;
    int chargeSum;
    bool zeroCharge = false;

    // generate randomized charges, try until sum of charge is 0
    while (!zeroCharge) {
      charges.clear();
      chargeSum = 0;

      for (int i = 0; i < nAtoms; i++) {
        randomCharge = rand() % 21 - 10;  // random between -10 and 10;
        chargeSum += randomCharge;
        charges.push_back(randomCharge);
      }

      if (chargeSum == 0) {
        zeroCharge = true;
      }
    }

    // generate atoms
    for (int i = 0; i < nAtoms; i++) {
      Atom newAtom = Atom(0, 0, 0);

      if (!atoms.empty()) {  // every atom after first
        Atom pair = atoms[
            rand() %
            atoms.size()];  // connect with a bond to another atom randomly
                            // (grow the tree with 1 node) => no circles

          
          float randomPhi =
              ((float)(rand() % 10001) / 10000) * 2.0f *
              (float)M_PI;  // random phi between 0 and (2 * (float)M_PI) radian
          newAtom.center =
              pair.center +
              vec3(cosf(randomPhi), sinf(randomPhi), 0) *
                 atomDistance;  // random directional unit vector * distance

        newAtom.charge = charges[i];  // precalculated charge

        bonds.push_back(Bond(newAtom, pair));
      } else {  // first atom ("root" of tree)
        newAtom.center.x = (float)(rand() % 201 - 100) / 100;
        newAtom.center.y = (float)(rand() % 201 - 100) / 100;
        newAtom.charge = charges[i];
      }
      atoms.push_back(newAtom);
    }

    calcCenterOfMass();
    calcMomentOfInertia();
  }

  void calcCenterOfMass() {
    // formula: http://www.batesville.k12.in.us/physics/apphynet/dynamics/center%20of%20mass/2d_1.html

    float sumMass_x_Xcord = 0.0f;
    float sumMass_x_Ycord = 0.0f;
    float sumMass = 0.0f;

    for (int i = 0; i < nAtoms; i++) {
      sumMass_x_Xcord += atoms[i].mass * atoms[i].center.x;
      sumMass_x_Ycord += atoms[i].mass * atoms[i].center.y;
      sumMass += atoms[i].mass;
    }

    centerOfMass = vec3(sumMass_x_Xcord / sumMass, sumMass_x_Ycord / sumMass, 0);
  
    mass = sumMass * hydrogenMass;
  }

  void calcMomentOfInertia() {
      float sumMomentOfInertia = 0.0f;
      for (int i = 0; i < nAtoms; i++) {
          float distanceX = atoms[i].center.x - centerOfMass.x;
          float distanceY = atoms[i].center.y - centerOfMass.y;
          sumMomentOfInertia = sumMomentOfInertia + atoms[i].mass * ((float)pow(distanceX, 2) + (float)pow(distanceY, 2));
      }
      momentOfInertia = sumMomentOfInertia * hydrogenMass;
  }

  void calcCoulombForces(Molecule othermolecule) {
    coulombForces.clear();
    for (int i = 0; i < nAtoms; i++) {
      vec3 sumCoulombForces = vec3(0,0,0);
      
      for (int j = 0; j < othermolecule.nAtoms; j++) {
        vec3 forceDirection = atoms[i].center - othermolecule.atoms[j].center;
        float distanceMeter = length(forceDirection) * 1e-3f;                                //0.1 nm to meter 10^-8 INVALID magic const needed
        float productOfCharges = (float)othermolecule.atoms[j].charge * chargeUnit * (float)atoms[i].charge * chargeUnit;
        vec3 force = forceDirection * -productOfCharges * coulombConst / (float)pow(distanceMeter, 3);
        sumCoulombForces = sumCoulombForces + force;
      }

      coulombForces.push_back(sumCoulombForces);
    }

    vec3 translationForce = vec3(0, 0, 0);
    for (int i = 0; i < nAtoms; i++) {
      translationForce = translationForce + dot(coulombForces[1], atoms[1].center - centerOfMass) * normalize(atoms[1].center - centerOfMass);
    }
    
   vec3 translationSpeed = translationForce  / mass;    
   wTranslate = wTranslate + (translationSpeed * 0.01f);


   vec3 turningMoment = vec3(0, 0, 0);
   for (int i = 0; i < nAtoms; i++) {
     vec3 atomsTurningMoment = cross(atoms[i].center - centerOfMass, coulombForces[i]);
     turningMoment = turningMoment + atomsTurningMoment;
   }
   float turningSpeed = turningMoment.z / momentOfInertia;

   wRotation = wRotation + (turningSpeed * 0.01f);
  }

  // initialize all components
  void create() {
    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds[i].create();
    }

    for (int i = 0; i < nAtoms; i++) {
      atoms[i].create();
    }
  }

  // rotate all atoms and bonds around center of mass clockwise
  void rotate() {
    calcCenterOfMass();

    for (int i = 0; i < nAtoms; i++) {
      atoms[i].rotate(centerOfMass, wRotation * 0.01f);           //0.01 time step
    }

    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds[i].rotate(centerOfMass, wRotation * 0.01f);           //0.01 time step
    }
  }

  // translate all atoms and bonds with a given vector
  void translate() {
    for (int i = 0; i < nAtoms; i++) {
      atoms[i].translate(wTranslate * 0.01f);                    //0.01 time step
    }

    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds[i].translate(wTranslate * 0.01f);                    //0.01 time step
    }
  }

  // draw all components
  void draw() {
    for (unsigned int i = 0; i < bonds.size(); i++) {
      bonds[i].draw();
    }

    for (int i = 0; i < nAtoms; i++) {
      atoms[i].draw();
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
  
 mat4 MVPTransform = camera.V();
 gpuProgram.setUniform(MVPTransform, "MVP");

  glutSwapBuffers();        // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
  switch (key) {
    case 'd':
      camera.Pan(vec2(-0.1f, 0.0f));
      break;
    case 's':
      camera.Pan(vec2(+0.1f, 0.0f));
      break;
    case 'x':
      camera.Pan(vec2(0.0f, 0.1f));
      break;
    case 'e':
      camera.Pan(vec2(0.0f, -0.1f));
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
void onMouseMotion(int pX, int pY) {}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
  long time = glutGet(
      GLUT_ELAPSED_TIME);      // elapsed time since the start of the program
  if (time % 10 == 0) {         // 0.01s time step
    m1.calcCoulombForces(m2);
    m2.calcCoulombForces(m1);
    m1.rotate();
    m2.rotate();
    m1.translate();
    m2.translate();
    glutPostRedisplay();  // redraw the scene
  }
}
