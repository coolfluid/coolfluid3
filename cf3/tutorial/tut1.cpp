// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file tut1.cpp
/// @brief Tutorial 1
///
/// This tutorial shows how create components, reorganize components, create links,
/// use Handles, and find components

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  // Create a main component to become a root.
  // Note that root has to be a shared_ptr, as it is not owned.
  shared_ptr<Group> world = allocate_component<Group>("World");



  // Create a few continents and countries
  // Note that the created components are stored in Handle,
  // as it is a non-owning pointer. The ownership is inside the parent component.
  Handle<Group> europe  = world->create_component<Group>("Europe");
  Handle<Group> africa  = world->create_component<Group>("Africa");
  Handle<Group> america = world->create_component<Group>("America");
  Handle<Group> asia    = world->create_component<Group>("Asia");

  Handle<Group> belgium = europe->create_component<Group>("Belgium");
  Handle<Group> uk      = europe->create_component<Group>("United_Kingdom");

  Handle<Group> canada = america->create_component<Group>("Canada");
  Handle<Group> usa    = america->create_component<Group>("USA");

  Handle<Group> india    = asia->create_component<Group>("India");
  Handle<Group> thailand = asia->create_component<Group>("Thailand");
  Handle<Group> china    = asia->create_component<Group>("China");

  Handle<Group> south_africa = africa->create_component<Group>("South_Africa");



  // Create a link pointing to a country
  Handle<Link> current_country = world->create_component<Link>("Current_Country");
  current_country->link_to(*belgium);
  CFinfo << "Current country : " << current_country->follow()->uri() << CFendl;



  // Mistakenly add Germany to Africa
  Handle<Group> germany = africa->create_component<Group>("Germany");
  CFinfo << "Germany's wrong path  : " << germany->uri() << CFendl;

  // Correctly add Germany to Europe
  germany->move_to(*europe);
  CFinfo << "Germany's correct path: " << germany->uri() << CFendl;



  // Check if the handle to Thailand valid
  CFinfo << "Is Thailand there? --> " << is_not_null(thailand) << CFendl;

  // Remove Thailand from the structure
  asia->remove_component("Thailand");
  CFinfo << "Is Thailand there? --> " << is_not_null(thailand) << CFendl;



  // Print the whole data structure. Useful to visualize the structure, as
  // a file-system analogy
  CFinfo << world->tree() << CFendl;



  // Access United Kingdom by its path
  Handle<Component> a_country = world->access_component("cpath:/Europe/United_Kingdom");
  CFinfo << "Accessed country " << a_country->name() << CFendl;

  // Find Canada in the structure
  Handle<Group> found_canada = find_component_ptr_recursively_with_name<Group>(*world,"Canada");
  CFinfo << "Found canada at " << found_canada->uri() << CFendl;

  return 0;
}

/* OUTPUT:

Current country : cpath:/Europe/Belgium
Germany's wrong path  : cpath:/Africa/Germany
Germany's correct path: cpath:/Europe/Germany
Is Thailand there? --> 1
Is Thailand there? --> 0
World
  Europe
    Belgium
    United_Kingdom
    Germany
  Africa
    South_Africa
  America
    Canada
    USA
  Asia
    India
    China
  Current_Country -> cpath:/Europe/Belgium

Accessed country United_Kingdom
Found canada at cpath:/America/Canada
*/
