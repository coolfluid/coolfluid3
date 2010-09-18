// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_COperation_hpp
#define CF_Mesh_COperation_hpp

#include "Common/ObjectProvider.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ConnectivityData.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

///////////////////////////////////////////////////////////////////////////////////////

class Mesh_API COperation : public Common::Component
{
public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < COperation , NB_ARGS_1 > PROVIDER;

  /// pointers
  typedef boost::shared_ptr<COperation> Ptr;
  typedef boost::shared_ptr<COperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  COperation ( const CName& name );

  /// Virtual destructor
  virtual ~COperation() {};

  /// Get the class name
  static std::string type_name () { return "COperation"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  virtual void set_loophelper (CElements& geometry_elements );

  virtual void set_loophelper (CArray& coordinates );

  virtual void execute (Uint index = 0 )
  {
    throw NotImplemented(FromHere(), "Must create child that overloads this function");
  }

  /// Templated version for high efficiency
  template < typename EType >
  void executeT ( Uint index=0 )
  {
    throw NotImplemented(FromHere(), "Must create child that overloads this function");
  }

  virtual COperation& operation();

  /// Only for use in non-templatized version
  COperation& create_operation(const std::string operation_type);


  ///@todo this function is temporary until statically linked childs are available.
  virtual void bind()
  {
    add_component(operation().get());
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  Uint m_counter;

};

///////////////////////////////////////////////////////////////////////////////////////

template < typename OP1, typename OP2 >
class Mesh_API COperationMergeT : public COperation
{
public: // typedefs

  typedef boost::shared_ptr<COperationMergeT> Ptr;
  typedef boost::shared_ptr<COperationMergeT const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  COperationMergeT ( const CName& name ) :
    COperation(name),
    m_op1(new OP1("operation_1"), Deleter<OP1>()),
    m_op2(new OP2("operation_2"), Deleter<OP2>())
  {
    BUILD_COMPONENT;
  }

  /// Virtual destructor
  virtual ~COperationMergeT() {};

  /// Get the class name
  static std::string type_name () { return "COutputField"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  void set_loophelper (CElements& geometry_elements )
  {
    m_op1->set_loophelper(geometry_elements);
    m_op2->set_loophelper(geometry_elements);
  }

  const OP1& operation1() const
  {
    return *m_op1;
  }

  OP1& operation1()
  {
    return *m_op1;
  }


  const OP2& operation2() const
  {
    return *m_op2;
  }

  OP2& operation2()
  {
    return *m_op2;
  }

  template < typename EType >
  void executeT (  Uint elem )
  {
    m_op1->template executeT<EType>( elem );
    m_op2->template executeT<EType>( elem );
  }

  void execute (  Uint elem )
  {
    m_op1->execute( elem );
    m_op2->execute( elem );
  }


  ///@todo this function is temporary until statically linked childs are available.
  virtual void bind()
  {
    add_component(operation1().get());
    add_component(operation2().get());
  }


private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  typename OP1::Ptr m_op1;
  typename OP2::Ptr m_op2;
};

typedef COperationMergeT<COperation,COperation> COperationMerge;

///////////////////////////////////////////////////////////////////////////////////////

class Mesh_API COutputField : public COperation
{
public: // typedefs

  typedef boost::shared_ptr<COutputField> Ptr;
  typedef boost::shared_ptr<COutputField const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  COutputField ( const CName& name ) : COperation(name)
  {
    BUILD_COMPONENT;
    m_property_list["Field"].as_option().attach_trigger ( boost::bind ( &COutputField::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    CPath field_path (property("Field").value<URI>());
    CFdebug << "field_path = " << field_path.string() << CFendl;
    scalar_field = look_component_type<CField>(field_path);
    scalar_name = scalar_field->field_name();
  }

  /// Virtual destructor
  virtual ~COutputField() {};

  /// Get the class name
  static std::string type_name () { return "COutputField"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options )
  {
    options.add_option< OptionT<URI> > ("Field","Field URI to output", URI("cpath://"))->mark_basic();
  }

  void set_loophelper (CElements& geometry_elements )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*scalar_field, geometry_elements ) );
    CFinfo << data->field_elements.full_path().string() << CFendl;
  }

  template < typename SFType >
  void executeT ( Uint elem )
  {
    execute(elem);
  }

  void execute ( Uint elem )
  {
    CFinfo << "   " << scalar_name << "["<<elem<<"] = " << data->scalars[elem][0] << CFendl;
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& scalar_field, CElements& geometry_elements) :
      field_elements(geometry_elements.get_field_elements(scalar_field.field_name())),
      scalars(field_elements.data())
    { }
    CFieldElements& field_elements;
    CArray& scalars;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr scalar_field;
  std::string scalar_name;
};

/////////////////////////////////////////////////////////////////////////////////////

class Mesh_API CComputeVolumes : public COperation
{
public: // typedefs

  typedef boost::shared_ptr<CComputeVolumes> Ptr;
  typedef boost::shared_ptr<CComputeVolumes const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CComputeVolumes ( const CName& name ) : COperation(name)
  {
    BUILD_COMPONENT;
    m_property_list["Field"].as_option().attach_trigger ( boost::bind ( &CComputeVolumes::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    CPath field_path (property("Field").value<URI>());
    CFdebug << "field_path = " << field_path.string() << CFendl;
    volume_field = look_component_type<CField>(field_path);
  }

  /// Virtual destructor
  virtual ~CComputeVolumes() {};

  /// Get the class name
  static std::string type_name () { return "CComputeVolume"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options )
  {
    options.add_option< OptionT<URI> > ("Field","Field URI to output", URI("cpath://"))->mark_basic();
  }

  void set_loophelper (CElements& geometry_elements )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*volume_field, geometry_elements ) );
  }

  template < typename SFType >
  void executeT ( Uint elem )
  {
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), data->coordinates, data->connectivity_table, elem );
    data->volumes[elem][0] = SFType::volume( nodes );
  }

  void execute ( Uint elem )
  {
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), data->coordinates, data->connectivity_table, elem );
    data->volumes[elem][0] = data->field_elements.element_type().computeVolume( nodes );
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& volume_field, CElements& geometry_elements) :
      field_elements(geometry_elements.get_field_elements(volume_field.field_name())),
      volumes(field_elements.data()),
      coordinates(field_elements.coordinates()),
      connectivity_table(field_elements.connectivity_table())
    { }
    CFieldElements& field_elements;
    CArray& volumes;
    CArray& coordinates;
    CTable& connectivity_table;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr volume_field;
};

/////////////////////////////////////////////////////////////////////////////////////

class Mesh_API CSetValue : public COperation
{
public: // typedefs

  typedef boost::shared_ptr<CSetValue> Ptr;
  typedef boost::shared_ptr<CSetValue const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CSetValue ( const CName& name ) : COperation(name)
  {
    BUILD_COMPONENT;
    m_property_list["Field"].as_option().attach_trigger ( boost::bind ( &CSetValue::trigger_Field,   this ) );
  }

  void trigger_Field()
  {
    CPath field_path (property("Field").value<URI>());
    CFdebug << "field_path = " << field_path.string() << CFendl;
    field = look_component_type<CField>(field_path);
  }

  /// Virtual destructor
  virtual ~CSetValue() {};

  /// Get the class name
  static std::string type_name () { return "CComputeVolume"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options )
  {
    options.add_option< OptionT<URI> > ("Field","Field URI to output", URI("cpath://"))->mark_basic();
  }

  virtual void set_loophelper (CArray& coordinates )
  {
    data = boost::shared_ptr<LoopHelper> ( new LoopHelper(*field, coordinates ) );
  }

  template < typename SFType >
  void executeT ( Uint node )
  {
    execute(node);
  }

  void execute ( Uint node )
  {

    CArray& field_data = data->field_data;
    field_data[node][0] = data->coordinates[node][XX]*data->coordinates[node][XX];
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  struct LoopHelper
  {
    LoopHelper(CField& field, CArray& coords) :
      coordinates(coords),
      node_connectivity(*coords.look_component_type<CNodeConnectivity>("../node_connectivity")),
      local_field(coords.get_parent()->get_type<CRegion>()->get_field(field.name())),
      field_data(get_tagged_component_typed<CArray>(local_field, "field_data"))
    { }
    const CArray& coordinates;
    const CNodeConnectivity& node_connectivity;
    CField& local_field;
    CArray& field_data;
  };

  boost::shared_ptr<LoopHelper> data;

  CField::Ptr field;
};

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_COperation_hpp
