//
//  HullWhiteTS.h
//  Seminaire
//
//  Created by Alexandre HUMEAU on 16/01/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef Seminaire_HullWhiteTS_h
#define Seminaire_HullWhiteTS_h

#include "Integral.h"

namespace Maths {
    class HullWhiteTS : public TermStructureIntegral
    {
    protected:
        double dLambda_;
    public:
        HullWhiteTS(const Finance::TermStructure<double,double> & sTermStructure, const double dLambda);
        virtual ~HullWhiteTS();
        
        //  primitive of function f() define in the above integral
		virtual double SubIntegral(double dA, double dB) const;
    };
}

#endif
