// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_CProtoFieldAction_hpp
#define CF_Solver_Actions_Proto_CProtoFieldAction_hpp

#include <set>

#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField2.hpp"

#include "Solver/Actions/CFieldAction.hpp"

#include "Terminals.hpp"
#include "Transforms.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
  
/// Abstract base class that encapsulates proto expressions that will be executed for a given region
template<typename ExprT>
class CProtoFieldAction : public CFieldAction
{
public:
  typedef boost::shared_ptr< CProtoFieldAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoFieldAction<ExprT> const> ConstPtr;

  CProtoFieldAction(const std::string& name) :
    CFieldAction(name)
  {
    m_region_path = boost::dynamic_pointer_cast<Common::OptionURI>( properties().template add_option<Common::OptionURI>("Region", "Region to loop over", std::string()) );
    m_region_path.lock()->supported_protocol(CF::Common::URI::Scheme::CPATH);
    m_region_path.lock()->mark_basic();
  }

  static std::string type_name() { return "CProtoFieldAction"; }

  /// Set the expression
  void set_expression(const ExprT& expr)
  {
    m_expr.reset(new CopiedExprT(boost::proto::deep_copy(expr)));
    // Store the variables
    CopyNumberedVars<VariablesT> ctx(m_variables);
    boost::proto::eval(expr, ctx);
    boost::fusion::for_each(m_variables, AddVariableOptions(follow()));
    raise_event("tree_updated");
  }

  /// Returns a reference to the region that we want to run an expression for
  Mesh::CRegion& root_region()
  {
    return *access_component_ptr(m_region_path.lock()->value_str())->as_ptr<Mesh::CRegion>();
  }
  
  /// Returns a reference to the region that we want to run an expression for
  const Mesh::CRegion& root_region() const
  {
    return *access_component_ptr(m_region_path.lock()->value_str())->as_ptr<Mesh::CRegion>();
  }
  
  virtual StringsT variable_names() const
  {
    StringsT result;
    boost::fusion::for_each( m_variables, GetVariableNames(result) );
    return result;
  }
  
  virtual StringsT field_names() const
  {
    StringsT result;
    boost::fusion::for_each( m_variables, GetFieldNames(result) );
    return result;
  }
  
  virtual SizesT variable_sizes() const
  {
    SizesT result;
    boost::fusion::for_each( m_variables, GetVariableSizes(result) );
    return result;
  }
  
  virtual Real nb_dofs() const
  {
    // Access the mesh
    const Mesh::CMesh& mesh = Common::find_parent_component<Mesh::CMesh>( root_region() );
    
    const SizesT var_sizes = variable_sizes();
    const StringsT fd_names = field_names();
    Uint result = 0;
    const Uint nb_vars = var_sizes.size();
    
    std::set<std::string> unique_fields;
    BOOST_FOREACH(const std::string& fd_name, fd_names)
    {
      if(unique_fields.insert(fd_name).second)
      {
        const Uint nb_field_rows = mesh.get_child_ptr(fd_name)->as_ptr<Mesh::CField2>()->data().size();
        for(Uint i = 0; i != nb_vars; ++i)
        {
          if(fd_names[i] == fd_name)
            result += var_sizes[i] * nb_field_rows;
        }
      }
    }
    
    return result;
  }
  
  virtual void create_fields()
  {
    // Access the mesh
    Mesh::CMesh& mesh = Common::find_parent_component<Mesh::CMesh>( root_region() );
    
    const StringsT fd_names = field_names();
    
    const Uint nb_fd_vars = fd_names.size();
    
    std::set<std::string> unique_fields;
    BOOST_FOREACH(const std::string& s, fd_names)
    {
      unique_fields.insert(s);
    }
    
    const StringsT var_names = variable_names();
    const SizesT var_sizes = variable_sizes();
    
    std::vector<bool> field_constness;
    boost::fusion::for_each( m_variables, GetFieldConstness(field_constness) );
    
    BOOST_FOREACH(const std::string& fd_name, unique_fields)
    {
      StringsT fd_var_names;
      std::vector<Mesh::CField2::VarType> fd_var_types;
      
      // Check if the field exists
      Component::ConstPtr existing_field = mesh.get_child_ptr(fd_name);
      
      Uint var_idx = 0;
      for(Uint i = 0; i != nb_fd_vars; ++i)
      {
        if(fd_names[i] != fd_name || field_constness[i])
          continue;
        
        fd_var_names.push_back(var_names[i]);
        fd_var_types.push_back( static_cast<Mesh::CField2::VarType>(var_sizes[i]) );
        
        if(existing_field)
        {
          const Mesh::CField2& efield = existing_field->as_type<Mesh::CField2>();

          if(   efield.nb_vars() <= var_idx
             || efield.var_name(var_idx) != var_names[i]
             || efield.var_type(var_names[i]) != var_sizes[i])
            throw Common::ValueExists(FromHere(), "Field with name " + fd_name + " exists, but is incompatible with the requested solution.");
        }
        
        ++var_idx;
      }
      
      if(!existing_field && fd_var_names.size())
        mesh.create_field2(fd_name, Mesh::CField2::Basis::POINT_BASED, fd_var_names, fd_var_types);
    }
  }

protected:
  // Type of a fusion vector that can contain a copy of each variable that is used in the expression
  typedef typename ExpressionProperties<ExprT>::VariablesT VariablesT;

  // type of the copied expression
  typedef typename boost::proto::result_of::deep_copy<ExprT>::type CopiedExprT;

  /// Copy of each variable in the expression
  VariablesT m_variables;

  /// Copy of the expression
  boost::scoped_ptr<CopiedExprT> m_expr;

private:
  /// Link to the option with the path to the region to loop over
  boost::weak_ptr<Common::OptionURI> m_region_path;
  
  /// Functor to get the variable names
  struct GetVariableNames
  {
    template<typename T>
    struct impl
    {
      void operator()(const T&, StringsT&) {}
    };
    
    template<typename T>
    struct impl< Field<T> >
    {
      void operator()(const Field<T>& var, StringsT& r)
      {
        r.push_back(var.var_name);
      }
    };
    
    GetVariableNames(StringsT& s) : result(s)
    {
    }
    
    template<typename T>
    void operator()(const T& var) const
    {
      impl<T>()(var, result);
    }
    
    StringsT& result;
  };
  
  /// Functor to get the field names
  struct GetFieldNames
  {
    template<typename T>
    struct impl
    {
      void operator()(const T&, StringsT&) {}
    };
    
    template<typename T>
    struct impl< Field<T> >
    {
      void operator()(const Field<T>& var, StringsT& r)
      {
        r.push_back(var.field_name);
      }
    };
    
    GetFieldNames(StringsT& s) : result(s)
    {
    }
    
    template<typename T>
    void operator()(const T& var) const
    {
      impl<T>()(var, result);
    }
    
    StringsT& result;
  };
  
  /// Functor to get the variable sizes
  struct GetVariableSizes
  {
    
    template<typename T>
    struct impl
    {
      void operator()(const T&, SizesT&) {}
    };
    
    template<typename T>
    struct impl< Field<T> >
    {
      void operator()(const Field<T>& var, SizesT& r)
      {
        r.push_back(1);
      }
    };
    GetVariableSizes(SizesT& s) : result(s)
    {
    }
    
    template<typename T>
    void operator()(const T& var) const
    {
      impl<T>()(var, result);
    }
    
    SizesT& result;
  };
  
  /// Functor to determine if fields are const
  struct GetFieldConstness
  {
    
    template<typename T>
    struct impl
    {
      void operator()(const T&, std::vector<bool>&) {}
    };
    
    template<typename T>
    struct impl< Field<T> >
    {
      void operator()(const Field<T>& var, std::vector<bool>& r)
      {
        r.push_back(var.is_const);
      }
    };
    GetFieldConstness(std::vector<bool>& b) : result(b)
    {
    }
    
    template<typename T>
    void operator()(const T& var) const
    {
      impl<T>()(var, result);
    }
    
    std::vector<bool>& result;
  };
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoFieldAction_hpp
