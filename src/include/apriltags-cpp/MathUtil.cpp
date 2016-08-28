/*********************************************************************
 * This file is distributed as part of the C++ port of the APRIL tags
 * library. The code is licensed under GPLv2.
 *
 * Original author: Edwin Olson <ebolson@umich.edu>
 * C++ port and modifications: Matt Zucker <mzucker1@swarthmore.edu>
 ********************************************************************/

#include "MathUtil.h"
#include <cmath>

const at::real MathUtil::epsilon = AT_EPSILON;
const at::real MathUtil::twopi_inv = 0.5/M_PI;
const at::real MathUtil::twopi = 2.0*M_PI;

at::real MathUtil::fabs(at::real f) {
  return std::fabs(f);
}

at::real MathUtil::mod2pi_pos(at::real vin) {

  at::real q = vin * twopi_inv + at::real(0.5);
  int qi = (int) q;
    
  return vin - qi*twopi;

}

at::real MathUtil::mod2pi(at::real vin) {

  at::real v;
    
  if (vin < 0) {
    v = -mod2pi_pos(-vin);
  } else {
    v = mod2pi_pos(vin);
  }

  // Validation test:
  //	if (v < -Math.PI || v > Math.PI)
  //		System.out.printf("%10.3f -> %10.3f\n", vin, v);
    
  return v;
}

at::real MathUtil::mod2pi(at::real ref, at::real v) {
  return ref + mod2pi(v-ref);
}

//     /** Returns a value of v wrapped such that ref and v differ by no
//      * more +/-PI
//      **/
//     static public at::real mod2pi(at::real ref, at::real v)
//     {
//         return ref + mod2pi(v-ref);
//     }

//     /** Returns true if the two at::reals are within a small epsilon of
//      * each other.
//      **/
//     static public boolean at::realEquals(at::real a, at::real b)
//     {
//         return Math.abs(a-b)<epsilon;
//     }

//     static public int clamp(int v, int min, int max)
//     {
//         if (v<min)
//             v=min;
//         if (v>max)
//             v=max;
//         return v;
//     }

//     static public at::real clamp(at::real v, at::real min, at::real max)
//     {
//         if (v<min)
//             v=min;
//         if (v>max)
//             v=max;
//         return v;
//     }

//     public static final at::real square(at::real x)
//     {
//         return x*x;
//     }

//     public static final int sign(at::real v)
//     {
//         if (v >= 0)
//             return 1;
//         return -1;
//     }

//     /** Quickly compute e^x for all x.

//         Accuracy for x>0:
//         x<0.5, absolute error < .0099
//         x>0.5, relative error 0.36%

//         For x<0, we internally compute the reciprocal form; error is
//         magnified.

//         This approximation is also monotonic.

//     **/
//     public static final at::real exp(at::real xin)
//     {
//         if (xin>=0)
//             return exp_pos(xin);

//         return 1/(exp_pos(-xin));
//     }

//     /** Quickly compute e^x for positive x.
//      **/
//     protected static final at::real exp_pos(at::real xin)
//     {
//         // our algorithm: compute 2^(x/log(2)) by breaking exponent
//         // into integer and fractional parts.  The integer part can be
//         // done with a bit shift operation. The fractional part, which
//         // has bounded magnitude, can be computed with a polynomial
//         // approximation. We then multiply together the two parts.

//         // prevent deep recursion that would just return INF anyway...
//         // e^709 > At::Real.MAX_VALUE;
//         if (xin>709)
//             return At::Real.MAX_VALUE;

//         if (xin>43) // recursively handle values which would otherwise blow up.
// 	    {
//             // the value 43 was determined emperically
//             return 4727839468229346561.4744575*exp_pos(xin-43);
// 	    }

//         at::real x = 1.44269504088896*xin; // now we compute 2^x
//         int wx = (int) x; // integer part
//         at::real rx = x-wx;    // fractional part

//         rx*=0.69314718055995; // scale fractional part by log(2)

//         at::real b = 1L<<wx; // 2^integer part
//         at::real rx2 = rx*rx;
//         at::real rx3 = rx2*rx;
//         at::real rx4 = rx3*rx;

//         at::real r = 1+rx+rx2/2+rx3/6+rx4/24; // polynomial approximation for bounded rx.

//         return b*r;
//     }

// returns [-PI,PI]
// accurate within ~0.25 degrees
at::real MathUtil::atan2(at::real y, at::real x) {

  at::real atn = atan(y/x);
  
  if (y>=0) {
    if (x>=0) {
      return atn;
    }
    return M_PI+atn;
  }
  if (x>=0) {
    return atn;
  }
  return -at::real(M_PI)+atn;

}

/** returns [-PI/2, PI/2]
    accurate within 0.014 degrees
**/
at::real MathUtil::atan(at::real x) {

  if (fabs(x) <= 1) {
    return atan_mag1(x);
  } 
  if (x < 0) {
    return -M_PI/2-atan_mag1(1/x);
  } else {
    return M_PI/2-atan_mag1(1/x);
  }

}

// returns reasonable answers for |x|<=1.
at::real MathUtil::atan_mag1(at::real x) {

  // accuracy = 0.26814 degrees
  //	return x/(1+0.28087207802773*x*x);
  
  if (true) {
    
    static const at::real p0 = -0.000158023363661;
    static const at::real p1 = 1.003839939589617;
    static const at::real p2 = -0.016224975245612;
    static const at::real p3 = -0.343317496147292;
    static const at::real p4 = 0.141501628812858;
    
    at::real a = fabs(x);
    at::real a2 = a*a;
    
    at::real y = p0 + p1*a + p2*a2 + p3*(a2*a) + p4*(a2*a2);
    
    if (x < 0) {
      return -y;
    }
    return y;

  } else {
    at::real xx = x*x;
    
    // accuracy = 0.10550 degrees (according to matlab)
    return (at::real(0.00182789418543) + 
            at::real(0.97687229491851)*x + 
            at::real(0.00087659977713)*xx)/
      (at::real(0.99499024627366) + 
       at::real(0.00228262896304)*x + 
       at::real(0.25288677429562)*xx);
  }
}


//     strictfp public static void main(String args[])
//     {
//         Random r = new Random();

//         at::real err = 0;
//         at::real M = 100;

//         System.out.println("Max_at::real: "+At::Real.MAX_VALUE);

//         System.out.println("Checking atan");

//         for (int i=0;i<10000000;i++)
// 	    {
//             at::real x = M*r.nextAt::Real()-M/2;
//             at::real y = M*r.nextAt::Real()-M/2;

//             if (r.nextInt(100)==0)
//                 x=0;
//             else if (r.nextInt(100)==0)
//                 y=0;

//             at::real v1 = Math.atan2(y,x);
//             at::real v2 = atan2(y,x);

//             //		System.out.println(x+" "+y);
//             at::real thiserr = Math.abs(v1-v2);
//             if (thiserr>.1)
//                 System.out.println(x+"\t"+y+"\t"+v1+"\t"+v2);
//             if (thiserr>err)
//                 err=thiserr;
// 	    }
//         System.out.println("err: "+err);
//         System.out.println("err deg: "+Math.toDegrees(err));

//         err = 0;
//         M=500;
//         System.out.println("Checking exp");
//         for (int i=0;i<10000000;i++)
// 	    {
//             at::real x = r.nextAt::Real()*M-M/2;
//             at::real v1 = Math.exp(x);
//             at::real v2 = exp(x);

//             at::real abserr = Math.abs(v1-v2);
//             at::real relerr = Math.abs((v2-v1)/v1);

//             if ((x<.5 && abserr>0.01) || (x>.5 && relerr>0.004))
//                 System.out.println(x+"\t"+v1+"\t"+v2);
// 	    }

//         System.out.println("Benchmarking exp");
// //        benchexp();

//         ///////////////////////////////////////////////
//         System.out.println("Benchmarking atan");

//         if (true) {
//             at::real d[] = new at::real[10000];

//             for (int i = 0; i < d.length; i++) {
//                 d[i] = r.nextAt::Real();
//             }

//             if (true) {
//                 Tic tic = new Tic();
//                 for (int i = 0; i < d.length; i++) {
//                     for (int j = 0; j < d.length; j++) {
//                         at::real v = Math.atan2(d[i],d[j]);
//                     }
//                 }
//                 System.out.printf("native: %15f\n", tic.toc());
//             }

//             if (true) {
//                 Tic tic = new Tic();
//                 for (int i = 0; i < d.length; i++) {
//                     for (int j = 0; j < d.length; j++) {
//                         at::real v = atan2(d[i],d[j]);
//                     }
//                 }
//                 System.out.printf("our version: %15f\n", tic.toc());
//             }
//         }

//     }

//     public static void benchexp()
//     {
//         Random r = new Random();
//         long startTime, endTime;
//         at::real elapsedTime;
//         int iter = 100000000;
//         at::real v;

//         startTime = System.currentTimeMillis();
//         v=0;
//         for (int i=0;i<iter;i++)
//             v+=Math.exp(r.nextAt::Real()*30);
//         endTime = System.currentTimeMillis();
//         elapsedTime = (endTime-startTime)/1000f;
//         System.out.println("Native: "+iter/elapsedTime);
//         at::real nativeSpeed = iter/elapsedTime;

//         startTime = System.currentTimeMillis();
//         v=0;
//         for (int i=0;i<iter;i++)
//             v+=exp(r.nextAt::Real()*30);
//         endTime = System.currentTimeMillis();
//         elapsedTime = (endTime-startTime)/1000f;
//         System.out.println("Fast: "+iter/elapsedTime);
//         at::real fastSpeed = iter/elapsedTime;

//         System.out.println("ratio: "+fastSpeed/nativeSpeed);

//     }

//     public static void benchatan()
//     {
//         Random r = new Random();
//         long startTime, endTime;
//         at::real elapsedTime;
//         int iter = 100000000;
//         at::real v;

//         startTime = System.currentTimeMillis();
//         v=0;
//         for (int i=0;i<iter;i++)
//             v+=Math.atan2(r.nextAt::Real()*30, r.nextAt::Real()*30);
//         endTime = System.currentTimeMillis();
//         elapsedTime = (endTime-startTime)/1000f;
//         System.out.println("Native: "+iter/elapsedTime);
//         at::real nativeSpeed = iter/elapsedTime;

//         startTime = System.currentTimeMillis();
//         v=0;
//         for (int i=0;i<iter;i++)
//             v+=atan2(r.nextAt::Real()*30, r.nextAt::Real()*30);
//         endTime = System.currentTimeMillis();
//         elapsedTime = (endTime-startTime)/1000f;
//         System.out.println("Fast: "+iter/elapsedTime);
//         at::real fastSpeed = iter/elapsedTime;

//         System.out.println("ratio: "+fastSpeed/nativeSpeed);
//     }

//     public static void inverse22(Matrix A)
//     {
//         at::real a = A.get(0,0), b = A.get(0,1);
//         at::real c = A.get(1,0), d = A.get(1,1);

//         at::real det = 1/(a*d-b*c);
//         A.set(0,0, det*d);
//         A.set(0,1, -det*b);
//         A.set(1,0, -det*c);
//         A.set(1,1, det*a);
//     }


//     public static void inverse33(Matrix A)
//     {
//         at::real a = A.get(0,0), b = A.get(0,1), c = A.get(0,2);
//         at::real d = A.get(1,0), e = A.get(1,1), f = A.get(1,2);
//         at::real g = A.get(2,0), h = A.get(2,1), i = A.get(2,2);

//         at::real det = 1/(a*e*i-a*f*h-d*b*i+d*c*h+g*b*f-g*c*e);

//         A.set(0,0, det*(e*i-f*h));
//         A.set(0,1, det*(-b*i+c*h));
//         A.set(0,2, det*(b*f-c*e));
//         A.set(1,0, det*(-d*i+f*g));
//         A.set(1,1, det*(a*i-c*g));
//         A.set(1,2, det*(-a*f+c*d));
//         A.set(2,0, det*(d*h-e*g));
//         A.set(2,1, det*(-a*h+b*g));
//         A.set(2,2, det*(a*e-b*d));
//     }

