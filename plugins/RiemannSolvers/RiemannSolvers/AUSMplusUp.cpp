#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"

#include "RiemannSolvers/AUSMplusUp.hpp"

namespace cf3 {
namespace RiemannSolvers {

using namespace common;
using namespace physics;

common::ComponentBuilder < AUSMplusUp, RiemannSolver, LibRiemannSolvers > AUSMplusUp_Builder;

////////////////////////////////////////////////////////////////////////////////

AUSMplusUp::AUSMplusUp ( const std::string& name ) :
    RiemannSolver(name),
    m_fa (0.0),
    m_CoeffKu(0.75),
    m_CoeffKp(0.25),
    m_Coeffsigma(1.),
    m_Machinf(0.),
    m_Beta(1./8.)
{
  options().option("physical_model").attach_trigger( boost::bind( &AUSMplusUp::trigger_physical_model, this) );

  options().add_option("Ku",m_CoeffKu)
    .description("Ku")
    .link_to(&m_CoeffKu);

  options().add_option("Kp",m_CoeffKp)
    .description("Kp")
    .link_to(&m_CoeffKp);

  options().add_option("sigma",m_Coeffsigma)
    .description("sigma")
    .link_to(&m_Coeffsigma);

  options().add_option("machinf",m_Machinf)
    .description("machinf")
    .link_to(&m_Machinf);

  options().add_option("beta",m_Beta)
    .description("beta")
    .link_to(&m_Beta);

}

////////////////////////////////////////////////////////////////////////////////

AUSMplusUp::~AUSMplusUp()
{
}

////////////////////////////////////////////////////////////////////////////////

void AUSMplusUp::trigger_physical_model()
{
    if (physical_model().derived_type_name() != "cf3.physics.NavierStokes.NavierStokes2D")
        throw SetupError (FromHere(),"Physical model not correct");

  coord.resize(physical_model().ndim());
  grads.resize(physical_model().neqs(),physical_model().ndim());
  std::auto_ptr<physics::Properties> p_left_auto = physical_model().create_properties();
  physics::NavierStokes::NavierStokes2D::Properties* p_left_ptr = reinterpret_cast<physics::NavierStokes::NavierStokes2D::Properties*>(p_left_auto.release());
  p_left = std::auto_ptr<physics::NavierStokes::NavierStokes2D::Properties>(p_left_ptr);

  std::auto_ptr<physics::Properties> p_right_auto = physical_model().create_properties();
  physics::NavierStokes::NavierStokes2D::Properties* p_right_ptr = reinterpret_cast<physics::NavierStokes::NavierStokes2D::Properties*>(p_right_auto.release());
  p_right = std::auto_ptr<physics::NavierStokes::NavierStokes2D::Properties>(p_right_ptr);


  f_left.resize(physical_model().neqs(),physical_model().ndim());
  f_right.resize(physical_model().neqs(),physical_model().ndim());

  eigenvalues.resize(physical_model().neqs());
//  eigenvalues_left.resize(physical_model().neqs());
//  eigenvalues_right.resize(physical_model().neqs());
  right_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  left_eigenvectors.resize(physical_model().neqs(),physical_model().neqs());
  abs_jacobian.resize(physical_model().neqs(),physical_model().neqs());

  // Try to configure solution_vars automatically
  if (is_null(m_solution_vars))
  {
    if (Handle<Component> found_solution_vars = find_component_ptr_recursively_with_name(physical_model(),"solution_vars"))
    {
      options().configure_option("solution_vars",found_solution_vars->uri());
    }
    else
    {
      CFwarn << "AUSMplusUp RiemannSolver " << uri().string() << " could not auto-config \"solution_vars\".\n"
             << "Reason: component with name \"solution_vars\" not found in ["<<physical_model().uri().string() << "]\n"
             << "Configure manually" << CFendl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
Real AUSMplusUp::M1(Real Mach, char chsign)
{
    Real sign;
    switch (chsign) {
      case '+':
        sign = 1.;
        break;
      case '-':
        sign = -1.;
        break;
      default:
        std::cout << "sign M1 is unknown \n";
      }
    return 0.5*(Mach + sign*abs(Mach));
}

Real AUSMplusUp::M2(Real Mach, char chsign)
{
    Real sign;
    switch (chsign) {
      case '+':
        sign = 1.;
        break;
      case '-':
        sign = -1.;
        break;
      default:
        std::cout << "sign M1 is unknown \n";
      }
    return sign*0.25*(Mach + sign*1.)*(Mach + sign*1.);
}

Real AUSMplusUp::M4(Real Mach, char chsign)
{
    Real sign;
    char invchsign = '+';
    switch (chsign) {
      case '+':
        sign = 1.;
        invchsign = '-';
        break;
      case '-':
        sign = -1.;
        break;
      default:
        std::cout << "sign M1 is unknown \n";
      }
    return abs(Mach) >= 1. ? M1(Mach, chsign) : M2(Mach, chsign)*(1.-sign*16. * m_Beta * M2(Mach, invchsign));
}

Real AUSMplusUp::P5(Real Mach, Real alpha, char chsign)
{
    Real sign;
    char invchsign = '+';
    switch (chsign) {
      case '+':
        sign = 1.;
        invchsign = '-';
        break;
      case '-':
        sign = -1.;
        break;
      default:
        std::cout << "sign M1 is unknown \n";
      }

    return abs(Mach) >= 1. ? 1./Mach*M1(Mach, chsign) : M2(Mach, chsign)*((sign*2.-Mach)-sign*16.*alpha*Mach*M2(Mach, invchsign));
}

////////////////////////////////////////////////////////////////////////////////

void AUSMplusUp::compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                       RealVector& flux)
{
    physics::Variables& sol_vars = *m_solution_vars;
    sol_vars.compute_properties(coord, left, grads, *p_left);
    sol_vars.compute_properties(coord, right, grads, *p_right);

    RealVector U_left(2)  ; U_left << p_left->u, p_left->v;
    RealVector U_right(2)  ;  U_right << p_right->u, p_right->v;

    Real a12 = 0.5*(p_left->a + p_right->a);

    Real M_left = Real(U_left.transpose() * normal) / a12;
    Real M_right = Real(U_right.transpose() * normal) / a12;

    Real Mbar2 =( Real(U_left.transpose()*U_left) + Real(U_right.transpose()*U_right))/(2*a12*a12);

    Real M02 = 1 < (Mbar2 > m_Machinf*m_Machinf ? Mbar2 : m_Machinf*m_Machinf) ? 1 : (Mbar2 > m_Machinf*m_Machinf ? Mbar2 : m_Machinf*m_Machinf);
    m_fa = sqrt(M02)*(2-sqrt(M02));

    Real rho12 = 0.5*(p_right->rho+p_left->rho);

    Real M12 = -m_CoeffKp/m_fa*(((1-m_Coeffsigma*Mbar2)) > 0 ? (1-m_Coeffsigma*Mbar2) : 0 );
    M12 *=(p_right->P - p_left->P);
    M12 /= (rho12*a12*a12);
    M12 += M4(M_left, '+') + M4(M_right,'-');

    Real alpha = 3./16.*(-4.+5*m_fa*m_fa);

    Real P12 = P5(M_left, alpha, '+')* p_left->P + P5(M_right, alpha, '-')* p_right->P;
    P12 -= (m_CoeffKu*P5(M_left, alpha, '+')*P5(M_right, alpha, '-')*(p_right->rho+p_left->rho)*(m_fa*a12)*(Real(U_right.transpose()*normal) - Real(U_left.transpose()*normal)));

    Real mdot12 = a12*M12*(M12 > 0 ? p_left->rho : p_right->rho);

    sol_vars.flux(*p_left, f_left);
    sol_vars.flux(*p_right, f_right);

    RealMatrix psi_left(4,2);
    RealMatrix psi_right(4,2);
    RealMatrix tmp(4,2);
    tmp(0,XX) = 0;
    tmp(1,XX) = p_left->P;
    tmp(2,XX) = 0;
    tmp(3,XX) = 0;

    tmp(0,YY) =0;
    tmp(1,YY) =0;
    tmp(2,YY) =p_left->P;
    tmp(3,YY) =0;

    psi_left = (f_left - tmp);
    psi_left /= (p_left->rho * sqrt(p_left->uuvv));

    tmp *=0;
    tmp(1,XX) = p_right->P;
    tmp(2,YY) =p_right->P;

    psi_right = (f_right - tmp);
    psi_right /= (p_right->rho * sqrt(p_right->uuvv));

    flux = mdot12 * ( mdot12 > 0 ? psi_left*normal : psi_right*normal);
    tmp /= p_right->P * P12;
    flux += (tmp*normal);


}

////////////////////////////////////////////////////////////////////////////////

void AUSMplusUp::compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                      RealVector& flux, RealVector& wave_speeds)
{
  compute_interface_flux(left,right,coords,normal,flux);
  /// @todo compute eigenvalues
  wave_speeds = eigenvalues;
}

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3
