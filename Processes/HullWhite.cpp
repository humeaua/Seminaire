//
//  HullWhite.cpp
//  Seminaire
//
//  Created by Alexandre HUMEAU on 19/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "HullWhite.h"
#include "Require.h"
#include "Gaussian.h"
#include "MathFunctions.h"

namespace Processes {
    
    LinearGaussianMarkov::LinearGaussianMarkov()
    {}
    
    LinearGaussianMarkov::LinearGaussianMarkov(const Finance::YieldCurve & sDiscountCurve, double dLambda, const Finance::TermStructure<double, double> & dSigma) : dLambda_(dLambda)
    {
        sDiscountCurve_ = sDiscountCurve;
        sForwardCurve_ = sDiscountCurve;

        dSigma_ = dSigma;
    }
    
    LinearGaussianMarkov::LinearGaussianMarkov(const Finance::YieldCurve & sDiscountCurve, const Finance::YieldCurve & sForwardCurve, double dLambda, const Finance::TermStructure<double, double> & dSigma) : dLambda_(dLambda), dSigma_(dSigma)
    {
        sDiscountCurve_ = sDiscountCurve;
        sForwardCurve_ = sForwardCurve;
    }
    
    LinearGaussianMarkov::~LinearGaussianMarkov()
    {}
    
    void LinearGaussianMarkov::Simulate(std::size_t iNRealisations,
                                        const std::vector<double> & dSimulationTenors,
                                        Finance::SimulationData & sSimulationData,
                                        bool bIsStepByStepMC) const
    {
        Utilities::require(!dSimulationTenors.empty(), "Simulation Times is empty");
        Utilities::require(iNRealisations > 0, "Number of paths has to be positive");
        
        sSimulationData.SetDates(dSimulationTenors);
        
        std::vector<double> dSimulationTenorsCopy;
        dSimulationTenorsCopy.insert(dSimulationTenorsCopy.begin(), 0.0);
        dSimulationTenorsCopy.insert(dSimulationTenorsCopy.end(), dSimulationTenors.begin(), dSimulationTenors.end());
        
        //  In this function, we will simulate the factor \int_{s}^{t} a(u) dW^Q_u for s,t in the simulation tenors vector
        std::size_t iNTenors = dSimulationTenorsCopy.size();
        RandomNumbers::Gaussian1D sGaussian(0.0, 1.0, iNRealisations * iNTenors, 0);
        sGaussian.GenerateGaussian();
        std::vector<double> dGaussianRealisations = sGaussian.GetRealisations();
        
        if (bIsStepByStepMC)
        {
            //  Step by Step Monte Carlo
            for (std::size_t iSimulationTenor = 0 ; iSimulationTenor < iNTenors - 1 ; ++iSimulationTenor)
            {
                double dVariance = 0.0;
                if (!dSigma_.IsTermStructure())
                {
                    dVariance= dSigma_.GetValues()[0] * dSigma_.GetValues()[0] * MathFunctions::Beta_OU(-2.0 * dLambda_, dSimulationTenorsCopy[iSimulationTenor + 1]);
                }
                else
                {
                    std::cout<<"Not yet implemented"<<std::endl;
                }
                
                for (std::size_t iPath = 0 ; iPath < iNRealisations ; ++iPath)
                {
                    double dCurrentValue = dGaussianRealisations[iPath + iNRealisations * iSimulationTenor] * sqrt(dVariance);
                    sSimulationData.Put(iSimulationTenor, iPath, std::vector<double>(1, dCurrentValue));
                    if (1) // Antithetic variables
                    {
                        sSimulationData.Put(iSimulationTenor, iNRealisations + iPath, std::vector<double>(1, -dCurrentValue));
                    }
                }
            }
        }
        else
        {
            //  Path by Path Monte Carlo
            
            //  Compute the standard deviation
            std::vector<double> dStdDev(iNTenors - 1, 0.0);
            if (!dSigma_.IsTermStructure())
            {
                std::cout << "LinearGaussianMarkov::Simulate : Simulation with no Term-structure" << std::endl;
                std::cout << "Not yet implemented"<<std::endl;
            }
            else
            {
                std::cout << "LinearGaussianMarkov::Simulate : Simulation with Term-structure" << std::endl;
                
                std::cout<<"Not yet implemented"<<std::endl;
            }
            //  Begin the simulation
            for (std::size_t iPath = 0 ; iPath < iNRealisations ; ++iPath)
            {
                //  We simulate the wanted factor
                double dCurrentValue = 0.0;
                for (std::size_t iSimulationTenor = 0 ; iSimulationTenor < iNTenors - 1; ++iSimulationTenor)
                {
                    dCurrentValue = dGaussianRealisations[iPath + iSimulationTenor * iNRealisations] * dStdDev[iSimulationTenor];
                    sSimulationData.Put(iSimulationTenor, iPath, std::vector<double>(1,dCurrentValue));
                    if (1) // Antithetic variables
                    {
                        sSimulationData.Put(iSimulationTenor, iPath + iNRealisations, std::vector<double>(1,- dCurrentValue));
                    }
                }
                
            }
        }
    }
    
    void LinearGaussianMarkov::ChangeOfProbability(double dT, 
                                                   const Finance::SimulationData &sSimulationDataRiskNeutral, 
                                                   Finance::SimulationData &sSimulationDataTForward) const
    {
        //  Create a new simulated data object w.r.t. the T-forward neutral probability
        //  The results factors sSimulationDataTForward are martingales under the T-forward neutral probability and the input factors sSimulationDataRiskNeutral are martingales under the risk neutral probability
        
        sSimulationDataTForward.SetDates(sSimulationDataRiskNeutral.GetDateList());
		
        std::vector<long> lDates = sSimulationDataRiskNeutral.GetDateList();
        Finance::SimulationData::Data sDataRiskNeutral = sSimulationDataRiskNeutral.GetData();
		
        for (std::size_t iDate = 0 ; iDate < lDates.size() ; ++iDate)
        {
            double dDate = lDates[iDate] / 365.0;
            
            double dBracket = BracketChangeOfProbability(dDate, dT); //  Bracket of the factor X_t and dB(t,T) / B(t,T)
            
            for (std::size_t iPath =  0 ; iPath < sDataRiskNeutral.second[iDate].size() ; ++iPath)
            {
                std::vector<double> dTForwardValues;
                for (std::size_t iVar = 0 ; iVar < sDataRiskNeutral.second[iDate][iPath].size() ; ++iVar)
                {
                    dTForwardValues.push_back(sDataRiskNeutral.second[iDate][iPath][iVar] - dBracket);
                }
                sSimulationDataTForward.Put(iDate, iPath, dTForwardValues);
            }
			
        }
		
		std::cout << "Shift to " << dT << "Y-Forward Probability, done." << std::endl;
    }
    
    double LinearGaussianMarkov::BracketChangeOfProbability(double dt, double dT) const
    {
        if (dSigma_.IsTermStructure())
        {
            if (std::abs(dLambda_) > BETAOUTHRESHOLD)
            {
                std::size_t i = 1;
                std::vector<double> dTis = dSigma_.GetVariables(), dSigmaTis = dSigma_.GetValues();
                double dBracket = 0.0;
                
                //  First part in case first time in term structure of volatility is not 0
                
                dBracket += dSigmaTis[0] * dSigmaTis[0] / (dLambda_ * dLambda_) * (exp(dLambda_ * dTis[0]) - 1.0 - 0.5 * exp(-dLambda_ * dT) * (exp(2.0 * dLambda_ * dTis[0]) - 1.0));
                
                //  Loop over the time in the term structure of volatility
                
                while (dTis[i] < dt && i < dTis.size())
                {
                    //  Sigma is supposed to be càdlàg
                    dBracket += dSigmaTis[i - 1] * dSigmaTis[i - 1] / (dLambda_ * dLambda_) * (exp(dLambda_ * dTis[i]) - exp(dLambda_ * dTis[i - 1]) - exp(-dLambda_ * dT) * 0.5 * (exp(2.0 * dLambda_ * dTis[i]) - exp(-2.0 * dLambda_ * dTis[i - 1])));
                    i++;
                }
                
                //  Last term in the term structure
                //  
                //  Test if t is after the last date in the term-structure
                if (i == dTis.size())
                {
                    i--;
                }
                dBracket += dSigmaTis[i] * dSigmaTis[i] / (dLambda_ * dLambda_) * (exp(dLambda_ * dt) - exp(dLambda_ * dTis[i]) - 0.5 * exp(-dLambda_ * dT) * (exp(2.0 * dLambda_ * dt) - exp(2.0 * dLambda_ * dTis[i])));
                
                return dBracket;
            }
            else
            {
                std::size_t i = 1;
                std::vector<double> dTis = dSigma_.GetVariables(), dSigmaTis = dSigma_.GetValues();
                double dBracket = 0.0;
                
                //  First part in case first time in term structure of volatility is not 0
                
                dBracket += dSigmaTis[0] * dSigmaTis[0] * dTis[0] * (dT - dTis[0] * 0.5);
                
                //  Loop over the time in the term structure of volatility
                
                while (dTis[i] < dt && i < dTis.size())
                {
                    //  Sigma is supposed to be càdlàg
                    dBracket += dSigmaTis[i - 1] * dSigmaTis[i - 1] * (dTis[i] - dTis[i - 1]) * (dT - (dTis[i] + dTis[i - 1]) * 0.5);
                    i++;
                }
                
                //  Last term in the term structure
                //  
                //  Test if t is after the last date in the term-structure
                if (i == dTis.size())
                {
                    i--;
                }
                dBracket += dSigmaTis[i] * dSigmaTis[i] * (dt - dTis[i]) * (dT - (dt + dTis[i]) * 0.5);
                
                return dBracket;
            }
        }
        else
        {
            double dSigma = dSigma_.GetValues()[0];
            if (std::abs(dLambda_) > BETAOUTHRESHOLD)
            {
                return dSigma * dSigma / (dLambda_ * dLambda_) * (exp(dLambda_ * dt) - 1.0 - 0.5 * exp(-dLambda_ * dT) * (-1.0 + exp(2.0 * dLambda_ * dt)));
            }
            else
            {
                return dSigma * dSigma * dt * (dT - dt * 0.5);
            }
        }
    }
    
    double LinearGaussianMarkov::A(double t) const
    {
        return dSigma_.Interpolate(t) * exp(dLambda_ * t);
    }
    
    double LinearGaussianMarkov::B(double t) const
    {
        return exp(-dLambda_ * t);
    }
    
    double LinearGaussianMarkov::DeterministPart(double dt, double dT) const
    {
        //  Compute the integral \int_{0}^{t} a(s)^2 (\beta(t) + \beta(T) - 2\beta(s))ds
        if (dSigma_.IsTermStructure())
        {
            if (std::abs(dLambda_) > BETAOUTHRESHOLD)
            {
                std::size_t i = 1;
                std::vector<double> dTis = dSigma_.GetVariables(), dSigmaTis = dSigma_.GetValues();
                double dDeterministPart = 0.0;
                
                //  First part in case first time in term structure of volatility is not 0
                
                dDeterministPart += dSigmaTis[0] * dSigmaTis[0] / (dLambda_ * dLambda_) * (4.0 - (exp(-dLambda_ * dT) + exp(-dLambda_ * dt)) * (exp(dLambda_ * dTis[0]) + 1.0)) * (exp(dLambda_ * dTis[0]) - 1.0);
                
                //  Loop over the time in the term structure of volatility
                
                while (dTis[i] < dt && i < dTis.size())
                {
                    //  Sigma is supposed to be càdlàg
                    dDeterministPart += dSigmaTis[i-1] * dSigmaTis[i-1] / (dLambda_ * dLambda_) * (4.0 - (exp(-dLambda_ * dT) + exp(-dLambda_ * dt)) * (exp(dLambda_ * dTis[i]) + exp(dLambda_ * dTis[i-1]))) * (exp(dLambda_ * dTis[i]) - exp(dLambda_ * dTis[i-1]));
                    i++;
                }
                
                //  Last term in the term structure
                //  
                //  Test if t is after the last date in the term-structure
                if (i == dTis.size())
                {
                    i--;
                }
                dDeterministPart += dSigmaTis[i] * dSigmaTis[i] / (dLambda_ * dLambda_) * (4.0 - (exp(-dLambda_ * dT) + exp(-dLambda_ * dt)) * (dt + exp(dLambda_ * dTis[i]))) * (exp(dLambda_ * dt) - exp(dLambda_ * dTis[i]));;
                
                return 0.5 * dDeterministPart;
            }
            else
            {
                std::size_t i = 1;
                std::vector<double> dTis = dSigma_.GetVariables(), dSigmaTis = dSigma_.GetValues();
                double dDeterministPart = 0.0;
                
                //  First part in case first time in term structure of volatility is not 0
                
                dDeterministPart += dSigmaTis[0] * dSigmaTis[0] * dTis[0] * (dTis[0] - (dT + dt));
                
                //  Loop over the time in the term structure of volatility
                
                while (dTis[i] < dt && i < dTis.size())
                {
                    //  Sigma is supposed to be càdlàg
                    dDeterministPart += dSigmaTis[i-1] * dSigmaTis[i-1] * (dTis[i] - dTis[i-1]) * (dTis[i] + dTis[i-1] - (dT + dt));
                    i++;
                }
                
                //  Last term in the term structure
                //  
                //  Test if t is after the last date in the term-structure
                if (i == dTis.size())
                {
                    i--;
                }
                dDeterministPart += dSigmaTis[i] * dSigmaTis[i] * (dt - dTis[i]) * (dt + dTis[i] - (dT + dt));
                
                return 0.5 * dDeterministPart;
            }
        }
        else
        {
            double dSigma = dSigma_.GetValues()[0];
            if (std::abs(dLambda_) > BETAOUTHRESHOLD)
            {
  				return 0.5 * dSigma * dSigma / (dLambda_ * dLambda_) * (4.0 * (exp(dLambda_ * dt) - 1.0) - (exp(-dLambda_ * dt) + exp(-dLambda_ * dT)) *(exp(2.0 * dLambda_ * dt) - 1.0));
            }
            else
            {
                return -0.5 * dSigma * dSigma * dt * dt * dT;
            }
        }
    }
    
    double LinearGaussianMarkov::BondPrice(double dt, double dT, double dX, const CurveName & eCurveName) const
    {
        Utilities::require(dt <= dT);
        Finance::YieldCurve sYieldCurve;
        if (eCurveName == DISCOUNT)
        {
            sYieldCurve = sDiscountCurve_;
        }
        else 
        {
            sYieldCurve = sForwardCurve_;
        }
        if (dt < dT)
        {
            return exp(-sYieldCurve.YC(dT) * dT) / exp(-sYieldCurve.YC(dt) * dt) * exp((MathFunctions::Beta_OU(dLambda_, dT) - MathFunctions::Beta_OU(dLambda_, dt)) * ( -0.5 * DeterministPart(dt, dT) - dX));
        }
        else 
        {
            //  dt = dT
            return 1.0;
        }
    }
    
    double LinearGaussianMarkov::Libor(double dt, double dStart, double dEnd, double dX, const CurveName & eCurveName, double dQA) const
    {
        //  Must change coverage to take into account real basis
        double dDFStart = BondPrice(dt, dStart, dX, eCurveName);
        double dDFEnd = BondPrice(dt, dEnd, dX, eCurveName);
        return 1.0 / (dEnd - dStart) * (dDFStart / dDFEnd * dQA - 1.0);
    }
}