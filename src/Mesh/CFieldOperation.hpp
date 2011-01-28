// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFieldOperation_hpp
#define CF_Mesh_CFieldOperation_hpp

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ConnectivityData.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

///////////////////////////////////////////////////////////////////////////////////////

/// @todo missing documentation
class Mesh_API CFieldOperation : public Common::Component
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CFieldOperation> Ptr;
  typedef boost::shared_ptr<CFieldOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CFieldOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CFieldOperation() {};

  /// Get the class name
  static std::string type_name () { return "CFieldOperation"; }

  virtual void set_loophelper (CElements& geometry_elements );

  virtual void set_loophelper (CTable<Real>& coordinates );

  virtual void execute ( Uint index = 0 );

  /// Templated version for high efficiency
  template < typename EType > void executeT ( Uint index = 0 );

  virtual CFieldOperation& operation();

  /// Only for use in non-templatized version
  CFieldOperation& create_operation(const std::string operation_type);

private: // data

  Uint m_counter;

};

///////////////////////////////////////////////////////////////////////////////////////

template < typename OP1, typename OP2 >
class CFieldOperationMergeT : public CFieldOperation
{
public: // typedefs

  typedef boost::shared_ptr<CFieldOperationMergeT> Ptr;
  typedef boost::shared_ptr<CFieldOperationMergeT const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CFieldOperationMergeT ( const std::string& name ) :
    CFieldOperation(name),
    m_op1(Common::allocate_component<OP1>("operation_1") ),
    m_op2(Common::allocate_component<OP2>("operation_2") )
  {
    add_static_component(m_op1);
    add_static_component(m_op2);
  }

  /// Virtual destructor
  virtual ~CFieldOperationMergeT() {};

  /// Get the class name
  static std::string type_name () { return "CFieldOperationMergeT"; }

  void set_loophelper (CElements& geometry_elements )
  {
    m_op1->set_loophelper(geometry_elements);
    m_op2->set_loophelper(geometry_elements);
  }

  const OP1& operation1() const {  return *m_op1; }

  OP1& operation1() { return *m_op1; }

  const OP2& operation2() const { return *m_op2; }

  OP2& operation2() {  return *m_op2; }

  template < typename EType >
  void executeT (  Uint elem )
  {
    m_op1->template executeT<EType>( elem );
    m_op2->template executeT<EType>( elem );
  }

  virtual void execute ( Uint index )
  {
    m_op1->execute( index );
    m_op2->execute( index );
  }

private: // data

  typename OP1::Ptr m_op1;
  typename OP2::Ptr m_op2;
};

typedef CFieldOperationMergeT<CFieldOperation,CFieldOperation> CFieldOperationMerge;

///////////////////////////////////////////////////////////////////////////////////////

class Mesh_API COutputField : public CFieldOperation
{
public: // typedefs

  typedef boost::shared_ptr<COutputField> Ptr;
  typedef boost::shared_ptr<COutputField const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  COutputField ( const std::string& name ) : CFieldOperation(name)
  {
    m_properties["Field"].as_option().attach_trigger ( boost::bind ( &COutputField::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    Common::URI field_path (property("Field").value<Common::URI>());
    scalar_field = look_component<CField>(field_path);
    scalar_name = scalar_field->field_name();
  }

  /// Virtual destructor
  virtual ~COutputField() {};

  /// Get the class name
  static std::string type_name () { return "COutputField"; }

  void set_loophelper (CElements& geometry_elements )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*scalar_field, geometry_elements ) );
  }

  template < typename SFType >
  void executeT ( Uint elem )
  {
    execute(elem);
  }

  virtual void execute ( Uint elem )
  {
//    CFinfo << "   " << scalar_name << "["<<elem<<"] = " << data->scalars[elem][0] << CFendl;
  }

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& scalar_field, CElements& geometry_elements) :
      field_elements(geometry_elements.get_field_elements(scalar_field.field_name())),
      scalars(field_elements.data())
    { }
    CElements& field_elements;
    CTable<Real>& scalars;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr scalar_field;
  std::string scalar_name;
};

/////////////////////////////////////////////////////////////////////////////////////

class Mesh_API CComputeVolumes : public CFieldOperation
{
public: // typedefs

  typedef boost::shared_ptr<CComputeVolumes> Ptr;
  typedef boost::shared_ptr<CComputeVolumes const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeVolumes ( const std::string& name ) : CFieldOperation(name)
  {
    m_properties["Field"].as_option().attach_trigger ( boost::bind ( &CComputeVolumes::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    Common::URI field_path (property("Field").value<Common::URI>());
//    CFdebug << "field_path = " << field_path.string() << CFendl;
    volume_field = look_component<CField>(field_path);
  }

  /// Virtual destructor
  virtual ~CComputeVolumes() {};

  /// Get the class name
  static std::string type_name () { return "CComputeVolumes"; }

  void set_loophelper (CElements& geometry_elements )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*volume_field, geometry_elements ) );
  }

  template < typename SFType >
  void executeT ( Uint elem )
  {
    typename SFType::NodeMatrixT nodes;
    fill( nodes, data->coordinates, data->connectivity_table[elem] );
    data->volumes[elem][0] = SFType::volume( nodes );
  }

  virtual void execute ( Uint elem )
  {
    ElementType::NodesT nodes(data->connectivity_table.row_size(), data->coordinates.row_size());
    fill(nodes, data->coordinates, data->connectivity_table[elem]);
    data->volumes[elem][0] = data->field_elements.element_type().compute_volume( nodes );
  }

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& volume_field, CElements& geometry_elements) :
      field_elements(geometry_elements.get_field_elements(volume_field.field_name())),
      volumes(field_elements.data()),
      coordinates(field_elements.nodes().coordinates()),
      connectivity_table(field_elements.connectivity_table())
    { }
    CElements& field_elements;
    CTable<Real>& volumes;
    CTable<Real>& coordinates;
    CTable<Uint>& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr volume_field;
};

/////////////////////////////////////////////////////////////////////////////////////

class Mesh_API CSetValue : public CFieldOperation
{
public: // typedefs

  typedef boost::shared_ptr<CSetValue> Ptr;
  typedef boost::shared_ptr<CSetValue const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSetValue ( const std::string& name ) : CFieldOperation(name)
  {
    m_properties.add_option< Common::OptionT<Common::URI> > ("Field","Field URI to output", Common::URI("cpath://"))->mark_basic();
    m_properties["Field"].as_option().attach_trigger ( boost::bind ( &CSetValue::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    Common::URI field_path (property("Field").value<Common::URI>());
//    CFdebug << "field_path = " << field_path.string() << CFendl;
    field = look_component<CField>(field_path);
  }

  /// Virtual destructor
  virtual ~CSetValue() {};

  /// Get the class name
  static std::string type_name () { return "CSetValue"; }

  virtual void set_loophelper (CTable<Real>& coordinates )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*field, coordinates ) );
  }

  template < typename SFType >
  void executeT ( Uint node )
  {
    execute(node);
  }

  virtual void execute ( Uint node )
  {

    CTable<Real>& field_data = data->field_data;
    field_data[node][0] = data->coordinates[node][XX]*data->coordinates[node][XX];
  }

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& field, CTable<Real>& coords) :
      coordinates(coords),
      node_connectivity(*coords.look_component<CNodeConnectivity>("../node_connectivity")),
      local_field(coords.get_parent()->as_type<CRegion>()->get_field(field.name())),
      field_data(Common::find_component_with_tag<CTable<Real> >(local_field, "field_data"))
    { }
    const CTable<Real>& coordinates;
    const CNodeConnectivity& node_connectivity;
    CField& local_field;
    CTable<Real>& field_data;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr field;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFieldOperation_hpp
