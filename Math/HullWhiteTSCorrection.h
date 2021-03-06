/*
 *  HullWhiteTSCorrection.h
 *  Seminaire
 *
 *  Created by Emile on 2/4/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Seminaire_HullWhiteTSCorrection_h
#define Seminaire_HullWhiteTSCorrection_h

#include "Integral.h"

namespace Maths {
	// cross terms in the integral of the multiplicative quanto adjustment
	// rho * gamma_f(T1,T2) * gamma_d(t,T2)
	// the associated term structure is sigma_d * sigma_f
    class HullWhiteTSCorrection : public TermStructureIntegral
    {
    protected:
        double dLambdaDiscount_;
		double dLambdaForward_;
		double dT1_;
		double dT2_;
    public:
        HullWhiteTSCorrection(const Finance::TermStructure<double,double> & sTermStructure, double dLambdaDiscount, double dLambdaForward/*, const double dRho*/, double dT1, double dT2);
        virtual ~HullWhiteTSCorrection();
        
        //  primitive of function f() define in the above integral
		virtual double SubIntegral(double dA, double dB) const;
		
        
        
    };
}

#endif
