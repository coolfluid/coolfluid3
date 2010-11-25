// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/tuple/tuple.hpp>

#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/CLink.hpp"


#include "Mesh/CHoneycombInterpolator.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CFieldElements.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < Mesh::CHoneycombInterpolator,
                               Mesh::CInterpolator,
                               LibMesh >
aHoneyCombInterpolator_Builder ( "Honeycomb" );

//////////////////////////////////////////////////////////////////////////////

CHoneycombInterpolator::CHoneycombInterpolator( const std::string& name )
  : CInterpolator(name), m_dim(0), m_ranges(3), m_N(3), m_D(3), m_comb_idx(3), m_sufficient_nb_points(0)
{
  BuildComponent<full>().build(this);
}

/////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::define_config_properties ( CF::Common::PropertyList& options )
{
  m_properties.add_option< OptionT<Uint> >
  ( "ApproximateNbElementsPerCell",
    "The approximate amount of elements that are stored in a structured" ,
    1 );

  std::vector<Uint> dummy;
  m_properties.add_option< OptionArrayT<Uint> >
  ( "Divisions",
    "The number of divisions in each direction of the comb. Takes precedence over \"ApproximateNbElementsPerCell\". " ,
    dummy);
}

//////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::construct_internal_storage(const CMesh::Ptr& source)
{
  if (m_source_mesh != source)
  {
    m_source_mesh = source;
    create_honeycomb();
  }
}
	
//////////////////////////////////////////////////////////////////////
	
void CHoneycombInterpolator::interpolate_field_from_to(const CField& source, CField& target)
{  
	if (source.basis() == CField::NODE_BASED && target.basis() == CField::NODE_BASED)
	{
		// Loop over all target data
		BOOST_FOREACH(CArray& t_data, recursive_filtered_range_typed<CArray>(target,IsComponentTag("field_data")))
		{
			// get the target coordinates table
			const CArray& t_coords = *t_data.get_child_type<CLink>("coordinates")->get_type<CArray>();
			
			// Allocations
			CElements::ConstPtr s_geom_elements;
			Uint s_elm_idx;
			Uint t_coords_dim = t_coords.row_size();
			RealVector t_node(m_dim);
      t_node.setZero();
			
			for (Uint t_node_idx=0; t_node_idx<t_data.size(); ++t_node_idx)
			{
				// find the element in the source
				for (Uint d=0; d<t_coords_dim; ++d)
					t_node[d] = t_coords[t_node_idx][d];

				boost::tie(s_geom_elements,s_elm_idx) = find_element(t_node);
				
				if (s_geom_elements)
				{
					// look for the source_field elements
					const CFieldElements& s_field_elements = s_geom_elements->get_field_elements(source.field_name());
					
					// extract the single source element of interest
					CTable::ConstRow s_elm = s_field_elements.connectivity_table()[s_elm_idx];
					
					// get the coordinates of the nodes of the source element
					std::vector<RealVector> s_nodes(s_elm.size(), RealVector(s_field_elements.coordinates().row_size()));
					fill( s_nodes , s_field_elements.coordinates() , s_elm );
					
					// get the source data
					const CArray& s_data = s_field_elements.data();
					
					// interpolate the source data to the target data
					// CFinfo << "interpolating with " << s_nodes.size() << " source nodes" << CFendl;
					std::vector<Real> w(s_nodes.size());
					pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_node, w);
					
					for (Uint idata=0; idata<t_data.row_size(); ++idata)
						t_data[t_node_idx][idata] = 0;
					
					for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
						for (Uint idata=0; idata<t_data.row_size(); ++idata)
							t_data[t_node_idx][idata] += w[s_node_idx]*s_data[s_elm[s_node_idx]][idata];

					
				}
			}
		}
	}
	else if (source.basis() == CField::ELEMENT_BASED && target.basis() == CField::NODE_BASED)
	{
		// Loop over all target data
		BOOST_FOREACH(CArray& t_data, recursive_filtered_range_typed<CArray>(target,IsComponentTag("field_data")))
		{
			// get the target coordinates table
			const CArray& t_coords = *t_data.get_child_type<CLink>("coordinates")->get_type<CArray>();
			
			// Allocations
			Uint t_coords_dim = t_coords.row_size();
			RealVector t_node(m_dim);
      t_node.setZero();
			
			for (Uint t_node_idx=0; t_node_idx<t_data.size(); ++t_node_idx)
			{
				// find the element in the source
				for (Uint d=0; d<t_coords_dim; ++d)
					t_node[d] = t_coords[t_node_idx][d];
				
				if (find_comb_idx(t_node))
				{
					find_pointcloud(m_sufficient_nb_points);
					std::vector<RealVector> s_centroids;
					std::vector<boost::tuple<CArray::ConstPtr,Uint> > s_data;
					RealVector s_centroid(m_dim);
          s_centroid.setZero();
					BOOST_FOREACH(const Point* point, m_pointcloud)
					{
						const ElementType& etype = point->first->element_type();
						Uint nb_nodes_per_element = etype.nb_nodes();
						const CTable& connectivity_table = point->first->connectivity_table();
						const CArray& coordinates_table  = point->first->coordinates();
						CTable::ConstRow element = connectivity_table[point->second];
						std::vector<RealVector> s_nodes(element.size(), RealVector(coordinates_table.row_size()));
						fill( s_nodes , coordinates_table , element );
						s_centroid.setZero();
						BOOST_FOREACH(RealVector& s_node, s_nodes)
						{
							for (int d=0; d<s_node.size(); ++d)
								s_centroid[d] += s_node[d]/nb_nodes_per_element;
						}
						s_centroids.push_back(s_centroid);
						CArray::ConstPtr field_data = point->first->get_field_elements(source.field_name()).data().get_type<CArray const>();
						s_data.push_back(boost::make_tuple(field_data,point->second));
					}	
					
					std::vector<Real> w(s_centroids.size());
					pseudo_laplacian_weighted_linear_interpolation(s_centroids, t_node, w);
					
					for (Uint idata=0; idata<t_data.row_size(); ++idata)
						t_data[t_node_idx][idata] = 0;
					
					for (Uint s_elm_idx=0; s_elm_idx<s_centroids.size(); ++s_elm_idx)
						for (Uint idata=0; idata<t_data.row_size(); ++idata)
							t_data[t_node_idx][idata] += w[s_elm_idx]*s_data[s_elm_idx].get<0>()->array()[s_data[s_elm_idx].get<1>()][idata];
				}
			}
		}
	}
	else if (source.basis() == CField::NODE_BASED && target.basis() == CField::ELEMENT_BASED)
	{
		BOOST_FOREACH(CFieldElements& t_elems, recursive_range_typed<CFieldElements>(target))
		{
			RealVector t_centroid(m_dim);
      t_centroid.setZero();
			CTable& connectivity_table = t_elems.connectivity_table();
			CArray& coordinates = t_elems.coordinates();
			CArray& t_data = t_elems.data();
			Uint nb_nodes_per_element = t_elems.element_type().nb_nodes();
			CElements::ConstPtr s_geom_elements;
			Uint s_elm_idx;
			for (Uint t_elm_idx=0; t_elm_idx<connectivity_table.size(); ++t_elm_idx)
			{
				
				// Find the coordinates of the target element centroid
				CTable::ConstRow t_elm = connectivity_table[t_elm_idx];
				
				std::vector<RealVector> t_nodes(t_elm.size(), RealVector(coordinates.row_size()));
				fill( t_nodes , coordinates , t_elm );
				
				t_centroid.setZero();
				BOOST_FOREACH(RealVector& t_node, t_nodes)
				{
					for (int d=0; d<t_node.size(); ++d)
						t_centroid[d] += t_node[d]/nb_nodes_per_element;
				}				
				
				boost::tie(s_geom_elements,s_elm_idx) = find_element(t_centroid);
				
				if (s_geom_elements)
				{
					// look for the source_field elements
					const CFieldElements& s_field_elements = s_geom_elements->get_field_elements(source.field_name());
					
					// extract the single source element of interest
					CTable::ConstRow s_elm = s_field_elements.connectivity_table()[s_elm_idx];
					
					// get the coordinates of the nodes of the source element
					std::vector<RealVector> s_nodes(s_elm.size(), RealVector(s_field_elements.coordinates().row_size()));
					fill( s_nodes , s_field_elements.coordinates() , s_elm );
					
					// get the source data
					const CArray& s_data = s_field_elements.data();
					
					std::vector<Real> w(s_nodes.size());
					pseudo_laplacian_weighted_linear_interpolation(s_nodes, t_centroid, w);
					
					for (Uint idata=0; idata<t_data.row_size(); ++idata)
						t_data[t_elm_idx][idata] = 0;
					
					for (Uint s_node_idx=0; s_node_idx<s_nodes.size(); ++s_node_idx)
						for (Uint idata=0; idata<t_data.row_size(); ++idata)
							t_data[t_elm_idx][idata] += w[s_node_idx]*s_data[s_elm[s_node_idx]][idata];
					
				}					
			}
		}
	}
	else if (source.basis() == CField::ELEMENT_BASED && target.basis() == CField::ELEMENT_BASED)
	{
		BOOST_FOREACH(CFieldElements& t_elems, recursive_range_typed<CFieldElements>(target))
		{
			RealVector t_centroid(m_dim);
      t_centroid.setZero();
			CTable& connectivity_table = t_elems.connectivity_table();
			CArray& coordinates = t_elems.coordinates();
			CArray& t_data = t_elems.data();
			Uint nb_nodes_per_element = t_elems.element_type().nb_nodes();

			for (Uint t_elm_idx=0; t_elm_idx<connectivity_table.size(); ++t_elm_idx)
			{
				
				// Find the coordinates of the target element centroid
				CTable::ConstRow t_elm = connectivity_table[t_elm_idx];
				
				std::vector<RealVector> t_nodes(t_elm.size(), RealVector(coordinates.row_size()));
				fill( t_nodes , coordinates , t_elm );
				
				t_centroid.setZero();
				BOOST_FOREACH(RealVector& t_node, t_nodes)
				{
					for (int d=0; d<t_node.size(); ++d)
						t_centroid[d] += t_node[d]/nb_nodes_per_element;
				}				
				
				if (find_comb_idx(t_centroid))
				{
					find_pointcloud(m_sufficient_nb_points);
					std::vector<RealVector> s_centroids;
					std::vector<boost::tuple<CArray::ConstPtr,Uint> > s_data;
					RealVector s_centroid(m_dim);
          s_centroid.setZero();
					BOOST_FOREACH(const Point* point, m_pointcloud)
					{
						const ElementType& etype = point->first->element_type();
						Uint nb_nodes_per_element = etype.nb_nodes();
						const CTable& connectivity_table = point->first->connectivity_table();
						const CArray& coordinates_table  = point->first->coordinates();
						CTable::ConstRow element = connectivity_table[point->second];
						std::vector<RealVector> s_nodes(element.size(), RealVector(coordinates_table.row_size()));
						fill( s_nodes , coordinates_table , element );
						s_centroid.setZero();
						BOOST_FOREACH(RealVector& s_node, s_nodes)
						{
							for (int d=0; d<s_node.size(); ++d)
								s_centroid[d] += s_node[d]/nb_nodes_per_element;
						}
						s_centroids.push_back(s_centroid);
						CArray::ConstPtr field_data = point->first->get_field_elements(source.field_name()).data().get_type<CArray const>();
						s_data.push_back(boost::make_tuple(field_data,point->second));
					}	
					
					std::vector<Real> w(s_centroids.size());
					pseudo_laplacian_weighted_linear_interpolation(s_centroids, t_centroid, w);
					
					for (Uint idata=0; idata<t_data.row_size(); ++idata)
						t_data[t_elm_idx][idata] = 0;
					
					for (Uint s_elm_idx=0; s_elm_idx<s_centroids.size(); ++s_elm_idx)
						for (Uint idata=0; idata<t_data.row_size(); ++idata)
							t_data[t_elm_idx][idata] += w[s_elm_idx]*s_data[s_elm_idx].get<0>()->array()[s_data[s_elm_idx].get<1>()][idata];
					
					
				}
			}
		}
	}
	else
	{
		throw ShouldNotBeHere(FromHere(), "CField::basis() should return NODE_BASED or ELEMENT_BASED");
	}
}

//////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::create_honeycomb()
{
  m_dim=0;
  std::set<const CArray*> all_coordinates;
  BOOST_FOREACH(CElements& elements, recursive_range_typed<CElements>(*m_source_mesh))
  {
    m_dim = std::max(elements.element_type().dimensionality() , m_dim);
    all_coordinates.insert(&elements.coordinates());
  }

  std::vector<Real> L(3);
  for (Uint d=0; d<m_dim; ++d)
  {
    m_ranges[d].resize(2,0.0);
  }

  Real V=1;
  BOOST_FOREACH(const CArray* coordinates , all_coordinates)
    BOOST_FOREACH(const CArray::ConstRow& node, coordinates->array())
      for (Uint d=0; d<m_dim; ++d)
      {
        m_ranges[d][0] = std::min(m_ranges[d][0],  node[d]);
        m_ranges[d][1] = std::max(m_ranges[d][1],  node[d]);
      }
  for (Uint d=0; d<m_dim; ++d)
  {
    L[d] = m_ranges[d][1] - m_ranges[d][0];
    V*=L[d];
  }

  m_nb_elems = get_component_typed<CRegion>(*m_source_mesh).recursive_filtered_elements_count(IsElementsVolume());


  if (property("Divisions").value<std::vector<Uint> >().size() > 0)
  {
    m_N = property("Divisions").value<std::vector<Uint> >();
    for (Uint d=0; d<m_dim; ++d)
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
  }
  else
  {
    Real V1 = V/m_nb_elems;
    Real D1 = std::pow(V1,1./m_dim)*property("ApproximateNbElementsPerCell").value<Uint>();

    for (Uint d=0; d<m_dim; ++d)
    {
      m_N[d] = (Uint) std::ceil(L[d]/D1);
      m_D[d] = (L[d])/static_cast<Real>(m_N[d]);
    }
  }


  CFinfo << "Honeycomb:" << CFendl;
	CFinfo << "----------" << CFendl;
  for (Uint d=0; d<m_dim; ++d)
  {
    CFinfo << "range["<<d<<"] :   L = " << L[d] << "    N = " << m_N[d] << "    D = " << m_D[d] << CFendl;
  }
  CFinfo << "V = " << V << CFendl;

  // initialize the honeycomb
  m_honeycomb.resize(boost::extents[std::max(Uint(1),m_N[0])][std::max(Uint(1),m_N[1])][std::max(Uint(1),m_N[2])]);


  Uint total_nb_elems=0;
  BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*m_source_mesh,IsElementsVolume()))
  {
    const CArray& coordinates = elements.coordinates();
    Uint nb_nodes_per_element = elements.connectivity_table().row_size();
    Uint elem_idx=0;
    BOOST_FOREACH(const CTable::ConstRow& elem, elements.connectivity_table().array())
    {
      RealVector centroid(m_dim);
      BOOST_FOREACH(const Uint node_idx, elem)
        centroid += to_vector(coordinates[node_idx]);

      centroid /= static_cast<Real>(nb_nodes_per_element);
      for (Uint d=0; d<m_dim; ++d)
        m_comb_idx[d]=std::min((Uint) std::floor( (centroid[d] - m_ranges[d][0])/m_D[d]), m_N[d]-1 );
      m_honeycomb[m_comb_idx[0]][m_comb_idx[1]][m_comb_idx[2]].push_back(std::make_pair<const CElements*,Uint>(&elements,elem_idx));
      elem_idx++;
    }
    total_nb_elems += elem_idx;
  }


  Uint total=0;

  switch (m_dim)
  {
    case DIM_2D:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
        {
          Uint k=0;
          CFinfo << "("<<i<<","<<j<<") has " << m_honeycomb[i][j][k].size() << " elems" << CFendl;
          total += m_honeycomb[i][j][k].size();
        }
      break;
    case DIM_3D:
      for (Uint i=0; i<m_N[0]; ++i)
        for (Uint j=0; j<m_N[1]; ++j)
          for (Uint k=0; k<m_N[2]; ++k)
          {
            CFinfo << "("<<i<<","<<j<<","<<k<<") has " << m_honeycomb[i][j][k].size() << " elems" << CFendl;
            total += m_honeycomb[i][j][k].size();
          }
      break;
    default:
      break;
  }

  CFinfo << "total = " << total << " of " << m_nb_elems << CFendl;

  m_sufficient_nb_points = static_cast<Uint>(std::pow(3.,(int)m_dim));

}


//////////////////////////////////////////////////////////////////////////////
  
bool CHoneycombInterpolator::find_comb_idx(const RealVector& coordinate)
{
  //CFinfo << "point " << coordinate << CFflush;
  cf_assert(coordinate.size() == static_cast<int>(m_dim));

  for (Uint d=0; d<m_dim; ++d)
	{
		if ( (coordinate[d] - m_ranges[d][0])/m_D[d] > m_N[d])
		{
			//CFinfo << "coord["<<d<<"] = " << coordinate[d] <<" is not inside bounding box" << CFendl;
			//CFinfo << (coordinate[d] - m_ranges[d][0])/m_D[d] << " > " << m_N[d] << CFendl;
			return false; // no index found
		}
    m_comb_idx[d] = std::min((Uint) std::floor( (coordinate[d] - m_ranges[d][0])/m_D[d]), m_N[d]-1 );
	}
		  
  //CFinfo << " should be in box ("<<m_comb_idx[0]<<","<<m_comb_idx[1]<<","<<m_comb_idx[2]<<")" << CFendl;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

void CHoneycombInterpolator::find_pointcloud(Uint nb_points)
{
  m_pointcloud.resize(0);
  Uint r=1;
  int i(0), j(0), k(0);
  int imin, jmin, kmin;
  int imax, jmax, kmax;
  
  if (m_honeycomb[m_comb_idx[0]][m_comb_idx[1]][m_comb_idx[2]].size() <= nb_points)
    r=0;
  
  while (m_pointcloud.size() < nb_points && m_pointcloud.size() < m_nb_elems)
  {
    ++r;
    m_pointcloud.resize(0);

    switch (m_dim)
    {
      case DIM_3D:
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_comb_idx[1])-int(r), 0);  jmax = std::min(int(m_comb_idx[1])+int(r),int(m_N[1])-1);
        kmin = std::max(int(m_comb_idx[2])-int(r), 0);  kmax = std::min(int(m_comb_idx[2])+int(r),int(m_N[2])-1);
        for (i = imin; i <= imax; ++i)
          for (j = jmin; j <= jmax; ++j)
            for (k = kmin; k <= kmax; ++k)
            {
              BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
              m_pointcloud.push_back(&point);
              //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
            }
        break;
      case DIM_2D:
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
        jmin = std::max(int(m_comb_idx[1])-int(r), 0);  jmax = std::min(int(m_comb_idx[1])+int(r),int(m_N[1])-1);
        for (i = imin; i <= imax; ++i)
          for (j = jmin; j <= jmax; ++j)
          {
            BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
            m_pointcloud.push_back(&point);
            //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
          }
        break;
      case DIM_1D:
        imin = std::max(int(m_comb_idx[0])-int(r), 0);  imax = std::min(int(m_comb_idx[0])+int(r),int(m_N[0])-1);
        for (i = imin; i <= imax; ++i)
        {
          BOOST_FOREACH(const Point& point, m_honeycomb[i][j][k])
          m_pointcloud.push_back(&point);
          //CFinfo << "   ("<<i<<","<<j<<","<<k<<")" <<  CFendl;
        }
        break;
    }
  }
    
  //CFinfo << m_pointcloud.size() << " points in the pointcloud " << CFendl;
 
}
	
//////////////////////////////////////////////////////////////////////////////

boost::tuple<CElements::ConstPtr,Uint> CHoneycombInterpolator::find_element(const RealVector& target_coord)
{
	if (find_comb_idx(target_coord))
	{
		find_pointcloud(1);
		
		BOOST_FOREACH(const Point* point, m_pointcloud)
		{
			const ElementType& etype = point->first->element_type();
			const CTable& connectivity_table = point->first->connectivity_table();
			const CArray& coordinates_table = point->first->coordinates();
			CTable::ConstRow element = connectivity_table[point->second];
      ElementType::NodesT nodes(etype.nb_nodes(), etype.dimension());
			fill(nodes, coordinates_table , element);
			if (etype.is_coord_in_element(target_coord,nodes))
			{
				//CFinfo << "found target in element" << point->first->full_path().string() << " [" << point->second << "]" <<CFendl;
				return boost::make_tuple(point->first->get_type<CElements const>(),point->second);
			}
		}
	}
	return boost::make_tuple(CElements::ConstPtr(),(Uint)0);
}
	
//////////////////////////////////////////////////////////////////////
	
void CHoneycombInterpolator::pseudo_laplacian_weighted_linear_interpolation(const std::vector<RealVector>& s_points, const RealVector& t_point, std::vector<Real>& weights)
{
	switch (m_dim)
	{
		case DIM_3D:
		{
			Real Ixx(0),Ixy(0),Ixz(0),Iyy(0),Iyz(0),Izz(0), Rx(0),Ry(0),Rz(0), Lx,Ly,Lz, dx,dy,dz;
			RealVector Dx(s_points.size());
			RealVector Dy(s_points.size());
			RealVector Dz(s_points.size());
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				dx = s_points[s_pt_idx][XX] - t_point[XX];
				dy = s_points[s_pt_idx][YY] - t_point[YY];
				dz = s_points[s_pt_idx][ZZ] - t_point[ZZ];
				
				Ixx += dx*dx;
				Ixy += dx*dy;
				Ixz += dx*dz;
				Iyy += dy*dy;
				Iyz += dy*dz;
				Izz += dz*dz;
				
				Rx += dx;
				Ry += dy;
				Rz += dz;
				
				Dx[s_pt_idx]=dx;
				Dy[s_pt_idx]=dy;
				Dz[s_pt_idx]=dz;
			}
			Lx =  (-(Iyz*Iyz*Rx) + Iyy*Izz*Rx + Ixz*Iyz*Ry - Ixy*Izz*Ry - Ixz*Iyy*Rz + Ixy*Iyz*Rz)/
						(Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));
			Ly =  (Ixz*Iyz*Rx - Ixy*Izz*Rx - Ixz*Ixz*Ry + Ixx*Izz*Ry + Ixy*Ixz*Rz - Ixx*Iyz*Rz)/
						(Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixx*Iyz*Iyz + Ixy*Ixy*Izz - Ixx*Iyy*Izz);
			Lz =  (-(Ixz*Iyy*Rx) + Ixy*Iyz*Rx + Ixy*Ixz*Ry - Ixx*Iyz*Ry - Ixy*Ixy*Rz + Ixx*Iyy*Rz)/
						(Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));
			
			Real S(0);
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx] + Ly*Dy[s_pt_idx] + Lz*Dz[s_pt_idx];
				S += weights[s_pt_idx];
			}
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
				weights[s_pt_idx] /= S;
			return;
		}
		case DIM_2D:
		{
			Real Ixx(0),Ixy(0),Iyy(0), Rx(0),Ry(0), Lx,Ly, dx,dy;
			RealVector Dx(s_points.size());
			RealVector Dy(s_points.size());
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				dx = s_points[s_pt_idx][XX] - t_point[XX];
				dy = s_points[s_pt_idx][YY] - t_point[YY];
				
				Ixx += dx*dx;
				Ixy += dx*dy;
				Iyy += dy*dy;
				
				Rx += dx;
				Ry += dy;
				
				Dx[s_pt_idx]=dx;
				Dy[s_pt_idx]=dy;
			}
			Lx =  (Ixy*Ry - Iyy*Rx)/(Ixx*Iyy-Ixy*Ixy);
			Ly =  (Ixy*Rx - Ixx*Ry)/(Ixx*Iyy-Ixy*Ixy);
			
			Real S(0);
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx] + Ly*Dy[s_pt_idx] ;
				S += weights[s_pt_idx];
			}
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
				weights[s_pt_idx] /= S;
			return;
		}
		case DIM_1D:
		{
			Real Ixx(0), Rx(0), Lx, dx;
			RealVector Dx(s_points.size());
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				dx = s_points[s_pt_idx][XX] - t_point[XX];
				
				Ixx += dx*dx;
				
				Rx += dx;
				
				Dx[s_pt_idx]=dx;
			}
			Lx =  Rx/Ixx;
			
			Real S(0);
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
			{
				weights[s_pt_idx] = 1.0 + Lx*Dx[s_pt_idx];
				S += weights[s_pt_idx];
			}
			for (Uint s_pt_idx=0; s_pt_idx<s_points.size(); ++s_pt_idx)
				weights[s_pt_idx] /= S;
			return;
		}
		default:
			throw ShouldNotBeHere(FromHere(), "");
			break;
	}	
	
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
