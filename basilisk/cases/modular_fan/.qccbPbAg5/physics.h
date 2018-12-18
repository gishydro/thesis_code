#ifndef BASILISK_HEADER_13
#define BASILISK_HEADER_13
#line 1 "./physics.h"
#if dimension == 3
	#include "SGS.h"
#endif 

#define CP 1005.	// C_p for air 
#define gCONST 9.81	// Gravitational constant
#define TREF 273.	// Kelvin
#define INVERSION .2 	// Kelvin per meter

#define STRAT(s) gCONST*(INVERSION + gCONST/CP)*s/TREF // Stratification 
#define WIND(s) def.wind 	// Wind profile
//#define strat(s) gCONST*5.*log(sqrt(s)+1)/TREF

double crho = 1.;
scalar b[];
scalar * tracers = {b};
	
face vector av[]; 
struct sCase def;

struct sCase {
	double wind;
	double wphi;
};

void init_physics(){
 	def.wind = 1.;
        def.wphi = 0.;

	b.nodump = false; // TODO

	if(def.wind == 0.){    	
	    u.n[bottom] = dirichlet(0.);
	    u.t[bottom] = dirichlet(0.);
	    u.n[top] = dirichlet(0.);
 	    u.t[top] = dirichlet(WIND(y));

            periodic (left);

	} else if(fabs(def.wind) > 0.) {
	
            u.n[right] = dirichlet(WIND(y));
	    u.n[left] = dirichlet(WIND(y));
	    
	    u.n[bottom] = dirichlet(0.);
	    u.t[bottom] = neumann(0.);
	    u.n[top] = dirichlet(0.);
 	    u.t[top] = neumann(0.);
	    //if(def.wind > 0.) {
	    //} else if(def.wind<0.) {
	    //}
            b[left] = dirichlet(STRAT(y));
           b[right] = dirichlet(STRAT(y));

        }
	
	
	b[bottom] = dirichlet(0.);
	b[top] = dirichlet(STRAT(y));

	#if dimension == 3
		u.r[bottom] = dirichlet(0.);
		u.r[top] = neumann(0.); //dirichlet(def.vgeo);
		
		//Evis[bottom] = dirichlet(0.);
        	//Evis[top] = dirichlet(0.);

		periodic(front);
	#endif  

	foreach() {
		b[] = STRAT(y);
	}
}

/* Gravity forcing */
event acceleration(i++){
	foreach_face(y){
		av.y[] = (b[] + b[0,-1])/2.;
	}
}

mgstats mgb;

/* Diffusion */
event tracer_diffusion(i++){
	mgb = diffusion(b, dt, mu);
}

#endif