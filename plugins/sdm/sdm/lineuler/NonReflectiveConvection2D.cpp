// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/lineuler/NonReflectiveConvection2D.hpp"
#include "sdm/SDSolver.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<NonReflectiveConvection2D,Term,LibLinEuler> NonReflectiveConvection2D_builder;

/////////////////////////////////////////////////////////////////////////////

void NonReflectiveConvection2D::compute_non_reflective_fluxes(std::vector<RealVectorNEQS>& flux_in_flx_pts)
{
  RealVectorNEQS char_sol;
  RealVector char_normal = characteristic_normal();

  boost_foreach(Uint flx_pt, elem->get().sf->interior_flx_pts())
  {
    compute_flx_pt_phys_data(elem->get(),flx_pt,*flx_pt_data);
    {
      compute_analytical_non_reflective_flux(*flx_pt_data,flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt], char_normal,
                                             flux_in_flx_pts[flx_pt],flx_pt_wave_speed[flx_pt][0]);
      flux_in_flx_pts[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    }
  }
  /// 2) Calculate flux in face flux points
  for(m_face_nb=0; m_face_nb<elem->get().sf->nb_faces(); ++m_face_nb)
  {
    /// 2.1) Compute physical data in face
    compute_face();

    for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
    {

      // Set BC, default extrapolation
      if (m_face_nb == boundary_face_nb)
      {
        cons_to_char(left_face_data[face_pt]->solution,char_normal,char_sol);
#ifdef Aminzero
//        if( std::abs(char_sol[AMIN]) > 1e-10 )
//        {
//          std::stringstream ss;
//          ss<< "char_sol[AMIN] = " << char_sol[AMIN] << std::endl;
//          throw common::BadValue(FromHere(),ss.str());
//        }
        char_sol[AMIN] = 0.;
//        char_to_cons(char_sol,char_normal,left_face_data[face_pt]->solution);
#endif
        char_to_cons(char_sol,char_normal,right_face_data[face_pt]->solution);
      }

      Uint flx_pt = left_face_pt_idx[face_pt];
      compute_numerical_non_reflective_flux(*left_face_data[face_pt],*right_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt] * elem->get().sf->flx_pt_sign(flx_pt),
                                            char_normal,
                                            flux_in_flx_pts[flx_pt],flx_pt_wave_speed[flx_pt][0]);
      flux_in_flx_pts[flx_pt] *= elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
    }
    neighbour_elem->get().unlock();
  }
}

void NonReflectiveConvection2D::compute_fluxes(std::vector<RealVectorNEQS>& flux_in_flx_pts)
{
  RealVectorNEQS char_sol;
  RealVector char_normal = characteristic_normal();

  boost_foreach(Uint flx_pt, elem->get().sf->interior_flx_pts())
  {
    compute_flx_pt_phys_data(elem->get(),flx_pt,*flx_pt_data);
    compute_analytical_flux(*flx_pt_data,flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                            flux_in_flx_pts[flx_pt],flx_pt_wave_speed[flx_pt][0]);
    flux_in_flx_pts[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
  }
  /// 2) Calculate flux in face flux points
  for(m_face_nb=0; m_face_nb<elem->get().sf->nb_faces(); ++m_face_nb)
  {
    /// 2.1) Compute physical data in face
    compute_face();

    /// 2.2) Compute flux
    /// * Case 1: face is marked as outer_face --> extrapolate solution from interior and compute analytical flux
    if (face_entities->has_tag(mesh::Tags::outer_faces()))
    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
        Uint flx_pt = left_face_pt_idx[face_pt];
        compute_analytical_flux(*left_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt],
                                flux_in_flx_pts[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flux_in_flx_pts[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
    /// * Case 2: face is inner-face or boundary-face --> compute numerical flux
    else
    {
      for (Uint face_pt=0; face_pt<elem->get().sf->face_flx_pts(m_face_nb).size(); ++face_pt)
      {
        Uint flx_pt = left_face_pt_idx[face_pt];

        // Set BC, default extrapolation
        if (m_face_nb == boundary_face_nb)
        {
          cons_to_char(left_face_data[face_pt]->solution,char_normal,char_sol);
#ifdef Aminzero
//          if( std::abs(char_sol[AMIN]) > 1e-10 )
//          {
//            std::stringstream ss;
//            ss<< "char_sol[AMIN] = " << char_sol[AMIN] << std::endl;
//            throw common::BadValue(FromHere(),ss.str());
//          }
        char_sol[AMIN] = 0.;
//        char_to_cons(char_sol,char_normal,left_face_data[face_pt]->solution);
#endif
        char_to_cons(char_sol,char_normal,right_face_data[face_pt]->solution);
        }

        compute_numerical_flux(*left_face_data[face_pt],*right_face_data[face_pt],flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt] * elem->get().sf->flx_pt_sign(flx_pt),
                               flux_in_flx_pts[flx_pt],flx_pt_wave_speed[flx_pt][0]);
        flux_in_flx_pts[flx_pt] *= elem->get().sf->flx_pt_sign(flx_pt) * flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
        flx_pt_wave_speed[flx_pt] *= flx_pt_plane_jacobian_normal->get().plane_jacobian[flx_pt];
      }
    }
    neighbour_elem->get().unlock();
  }
}

void NonReflectiveConvection2D::execute()
{
  cf3_assert(boundary_face_nb>=0);

//  bool inspect=false;
//  Real y_coord = solution_field().coordinates()[elem->get().space->connectivity()[elem->get().idx][0]][YY];
//  if (y_coord >= -1 && y_coord <= 1)
//  {
//    inspect = true;
//  }

  RealVectorNDIM char_normal = characteristic_normal();
  RealVectorNDIM char_s; char_s << char_normal[YY],-char_normal[XX];
  RealVectorNEQS char_sol;
  const Uint direction = elem->get().sf->flx_pt_dirs( elem->get().sf->face_flx_pts(boundary_face_nb)[0] )[0];


  /// Strong BC
  /// ---------

//  /// Interpolate original term to flux points (of which some lie on the boundary)
//  std::vector<RealVectorNEQS> flx_pt_sol(elem->get().sf->nb_flx_pts());
//  mesh::Field::View solution = solution_field().view(elem->get().space->connectivity()[m_elem_idx]);
//  elem->get().reconstruct_from_solution_space_to_flux_points(solution,flx_pt_sol);

//  boost_foreach(const Uint flx_pt, elem->get().sf->face_flx_pts(boundary_face_nb))
//  {
//    cons_to_char(flx_pt_sol[flx_pt],char_normal,char_sol);
//#ifdef Aminzero
//    char_sol[AMIN] = 0;
//#endif
//    char_to_cons(char_sol,char_normal,flx_pt_sol[flx_pt]);
//  }
//  elem->get().reconstruct_from_flux_points_to_solution_space(direction,flx_pt_sol,solution);

//  // check
//  elem->get().reconstruct_from_solution_space_to_flux_points(solution,flx_pt_sol);
//  boost_foreach(const Uint flx_pt, elem->get().sf->face_flx_pts(boundary_face_nb))
//  {
//    cons_to_char(flx_pt_sol[flx_pt],char_normal,char_sol);
//#ifdef Aminzero
//    if (std::abs(char_sol[AMIN]) > 1e-10)
//      throw common::BadValue(FromHere(),"");
//#endif
//  }



  /// (A) Compute reflective term in flux points
  /// ------------------------------------------
  std::vector<RealVectorNEQS> cons_non_reflective_flux(elem->get().sf->nb_flx_pts());
  compute_non_reflective_fluxes(cons_non_reflective_flux);
  std::vector<RealVectorNEQS> cons_reflective_flux(elem->get().sf->nb_flx_pts());
  compute_fluxes(cons_reflective_flux);



//  // Initial values for non_reflective_deriv are the same ones as the reflective, values will be changed further
//  std::vector< std::vector<RealVectorNEQS> > cons_reflective_deriv(NDIM,std::vector<RealVectorNEQS>(elem->get().sf->nb_flx_pts()));
//  std::vector< std::vector<RealVectorNEQS> > cons_non_reflective_deriv(NDIM,std::vector<RealVectorNEQS>(elem->get().sf->nb_flx_pts()));

//  for (Uint flx_pt=0; flx_pt<elem->get().sf->nb_flx_pts(); ++flx_pt)
//  {
//    for (Uint d=0; d<NDIM; ++d)
//    {
//      DerivativeReconstructFromFluxPoint compute_flux_derivative;
//      compute_flux_derivative.build_coefficients(d,elem->get().sf->flx_pts().row(flx_pt),elem->get().sf);
//      compute_flux_derivative(cons_reflective_flux,cons_non_reflective_deriv[d][flx_pt]);
//      compute_flux_derivative(cons_reflective_flux,cons_reflective_deriv[d][flx_pt]);
//      cons_reflective_deriv[d][flx_pt].setZero();
//    }
//  }

//  /// (B) For every face point, compute high-order non-reflective term in flux points
//  /// --------------------------------------------------------------------
//  m_face_nb = boundary_face_nb;
//  compute_face();
//  neighbour_elem->get().unlock();

//  // Loop over face flux points
//  boost_foreach(Uint face_flx_pt, elem->get().sf->face_flx_pts(m_face_nb))
////  for (Uint face_flx_pt=0; face_flx_pt<elem->get().sf->nb_flx_pts(); ++face_flx_pt)
//  {
//    // - Get characteristic normal
//    const RealVectorNDIM& char_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[face_flx_pt];
//    Uint flx_dir = elem->get().sf->flx_pt_dirs(face_flx_pt)[0];
//    // - Compute non-reflective fluxes in flux points
//    std::vector<RealVectorNEQS> cons_non_reflective_flux(elem->get().sf->nb_flx_pts());
//    std::vector<RealVectorNEQS> bc_non_reflective_flux(elem->get().sf->nb_flx_pts());

//    compute_non_reflective_fluxes(flx_dir, char_normal,cons_non_reflective_flux);
////    compute_reflective_fluxes(cons_non_reflective_flux);

//    bool computed=false;
//    for (Uint face=0; face<elem->get().sf->nb_faces(); ++face)
//    {
//      boost_foreach(Uint flx_pt, elem->get().sf->face_flx_pts(face))
//      {
//        if (flx_pt == face_flx_pt)
//        {
//          if (face == boundary_face_nb) {
//            compute_non_reflective_fluxes(flx_dir,char_normal, cons_non_reflective_flux);
//            computed = true;
//            break;
//          } else {
//            compute_non_reflective_fluxes(flx_dir,char_normal,cons_non_reflective_flux);
////            compute_reflective_fluxes(cons_non_reflective_flux);
//            computed = true;
//            break;
//          }
//        }
//      }
//    }
//    if (!computed) { // inner flux points
//      compute_non_reflective_fluxes(flx_dir,char_normal,cons_non_reflective_flux);
////      compute_reflective_fluxes(cons_non_reflective_flux);
//    }

//    // - Transform fluxes to characteristic variables
//    for (Uint flx_pt=0; flx_pt<elem->get().sf->nb_flx_pts(); ++flx_pt)
//    {
//      cons_to_bc(cons_reflective_flux[flx_pt],char_normal,bc_reflective_flux[flx_pt]);
//      cons_to_bc(cons_non_reflective_flux[flx_pt],char_normal,bc_non_reflective_flux[flx_pt]);
//    }

//    // - Compute derivative in face flux point in characteristic variables
//    Uint d = flx_dir;
//    DerivativeReconstructFromFluxPoint compute_flux_derivative;
//    compute_flux_derivative.build_coefficients(d,elem->get().sf->flx_pts().row(face_flx_pt),elem->get().sf);
//    RealVectorNEQS bc_reflective_deriv;
//    RealVectorNEQS bc_non_reflective_deriv;
//    compute_flux_derivative(bc_reflective_flux,bc_reflective_deriv);
//    compute_flux_derivative(bc_non_reflective_flux,bc_non_reflective_deriv);

//    // - Correct the first 2 non-reflective equations with the usual ones
//    bc_non_reflective_deriv[0] = bc_reflective_deriv[0];  // dSdx     copied from original
//    bc_non_reflective_deriv[1] = bc_reflective_deriv[1];  // dOmegadx copied from original
////    bc_non_reflective_deriv[2] = bc_reflective_deriv[2];  // dAdx copied from original
////    bc_non_reflective_deriv[3] = bc_reflective_deriv[3];  // domegadx copied from original

//    // - Copy flux derivative now in correct place
//    bc_to_cons(bc_non_reflective_deriv,char_normal,cons_non_reflective_deriv[d][face_flx_pt]);
//  }

  /// (C) Compute term
  /// ----------------

//  mesh::Field::View term = m_term_field->view(elem->get().space->connectivity()[m_elem_idx]);

//  std::vector<RealVectorNEQS> reflective_term(elem->get().sf->nb_sol_pts());
//  for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts();++sol_pt)
//  {
//    ReconstructFromFluxPoint::set_zero(term[sol_pt]);
//    ReconstructFromFluxPoint::set_zero(reflective_term[sol_pt]);
//    for (Uint d=0; d<NDIM; ++d)
//    {
//      ReconstructFromFluxPoint reconstruct_from_flx_pt_to_sol_pt;
//      reconstruct_from_flx_pt_to_sol_pt.build_coefficients(d,elem->get().sf->sol_pts().row(sol_pt),elem->get().sf);
//      reconstruct_from_flx_pt_to_sol_pt.add(cons_non_reflective_deriv[d],term[sol_pt]);
//      reconstruct_from_flx_pt_to_sol_pt.add(cons_reflective_deriv[d],reflective_term[sol_pt]);
//    }
//  }


  /// We now have the original fluxes with extrapolation BC, and a "WRONG" non reflective fluxes, of which we only want to
  /// keep the Amin characteristic variable flux, and add it to the boundary solution points.

  /// Compute divergence of the original fluxes in all solution points
  std::vector<RealVectorNEQS> term(elem->get().sf->nb_sol_pts());
  elem->get().reconstruct_divergence_from_flux_points_to_solution_space(cons_reflective_flux,term);

  /// Compute divergence of the modified fluxes in all solution points
  std::vector<RealVectorNEQS> non_reflective_term(elem->get().sf->nb_sol_pts());
  elem->get().reconstruct_divergence_from_flux_points_to_solution_space(cons_non_reflective_flux,non_reflective_term);

  /// Transform fluxes to physical space
  mesh::Field::View jacob_det = jacob_det_field().view(elem->get().space->connectivity()[m_elem_idx]);
  for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (Uint v=0; v<NEQS; ++v) {
      non_reflective_term[sol_pt][v] /= jacob_det[sol_pt][0];
      term[sol_pt][v] /= jacob_det[sol_pt][0];
    }
  }

#ifndef NOTHING

  /// Interpolate original term to flux points (of which some lie on the boundary)
  std::vector<RealVectorNEQS> flx_pt_term(elem->get().sf->nb_flx_pts());
  elem->get().reconstruct_from_solution_space_to_flux_points(term,flx_pt_term);

  /// Interpolate modified term to flux points (of which some lie on the boundary)
  std::vector<RealVectorNEQS> flx_pt_non_reflective_term(elem->get().sf->nb_flx_pts());
  elem->get().reconstruct_from_solution_space_to_flux_points(non_reflective_term,flx_pt_non_reflective_term);

  /// In the boundary face points, modify the equation of Aminus
  RealVectorNEQS char_term;
  RealVectorNEQS char_non_reflective_term;
  boost_foreach(Uint flx_pt, elem->get().sf->face_flx_pts(boundary_face_nb))
  {
    cons_to_char(flx_pt_term[flx_pt],char_normal,char_term);
    cons_to_char(flx_pt_non_reflective_term[flx_pt],char_normal,char_non_reflective_term);

    char_term[AMIN]=char_non_reflective_term[AMIN];
#ifdef LILLA
    char_term[APLUS]=char_non_reflective_term[APLUS];
#endif
    char_to_cons(char_term,char_normal,flx_pt_term[flx_pt]);
  }

  /// Interpolate now back to the solution points, which may not lie on the boundary
  elem->get().reconstruct_from_flux_points_to_solution_space(direction,flx_pt_term,term);

#endif // ifndef NOTHING

  /// 4) Subtract this term from the residual field
  mesh::Field::View residual = residual_field().view(elem->get().space->connectivity()[m_elem_idx]);
  for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
  {
    for (Uint v=0; v<NEQS; ++v)
    {
      residual[sol_pt][v] = - term[sol_pt][v];
    }
  }


//  if (inspect)
//  {
//    if ( solver().handle<SDSolver>()->time_stepping().time().current_time() == 0)
//    {
//      std::vector<RealVectorNEQS> cons_flux(elem->get().sf->nb_sol_pts());
//      Uint d=XX;
//      elem->get().reconstruct_from_flux_points_to_solution_space(d,cons_non_reflective_flux,cons_flux);

//      RealVectorNEQS sol;
//      for (Uint v=0; v<NEQS; ++v)
//      {
//        std::cout << "sol["<<v<<"] = \n";
//        Uint count=0;
//        for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          ++count;
//          RealVectorNEQS bc_sol;

//          for (Uint v=0; v<NEQS; ++v)
//          {
//            sol[v]=solution_field().view(elem->get().space->connectivity()[m_elem_idx])[sol_pt][v];
//          }

//          cons_to_bc(sol,char_normal,bc_sol);
//          std::cout << bc_sol[v];
//          if (count < 4) std::cout << "\t";
//          else {std::cout << std::endl; count=0;}
//        }
//      }


//      RealVectorNEQS bc_flux;
//      for (Uint v=0; v<NEQS; ++v)
//      {
//        std::cout << "flux_X["<<v<<"] = \n";
//        Uint count=0;
//        for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          ++count;
//          cons_to_bc(cons_flux[sol_pt],char_normal,bc_flux);
//          std::cout << bc_flux[v];
//          if (count < 4) std::cout << "\t";
//          else {std::cout << std::endl; count=0;}
//        }
//      }

//      d=YY;
//      elem->get().reconstruct_from_flux_points_to_solution_space(d,cons_non_reflective_flux,cons_flux);

//      for (Uint v=0; v<NEQS; ++v)
//      {
//        std::cout << "flux_Y["<<v<<"] = \n";
//        Uint count=0;
//        for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          ++count;
//          cons_to_bc(cons_flux[sol_pt],char_normal,bc_flux);
//          std::cout << bc_flux[v];
//          if (count < 4) std::cout << "\t";
//          else {std::cout << std::endl; count=0;}
//        }
//      }

//      RealVectorNEQS res;
//      for (Uint v=0; v<NEQS; ++v)
//      {
//        std::cout << "res["<<v<<"] = \n";
//        Uint count=0;
//        for (Uint sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt)
//        {
//          ++count;

//          RealVectorNEQS cons_res;

//          for (Uint v=0; v<NEQS; ++v)
//          {
//            cons_res[v]=residual[sol_pt][v];
//          }

//          cons_to_bc(cons_res,char_normal,res);

//          std::cout << res[v];
//          if (count < 4) std::cout << "\t";
//          else {std::cout << std::endl; count=0;}
//        }

//      }
//    }
//  }

  // This is just to make sure we check at beginning of this function, it was set to something valid
  boundary_face_nb=-1;
}

/////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3
