#ifndef EXCESSHE_FUNCTIONS_H
#define EXCESSHE_FUNCTIONS_H

#include <memory>
#include <vector>
#include "CoolPropFluid.h"
#include "crossplatform_shared_ptr.h"
#include "Helmholtz.h"
#include "Backends/Helmholtz/HelmholtzEOSMixtureBackend.h"

namespace CoolProp{

typedef std::vector<std::vector<CoolPropDbl> > STLMatrix;

/** \brief The abstract base class for departure functions used in the excess part of the Helmholtz energy
 * 
 * The only code included in the ABC is the structure for the derivatives of the Helmholtz energy with 
 * the reduced density and reciprocal reduced temperature
 */
class DepartureFunction
{
public:
    DepartureFunction(){};
    virtual ~DepartureFunction(){};
    ResidualHelmholtzGeneralizedExponential phi;
    HelmholtzDerivatives derivs;

    void update(double tau, double delta){
        derivs.reset(0.0);
        phi.all(tau, delta, derivs);
    };

    double alphar(){ return derivs.alphar;};
    double dalphar_dDelta(){ return derivs.dalphar_ddelta;};
    double dalphar_dTau(){ return derivs.dalphar_dtau;};
    
    double d2alphar_dDelta2(){return derivs.d2alphar_ddelta2;};
    double d2alphar_dDelta_dTau(){return derivs.d2alphar_ddelta_dtau;};
    double d2alphar_dTau2(){return derivs.d2alphar_dtau2;};

	double d3alphar_dTau3(){ return derivs.d3alphar_dtau3; };
	double d3alphar_dDelta_dTau2(){ return derivs.d3alphar_ddelta_dtau2; }; 
	double d3alphar_dDelta2_dTau(){ return derivs.d3alphar_ddelta2_dtau; }; 
	double d3alphar_dDelta3(){ return derivs.d3alphar_ddelta3; };

    double d4alphar_dTau4(){ return derivs.d4alphar_dtau4; };
    double d4alphar_dDelta_dTau3(){ return derivs.d4alphar_ddelta_dtau3; };
    double d4alphar_dDelta2_dTau2(){ return derivs.d4alphar_ddelta2_dtau2; }; 
    double d4alphar_dDelta3_dTau(){ return derivs.d4alphar_ddelta3_dtau; };
    double d4alphar_dDelta4(){ return derivs.d4alphar_ddelta4; };
};

/** \brief The departure function used by the GERG-2008 formulation
 * 
 * This departure function has a form like
 * \f[
 * \alphar^r_{ij} = \sum_k n_{ij,k}\delta^{d_{ij,k}}\tau^{t_{ij,k}} + \sum_k n_{ij,k}\delta^{d_{ij,k}}\tau^{t_{ij,k}}\exp[-\eta_{ij,k}(\delta-\varepsilon_{ij,k})^2-\beta_{ij,k}(\delta-\gamma_{ij,k})]
 * \f]
 * It is symmetric so \f$\alphar^r_{ij} = \alphar^r_{ji}\f$
 */
class GERG2008DepartureFunction : public DepartureFunction
{    
public:
    GERG2008DepartureFunction(){};
    GERG2008DepartureFunction(const std::vector<double> &n,const std::vector<double> &d,const std::vector<double> &t,
                              const std::vector<double> &eta,const std::vector<double> &epsilon,const std::vector<double> &beta,
                              const std::vector<double> &gamma, std::size_t Npower)
    {
        /// Break up into power and gaussian terms
        {
            std::vector<CoolPropDbl> _n(n.begin(), n.begin()+Npower);
            std::vector<CoolPropDbl> _d(d.begin(), d.begin()+Npower);
            std::vector<CoolPropDbl> _t(t.begin(), t.begin()+Npower);
            std::vector<CoolPropDbl> _l(Npower, 0.0);
            phi.add_Power(_n, _d, _t, _l);
        }
        if (n.size() == Npower)
        {
        }
        else
        {
            std::vector<CoolPropDbl> _n(n.begin()+Npower,                   n.end());
            std::vector<CoolPropDbl> _d(d.begin()+Npower,                   d.end());
            std::vector<CoolPropDbl> _t(t.begin()+Npower,                   t.end());
            std::vector<CoolPropDbl> _eta(eta.begin()+Npower,             eta.end());
            std::vector<CoolPropDbl> _epsilon(epsilon.begin()+Npower, epsilon.end());
            std::vector<CoolPropDbl> _beta(beta.begin()+Npower,          beta.end());
            std::vector<CoolPropDbl> _gamma(gamma.begin()+Npower,       gamma.end());
            phi.add_GERG2008Gaussian(_n, _d, _t, _eta, _epsilon, _beta, _gamma);
        }
    };
    ~GERG2008DepartureFunction(){};
};

/** \brief A polynomial/exponential departure function
 * 
 * This departure function has a form like
 * \f[
 * \alpha^r_{ij} = \sum_k n_{ij,k}\delta^{d_{ij,k}}\tau^{t_{ij,k}}\exp(-\delta^{l_{ij,k}})
 * \f]
 * It is symmetric so \f$\alphar^r_{ij} = \alphar^r_{ji}\f$
 */
class ExponentialDepartureFunction : public DepartureFunction
{
public:
    ExponentialDepartureFunction(){};
    ExponentialDepartureFunction(const std::vector<double> &n, const std::vector<double> &d,
                                 const std::vector<double> &t, const std::vector<double> &l)
                                 {
                                     std::vector<CoolPropDbl> _n(n.begin(), n.begin()+n.size());
                                     std::vector<CoolPropDbl> _d(d.begin(), d.begin()+d.size());
                                     std::vector<CoolPropDbl> _t(t.begin(), t.begin()+t.size());
                                     std::vector<CoolPropDbl> _l(l.begin(), l.begin()+l.size());
                                     phi.add_Power(_n, _d, _t, _l);
                                 };
    ~ExponentialDepartureFunction(){};
};

typedef shared_ptr<DepartureFunction> DepartureFunctionPointer;

class ExcessTerm
{
public:
    std::size_t N;
    std::vector<std::vector<DepartureFunctionPointer> > DepartureFunctionMatrix;
    STLMatrix F;

    ExcessTerm():N(0){};

    /// Resize the parts of this term
    void resize(std::size_t N){
        this->N = N;
        F.resize(N, std::vector<CoolPropDbl>(N, 0));
        DepartureFunctionMatrix.resize(N);
        for (std::size_t i = 0; i < N; ++i){
            DepartureFunctionMatrix[i].resize(N);
        }
    };
    /// Update the internal cached derivatives in each departure function
    void update(double tau, double delta){
        for (std::size_t i = 0; i < N; i++){
            for (std::size_t j = i + 1; j < N; j++){
                DepartureFunctionMatrix[i][j]->update(tau, delta);
            }
            for (std::size_t j = 0; j < i; j++){
                DepartureFunctionMatrix[i][j]->update(tau, delta);
            }
        }
    }

    /// Calculate all the derivatives that do not involve any composition derivatives
    virtual HelmholtzDerivatives all(const CoolPropDbl tau, const CoolPropDbl delta, const std::vector<CoolPropDbl> &mole_fractions, bool cache_values = false)
    {
        HelmholtzDerivatives derivs;

        // If there is no excess contribution, just stop and return
        if (N == 0){ return derivs; }

        update(tau, delta);
        
        derivs.alphar = alphar(mole_fractions);
        derivs.dalphar_ddelta = dalphar_dDelta(mole_fractions);
        derivs.dalphar_dtau = dalphar_dTau(mole_fractions);

        derivs.d2alphar_ddelta2 = d2alphar_dDelta2(mole_fractions);
        derivs.d2alphar_ddelta_dtau = d2alphar_dDelta_dTau(mole_fractions);
        derivs.d2alphar_dtau2 = d2alphar_dTau2(mole_fractions);

        derivs.d3alphar_ddelta3 = d3alphar_dDelta3(mole_fractions);
        derivs.d3alphar_ddelta2_dtau = d3alphar_dDelta2_dTau(mole_fractions);
        derivs.d3alphar_ddelta_dtau2 = d3alphar_dDelta_dTau2(mole_fractions);
        derivs.d3alphar_dtau3 = d3alphar_dTau3(mole_fractions);

        derivs.d4alphar_ddelta4 = d4alphar_dDelta4(mole_fractions);
        derivs.d4alphar_ddelta3_dtau = d4alphar_dDelta3_dTau(mole_fractions);
        derivs.d4alphar_ddelta2_dtau2 = d4alphar_dDelta2_dTau2(mole_fractions);
        derivs.d4alphar_ddelta_dtau3 = d4alphar_dDelta_dTau3(mole_fractions);
        derivs.d4alphar_dtau4 = d4alphar_dTau4(mole_fractions);
        return derivs;
    }

    double alphar(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->alphar();
            }
        }
        return summer;
    }
    double dalphar_dDelta(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->dalphar_dDelta();
            }
        }
        return summer;
    }
    double d2alphar_dDelta2(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dDelta2();
            }
        }
        return summer;
    };
    double d2alphar_dDelta_dTau(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dDelta_dTau();
            }
        }
        return summer;
    }
    double dalphar_dTau(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->dalphar_dTau();
            }
        }
        return summer;
    };
    double d2alphar_dTau2(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N-1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i]*x[j]*F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dTau2();
            }
        }
        return summer;
    };
	double d3alphar_dTau3(const std::vector<CoolPropDbl> &x)
	{
		double summer = 0;
		for (std::size_t i = 0; i < N - 1; i++)
		{
			for (std::size_t j = i + 1; j < N; j++)
			{
				summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d3alphar_dTau3();
			}
		}
		return summer;
	};
	double d3alphar_dDelta_dTau2(const std::vector<CoolPropDbl> &x)
	{
		double summer = 0;
		for (std::size_t i = 0; i < N - 1; i++)
		{
			for (std::size_t j = i + 1; j < N; j++)
			{
				summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d3alphar_dDelta_dTau2();
			}
		}
		return summer;
	};
	double d3alphar_dDelta2_dTau(const std::vector<CoolPropDbl> &x)
	{
		double summer = 0;
		for (std::size_t i = 0; i < N - 1; i++)
		{
			for (std::size_t j = i + 1; j < N; j++)
			{
				summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d3alphar_dDelta2_dTau();
			}
		}
		return summer;
	};
	double d3alphar_dDelta3(const std::vector<CoolPropDbl> &x)
	{
		double summer = 0;
		for (std::size_t i = 0; i < N - 1; i++)
		{
			for (std::size_t j = i + 1; j < N; j++)
			{
				summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d3alphar_dDelta3();
			}
		}
		return summer;
	};
    double d4alphar_dTau4(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N - 1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d4alphar_dTau4();
            }
        }
        return summer;
    };
    double d4alphar_dDelta_dTau3(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N - 1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d4alphar_dDelta_dTau3();
            }
        }
        return summer;
    };
    double d4alphar_dDelta2_dTau2(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N - 1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d4alphar_dDelta2_dTau2();
            }
        }
        return summer;
    };
    double d4alphar_dDelta3_dTau(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N - 1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d4alphar_dDelta3_dTau();
            }
        }
        return summer;
    };
    double d4alphar_dDelta4(const std::vector<CoolPropDbl> &x)
    {
        double summer = 0;
        for (std::size_t i = 0; i < N - 1; i++)
        {
            for (std::size_t j = i + 1; j < N; j++)
            {
                summer += x[i] * x[j] * F[i][j] * DepartureFunctionMatrix[i][j]->d4alphar_dDelta4();
            }
        }
        return summer;
    };

    double dalphar_dxi(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k]*F[i][k]*DepartureFunctionMatrix[i][k]->alphar();
                }
            }
            return summer;
        }
        else if (xN_flag == XN_DEPENDENT) {
            CoolPropDbl dar_dxi = 0.0;
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->alphar();
            dar_dxi += (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i == k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->alphar();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->alphar();
                dar_dxi += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return dar_dxi;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }

    };
    double d2alphardxidxj(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->alphar();
            }
            else
            {
                return 0;
            }
        }
        else if (xN_flag == XN_DEPENDENT){
            std::size_t N = x.size();
            if (i == N-1 || j == N-1){ return 0; }
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->alphar();
            if (i == j) { return -2*FiNariN; }
            double Fijarij = F[i][j]*DepartureFunctionMatrix[i][j]->alphar();
            double FjNarjN = F[j][N-1]*DepartureFunctionMatrix[j][N-1]->alphar();
            return Fijarij - FiNariN - FjNarjN;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d3alphar_dxi_dxj_dDelta(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->dalphar_dDelta();
            }
            else
            {
                return 0;
            }
        }
        else if (xN_flag == XN_DEPENDENT)
        {
            std::size_t N = x.size();
            if (i == N-1 || j == N-1){ return 0; }
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->dalphar_dDelta();
            if (i == j) { return -2*FiNariN; }
            double Fijarij = F[i][j]*DepartureFunctionMatrix[i][j]->dalphar_dDelta();
            double FjNarjN = F[j][N-1]*DepartureFunctionMatrix[j][N-1]->dalphar_dDelta();
            return Fijarij - FiNariN - FjNarjN;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d3alphar_dxi_dxj_dTau(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->dalphar_dTau();
            }
            else
            {
                return 0;
            }
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d4alphar_dxi_dxj_dDelta2(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dDelta2();
            }
            else
            {
                return 0;
            }
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d4alphar_dxi_dxj_dDelta_dTau(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT)
        {
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dDelta_dTau();
            }
            else
            {
                return 0;
            }
        }
        else if (xN_flag == XN_DEPENDENT){
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->d2alphar_dDelta_dTau();
            CoolPropDbl d3ar_dxi_dDelta_dTau = (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i==k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->d2alphar_dDelta_dTau();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->d2alphar_dDelta_dTau();
                d3ar_dxi_dDelta_dTau += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return d3ar_dxi_dDelta_dTau;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d4alphar_dxi_dxj_dTau2(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            if (i != j)
            {
                return F[i][j]*DepartureFunctionMatrix[i][j]->d2alphar_dTau2();
            }
            else
            {
                return 0;
            }
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };

    double d3alphardxidxjdxk(const std::vector<CoolPropDbl> &x, std::size_t i, std::size_t j, std::size_t k, x_N_dependency_flag xN_flag)
    {
        return 0;
    };
    double d2alphar_dxi_dTau(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k]*F[i][k]*DepartureFunctionMatrix[i][k]->dalphar_dTau();
                }
            }
            return summer;
        }
        else if (xN_flag== XN_DEPENDENT){
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->dalphar_dTau();
            CoolPropDbl d2ar_dxi_dTau = (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i==k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->dalphar_dTau();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->dalphar_dTau();
                d2ar_dxi_dTau += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return d2ar_dxi_dTau;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d2alphar_dxi_dDelta(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT)
        {
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k]*F[i][k]*DepartureFunctionMatrix[i][k]->dalphar_dDelta();
                }
            }
            return summer;
        }
        else if (xN_flag == XN_DEPENDENT)
        {
            CoolPropDbl d2ar_dxi_dDelta = 0;
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->dalphar_dDelta();
            d2ar_dxi_dDelta += (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i==k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->dalphar_dDelta();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->dalphar_dDelta();
                d2ar_dxi_dDelta += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return d2ar_dxi_dDelta;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d3alphar_dxi_dDelta2(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
	{
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d2alphar_dDelta2();
                }
            }
            return summer;
        }
        else if (xN_flag == XN_DEPENDENT){
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->d2alphar_dDelta2();
            CoolPropDbl d3ar_dxi_dDelta2 = (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i==k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->d2alphar_dDelta2();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->d2alphar_dDelta2();
                d3ar_dxi_dDelta2 += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return d3ar_dxi_dDelta2;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
	};
    double d4alphar_dxi_dDelta3(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d3alphar_dDelta3();
                }
            }
            return summer;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d3alphar_dxi_dTau2(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
	{
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d2alphar_dTau2();
                }
            }
            return summer;
        }
        else if (xN_flag == XN_DEPENDENT)
        {
            double FiNariN = F[i][N-1]*DepartureFunctionMatrix[i][N-1]->d2alphar_dTau2();
            CoolPropDbl d3ar_dxi_dTau2 = (1-2*x[i])*FiNariN;
            for (std::size_t k = 0; k < N-1; ++k){
                if (i==k) continue;
                double Fikarik = F[i][k]*DepartureFunctionMatrix[i][k]->d2alphar_dTau2();
                double FkNarkN = F[k][N-1]*DepartureFunctionMatrix[k][N-1]->d2alphar_dTau2();
                d3ar_dxi_dTau2 += x[k]*(Fikarik - FiNariN - FkNarkN);
            }
            return d3ar_dxi_dTau2;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
	};
    double d4alphar_dxi_dTau3(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d3alphar_dTau3();
                }
            }
            return summer;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d3alphar_dxi_dDelta_dTau(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
	{
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d2alphar_dDelta_dTau();
                }
            }
            return summer;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
	};
    double d4alphar_dxi_dDelta2_dTau(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d3alphar_dDelta2_dTau();
                }
            }
            return summer;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
    double d4alphar_dxi_dDelta_dTau2(const std::vector<CoolPropDbl> &x, std::size_t i, x_N_dependency_flag xN_flag)
    {
        if (xN_flag == XN_INDEPENDENT){
            double summer = 0;
            for (std::size_t k = 0; k < N; k++)
            {
                if (i != k)
                {
                    summer += x[k] * F[i][k] * DepartureFunctionMatrix[i][k]->d3alphar_dDelta_dTau2();
                }
            }
            return summer;
        }
        else{
            throw ValueError(format("xN_flag is invalid"));
        }
    };
};

} /* namespace CoolProp */
#endif
