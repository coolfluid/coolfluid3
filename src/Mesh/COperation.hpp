#ifndef CF_Mesh_COperation_hpp
#define CF_Mesh_COperation_hpp

#include "Common/ObjectProvider.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/CElements.hpp"

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
  static void defineConfigOptions ( Common::OptionList& options ) {}
  
  
  void needs(CField& field);
  void needs(CField& field1, CField& field2);
  void needs(CField& field1, CField& field2, CField& field3);
  virtual void needs_fields (std::vector<CField::Ptr>& fields);
  
  void needs(CRegion& region);
  void needs(CRegion& region1, CRegion& region2);
  void needs(CRegion& region1, CRegion& region2, CRegion& region3);
  virtual void needs_regions (std::vector<CRegion::Ptr>& regions);
  
  void stores(CField& field);
  void stores(CField& field1, CField& field2);
  void stores(CField& field1, CField& field2, CField& field3);
  virtual void stores_fields (std::vector<CField::Ptr>& fields);
  
  virtual void setup (CElements& geometry_elements );
  
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
  static void defineConfigOptions ( Common::OptionList& options ) {}
    
  void setup (CElements& geometry_elements )
  {
    m_op1->setup(geometry_elements);
    m_op2->setup(geometry_elements);
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
  }
  
  /// Virtual destructor
  virtual ~COutputField() {};
  
  /// Get the class name
  static std::string type_name () { return "COutputField"; }
  
  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}
  
  void needs_fields(std::vector<CField::Ptr>& fields)
  {
    scalar_field = fields[0]; 
  }
  
  void setup (CElements& geometry_elements )
  {
    CFieldElements& field_elements = geometry_elements.get_field_elements(scalar_field->field_name());
    scalar_name = scalar_field->field_name();
    scalars = field_elements.elemental_data().get_type<CArray>();
    CFinfo << field_elements.full_path().string() << CFendl;
  }
  
  template < typename SFType >
  void executeT ( Uint elem )
  {
    execute(elem);
  }
  
  void execute ( Uint elem )
  {
    cf_assert(scalars.get());
    CFinfo << "   " << scalar_field->field_name() << "["<<elem<<"] = " << scalars->array()[elem][0] << CFendl;
  }
  
private: // helper functions
  
  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
  
private: // data
  
  CField::Ptr scalar_field;
  CArray::Ptr scalars;
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
  }
  
  
  /// Virtual destructor
  virtual ~CComputeVolumes() {};
  
  /// Get the class name
  static std::string type_name () { return "CComputeVolume"; }
  
  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}
  
  void stores_fields(std::vector<CField::Ptr>& fields)
  {
    volume_field = fields[0];
  }
  
  void setup (CElements& geometry_elements )
  {
    field_elements = geometry_elements.get_field_elements(volume_field->field_name()).get_type<CFieldElements>();
    volumes = field_elements->elemental_data().get_type<CArray>();    
    coordinates = field_elements->coordinates().get_type<CArray>();
    connectivity_table = field_elements->connectivity_table().get_type<CTable>();
  }
  
  template < typename SFType >
  void executeT ( Uint elem )
  {
    cf_assert(volumes.get());
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    volumes->array()[elem][0] = SFType::volume( nodes );
  }
  
  void execute ( Uint elem )
  {
    cf_assert(volumes.get());
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    volumes->array()[elem][0] = field_elements->element_type().computeVolume( nodes );
  }
  
private: // helper functions
  
  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
  
private: // data
  
  CField::Ptr volume_field;
  CFieldElements::Ptr field_elements;
  CArray::Ptr volumes;
  CArray::Ptr coordinates;
  CTable::Ptr connectivity_table;
};

/////////////////////////////////////////////////////////////////////////////////////
  
} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_COperation_hpp
