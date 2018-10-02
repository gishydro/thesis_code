//#include "grid/octree.h" // For 3D
#include "navier-stokes/centered.h"
#include "utils.h" 
#include "fractions.h"

/*
============================================================================
Global variables 
============================================================================
*/

int minlevel, maxlevel;
double eps;
struct sRotor rot; 
scalar fan[];

/*
============================================================================
Declarations
============================================================================
*/

/* Functions */
void rotor_init(); 
void rotor_update(double theta, double phi);
void rotor_coord();
void rotor_forcing();

/* Structures */
struct sRotor {	
	double rampT;			// Time to start up rotor
	double P, Prho;			// Power, powerdensity 
	double R, W, A, V;		// Diameter, Thickness, Area ,Volume
	double x0, y0, z0;		// Origin of rotor
	double theta, phi;		// Polar and Azimuthal angle 
	coord nf, nr;			// Normal vector fan, rotation 
};

/*
============================================================================
Main Code, Events
============================================================================
*/

/* Initialisation */
int main() {

    	// Grid variables 
    	init_grid(2<<7);
   	double L0 = 5.;
   	X0 = Y0 = Z0 = 0.;

   	// Initialize physics 
   	rotor_init(); 
	
  	// Adaptivity
  	minlevel = 3; 
  	maxlevel = 9;
  	eps = 0.005;

  	foreach_dimension() {
   		periodic (right);
    	}

    	run();
}

/* Forcing by the rotor */
event forcing(i = 1; i++) {
	rotor_coord();
	rotor_forcing();

}

/* Rotate the rotor */
event rotate(t = rot.rampT; t+=0.005 ) {
	rotor_update(rot.theta + 0., rot.phi + M_PI/12000);
}


/* Progress event */
event end(t += 2; t <= 41) {
	printf("i = %d t = %g\n", i, t);
}

/* Adaptivity function called */
event adapt(i++) {
	adapt_wavelet((scalar *){u},(double []){eps,eps},maxlevel,minlevel);
}

/* Visualisation */
event movies(t += 0.1) {	

    	vertex scalar omega[]; 	// Vorticity
	scalar lev[];	 	// Grid depth
	scalar ekin[];		// Kin energy 

	foreach () {
		omega[] = ((u.y[1,0] - u.y[-1,0]) - (u.x[0,1] - u.x[0,-1]))/(2*Delta); // Curl(u) 
		lev[] = level;
		ekin[] = Delta*(sq(u.x[]) + sq(u.y[]) + sq(u.z[]));
	}

	boundary ({lev, omega, ekin});

	output_ppm (ekin, file = "ppm2mp4 ekin.mp4", n = 512, linear = true, min = -0.0001, max = 0.0001);
	output_ppm (u.x, file = "ppm2mp4 vel_x.mp4", n = 512, linear = true, min = -1, max = 1);
	output_ppm (u.y, file = "ppm2mp4 vel_y.mp4", n = 512, linear = true, min = -1, max = 1);
	output_ppm (omega, file = "ppm2mp4 vort.mp4", n = 512, linear = true); 
	output_ppm (lev, file = "pp2mp4 grid_depth.mp4", n = 512, min = minlevel, max = maxlevel);
}

/*
============================================================================
Functions
============================================================================
*/

/* Function returning the sRotor structure */
void rotor_init() {
    
	// Set variables 
    	rot.rampT = 1.;
	rot.R = 0.01;     
	rot.W = 0.001;                      
    	rot.Prho = .5;
    
   	rot.x0 = rot.y0 = rot.z0 = L0/2.;

	rot.x0 = L0/2.;
	rot.y0 = L0/10.;
	rot.z0 = 0.;
	
	rotor_update(M_PI/2., 0);
}

/* Updating relevant rotor vars */
void rotor_update(double theta, double phi) {

	rot.theta = theta; 	// Polar angle
	rot.phi = phi;		// Azimuthal angle 

   	// Set normal vectors 
   	rot.nf.x = sin(rot.theta)*cos(rot.phi);
	rot.nf.y = sin(rot.theta)*sin(rot.phi);
	rot.nf.z = cos(rot.theta);

	rot.nr.x = sin(rot.theta)*cos(rot.phi);
    	rot.nr.y = sin(rot.theta)*sin(rot.phi);
    	rot.nr.z = cos(rot.theta);

    	// Calculate consequences
    	rot.A = sq(rot.R)*M_PI;                      
	rot.V = rot.A*rot.W;
	rot.P = rot.V*rot.Prho;
}


/* Function returning the volume fractions of a fan object */
void rotor_coord() {

      	scalar sph[], plnu[], plnd[];

   	fan.prolongation = fraction_refine; // Tell basilisk it is a volume field
    	fraction(sph, -sq((x - rot.x0)) - sq((y - rot.y0)) - sq((z - rot.z0)) + sq(rot.R));
    	fraction(plnu,  rot.nr.x*(x - rot.x0) + rot.nr.y*(y - rot.y0) + rot.nr.z*(z - rot.z0) + rot.W/2.);
    	fraction(plnd, -rot.nr.x*(x - rot.x0) - rot.nr.y*(y - rot.y0) - rot.nr.z*(z - rot.z0) + rot.W/2.);	

	foreach () {
    		fan[] = sph[] * plnu[] * plnd[];
   	}

}

 void rotor_forcing(){
	foreach() {
		if(fan[] > 0.) {
		foreach_dimension() {
			// Work in respective direction 
			double wsgn = sign(rot.nf.x*u.x[]) + (sign(rot.nf.x*u.x[]) == 0)*sign(rot.nf.x);
			double damp = rot.rampT > t ? t/rot.rampT : 1.;
			double w = wsgn*fan[]*damp*sq(rot.nf.x)*(2./rho[])*(rot.P/rot.V)*dt;
			// New kinetic energy
			double utemp = sq(u.x[]) + w;

			double usgn = 1.*(u.x[] >= 0)*(utemp > 0) +
			    	     -1.*(u.x[] >= 0)*(utemp < 0) +
		 		      1.*(u.x[] <  0)*(utemp < 0) +
				     -1.*(u.x[] <  0)*(utemp > 0); 

			u.x[] = usgn*sqrt(fabs(utemp));
		}	
		}
	}
}

