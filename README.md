# Computer Graphics HW 1

Assignment:

![image](https://user-images.githubusercontent.com/79463263/166258230-39630273-328b-4f3f-ba0a-714f35362db1.png)

Translation:
Create a molecular docking application. SPACE always results in two molecules, with a Coulomb force between their atoms, which moves or rotates the molecules. The atoms of the molecules are subjected to an interfacial drag proportional to their velocity. A molecule is a rigid structure of atoms with a random graph topology. The number of atoms is a random number between 2 and 8. The mass of the constituent atoms is a random positive integer multiple of the mass of the hydrogen atom and the charge of the electron. The total charge for each molecule is zero. The molecules move in 2D Euclidean space, the atoms here are circular, the edges of the graph within the atom are white and are sections in Euclidean geometry. Positively charged atoms are red and negatively charged atoms are blue, with intensity proportional to charge. Our microscope maps the Euclidean plane onto the hyperbolic plane, preserving the x, y coordinates, and then displays it using the Beltrami-Poincaré mapping on a circle of maximum radius that can be drawn on a 600x600 resolution screen. The s,d,x,e keys are used to shift the Euclidean virtual world left, right, down and up by 0.1 units. The time step can be 0.01 sec regardless of the drawing speed.

Result:

<p align="center">
  <img src="https://user-images.githubusercontent.com/79463263/166259410-72c8774a-815c-4b63-acb2-2fa18ed242b4.gif" />
</p>
