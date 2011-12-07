// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/Log.hpp"

using namespace cf3::common;

int main ( int argc, char * argv[])
{
   Option * option = new OptionArrayT<bool>("bla", "", std::vector<bool>());

   CFinfo << "pointer\t\t" << option << " " <<
       static_cast< OptionArray* >(option) << " " <<
       static_cast< OptionArrayT<bool>* >(option) << CFendl;

   std::vector<bool> vect;
   boost::shared_ptr<Option> opt(new OptionArrayT<bool>("bla", "", vect));

   CFinfo << "OptionArrayT\t" << opt.get() << " " <<
       boost::dynamic_pointer_cast<OptionArray>(opt).get() << " " <<
       boost::dynamic_pointer_cast<OptionArrayT<bool> >(opt).get() << CFendl;

   Property::Ptr optT(new OptionT<bool>("bla", "", true));

   CFinfo << "OptionT\t\t" << optT.get() << " " <<
       boost::dynamic_pointer_cast<Option>(optT).get() << " " <<
       boost::dynamic_pointer_cast<OptionT<bool> >(optT).get() << CFendl;

   Property::Ptr optURI(new OptionURI("bla", "", URI()));

   CFinfo << "OptionURI\t" << optURI.get() << " " <<
       boost::dynamic_pointer_cast<Option>(optURI).get() << " " <<
       boost::dynamic_pointer_cast<OptionURI >(optURI).get() << CFendl;

//   Component::Ptr cmp(new NLink("link"));

//   CFinfo << "NLink " << cmp.get() << " " << boost::dynamic_pointer_cast<CNode>(cmp).get() << " " << boost::dynamic_pointer_cast<NLink>(cmp).get() << CFendl;

   return 0;
}

