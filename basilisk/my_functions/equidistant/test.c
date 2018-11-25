/** Testing the equidistant averaging functions.

First we define a function, cubed in space and squared in time. Moving at a speed of 1/tau.
De time integration of this function is define afterwards 
*/ 

#define func(s,d,f) (cube(s-sq(t/tau)) + cube(d) + cube(f))
#define Ifunc(s,d,f) (cube(d) + cube(f) + cube(s) - sq(s)*sq(t)/sq(tau) + 3*s*pow(t, 4)/5./pow(tau,4) - pow(t,6)/(7*pow(tau,6)))

double tau;
double ave_err;

#include "grid/octree.h"
#include "run.h"
#include "equi_data.h"  // The functions that are to be tested


int minlevel = 2; 	 
int maxlevel = 5;
scalar b[];


int main(){    
    FILE * fp3 = fopen("outputfile", "w"); // Specify te output file
    L0=1.; 	
    DT=0.05;				
    int pp = 0;
/** Loop over different DT to see if the value converges */
    for(maxlevel=2; maxlevel<7; maxlevel++){
        init_grid(1<<maxlevel);
        
        tau = 1.2;
	ediag.ii=0;

        run();
	
	if(pid() == 0) {	
	    if(pp==0){
  	        fprintf(fp3, "err\tdt\tL0\n");
  	    }
            fprintf(fp3, "%g\t%g\t%g\n", ave_err, DT, L0); // Write away error
	}
	free(equifield);  // we dont want memory leaks
	equifield = NULL; // reset the field pointer 
	pp++;
    }	
}

/** Ability to test for non uniform grid */ 
event init(i=0){
	//unrefine(y <= L0/4. && level > minlevel-1); 
}

/** Set values of scalar b based on defined function */
event update(i++) {
    dt = dtnext(DT);
    foreach() {
	b[] = func(x, y, z);
    }
    boundary({b});
}

/** Keeping track of the field b (averaging), here we cheat a bit since the error determination is defined in the function itself. */    
event diag(i++){
    fprintf(stderr, "time=%g\n",t);
    equi_diag(b); 
}

/** Write away output of the averaging function */
event end(t=1){
    FILE * fp2 = fopen("output", "w");
    equi_output(b, fp2);
    fclose(fp2);
}

/**

~~~gnuplot
    set xr [0.:0.1]
    set yr [0.:0.1]
    set xlabel 'DT'
    set ylabel 'Error'
    set size square
    plot 'outputfile' using 1:2 title "DT vs err" 
~~~

*/
