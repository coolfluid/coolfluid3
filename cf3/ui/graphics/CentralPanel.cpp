// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>

#include "ui/core/CommitDetails.hpp"
#include "ui/core/NLog.hpp"
#include "ui/core/NTree.hpp"

#include "ui/graphics/ModifiedOptionsDialog.hpp"
#include "ui/graphics/OptionLayout.hpp"

#include "ui/graphics/CentralPanel.hpp"

using namespace cf3::common;
using namespace cf3::ui::core;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace graphics {

//////////////////////////////////////////////////////////////////////////

CentralPanel::CentralPanel(QWidget * parent)
  : QWidget(parent)
{
  Handle< NTree > tree = NTree::global();

  // create the components
  m_scroll_basic_options = new QScrollArea();
  m_scroll_advanced_options = new QScrollArea();
  m_gbox_basic_options = new QGroupBox(m_scroll_basic_options);
  m_gbox_advanced_options = new QGroupBox(m_scroll_advanced_options);
  m_bt_apply = new QPushButton("Apply");
  m_bt_see_changes = new QPushButton("See changes");
  m_bt_forget = new QPushButton("Forget");
  m_splitter = new QSplitter(this);

  m_basic_option_layout = new OptionLayout(m_gbox_basic_options);
  m_advanced_option_layout = new OptionLayout(m_gbox_advanced_options);

  m_main_layout = new QGridLayout(this);

  m_main_layout->setContentsMargins(0, 11, 0, 0);

  m_splitter->setOrientation(Qt::Vertical);

  m_scroll_basic_options->setWidgetResizable(true);
  m_scroll_basic_options->setWidget(m_gbox_basic_options);

  m_scroll_advanced_options->setWidgetResizable(true);
  m_scroll_advanced_options->setWidget(m_gbox_advanced_options);

  m_bt_see_changes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_bt_forget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  m_bt_apply->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  // add the components to the splitter
  m_splitter->addWidget(m_scroll_basic_options);
  m_splitter->addWidget(m_scroll_advanced_options);

  m_main_layout->addWidget(m_bt_see_changes, 0, 2);
  m_main_layout->addWidget(m_splitter, 1, 0, 1, 0);
  m_main_layout->addWidget(m_bt_forget, 2, 0);
  m_main_layout->addWidget(m_bt_apply, 2, 2, -1, -1, Qt::AlignRight);

  m_main_layout->setRowStretch(1, 10);

  advanced_mode_changed(tree->is_advanced_mode());

  this->set_buttons_visible(false);

  connect(m_bt_apply, SIGNAL(clicked()), this, SLOT(bt_apply_clicked()));
  connect(m_bt_see_changes, SIGNAL(clicked()), this, SLOT(bt_see_changes_clicked()));
  connect(m_bt_forget, SIGNAL(clicked()), this, SLOT(bt_forget_clicked()));

  connect(m_basic_option_layout, SIGNAL(value_changed()), this, SLOT(value_changed()));
  connect(m_advanced_option_layout, SIGNAL(value_changed()), this, SLOT(value_changed()));

  connect(tree.get(), SIGNAL(current_index_changed(QModelIndex, QModelIndex)),
          this, SLOT(current_index_changed(QModelIndex, QModelIndex)));

  connect(tree.get(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
          this, SLOT(data_changed(QModelIndex, QModelIndex)));

  connect(tree.get(), SIGNAL(advanced_mode_changed(bool)),
          this, SLOT(advanced_mode_changed(bool)));
}

//////////////////////////////////////////////////////////////////////////

CentralPanel::~CentralPanel()
{
  // buttons
  delete m_bt_apply;
  delete m_bt_forget;
  delete m_bt_see_changes;

  // layouts
  delete m_basic_option_layout;
  delete m_advanced_option_layout;

  // group boxes
  delete m_gbox_basic_options;
  delete m_gbox_advanced_options;

  // scroll areas
  delete m_scroll_advanced_options;
  delete m_scroll_basic_options;

  // splitter
  delete m_splitter;
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::set_options(const QList<boost::shared_ptr< Option > > & list)
{
  QList<boost::shared_ptr< Option > >::const_iterator it = list.begin();
  const Handle< NTree > & tree = NTree::global();

  // delete old options
  m_basic_option_layout->clear_options();
  m_advanced_option_layout->clear_options();

  // if there is at least one option, we set the group boxes title
  if(!list.isEmpty())
  {
    // get a UNIX-like path for the node
    QString parentPath = tree->node_path(tree->current_index());

    m_gbox_basic_options->setTitle(QString("Basic options of %1").arg(parentPath));
    m_gbox_advanced_options->setTitle(QString("Advanced options of %1").arg(parentPath));
    m_current_path = parentPath;
  }

  while(it != list.end())
  {
    // create the option
    try
    {
      boost::shared_ptr< Option > option = *it;
      bool basic = option->has_tag("basic");

      if (basic)
        m_basic_option_layout->add(option);
      else
        m_advanced_option_layout->add(option);
    }
    catch(Exception e)
    {
      NLog::global()->add_exception(e.what());
    }

    it++;

  } // end of "while()"

  // change row stretch and panel visibilities
  this->advanced_mode_changed(tree->is_advanced_mode());
  this->set_buttons_visible(!list.isEmpty());
  this->set_buttons_enabled(false);

  // set options to enabled or disabled (depending on their mode)
//  this->setEnabled(m_basicOptionsNodes, m_basicOptions);
//  this->setEnabled(m_advancedOptionsNodes, m_advancedOptions);
}

//////////////////////////////////////////////////////////////////////////

bool CentralPanel::is_modified() const
{
  return m_basic_option_layout->is_modified() || m_advanced_option_layout->is_modified();
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::list_modified_options(CommitDetails & commitDetails) const
{
  commitDetails.clear();
  commitDetails.set_node_path(m_current_path);

  m_basic_option_layout->modified_options(commitDetails);
  m_advanced_option_layout->modified_options(commitDetails);
}

//////////////////////////////////////////////////////////////////////////

QString CentralPanel::current_path() const
{
  return m_current_path;
}

/****************************************************************************

 PRIVATE METHODS

 ****************************************************************************/

void CentralPanel::set_buttons_visible(bool visible)
{
  m_bt_apply->setVisible(visible);
  m_bt_see_changes->setVisible(visible);
  m_bt_forget->setVisible(visible);
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::set_buttons_enabled(bool enabled)
{
  m_bt_apply->setEnabled(enabled);
  m_bt_see_changes->setEnabled(enabled);
  m_bt_forget->setEnabled(enabled);
}

/****************************************************************************

 SLOTS

 ****************************************************************************/

void CentralPanel::bt_apply_clicked()
{
  QMap<QString, QString> options;

  m_basic_option_layout->options(options, false);
  m_advanced_option_layout->options(options, false);

  // if there is at least one option that has been modified
  if(!options.isEmpty())
  {
    try
    {
      QModelIndex currentIndex = NTree::global()->current_index();

      NTree::global()->modify_options(currentIndex, options);

      m_basic_option_layout->commit_options();
      m_advanced_option_layout->commit_options();
    }
    catch (ValueNotFound & vnf)
    {
      NLog::global()->add_exception(vnf.msg().c_str());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::current_index_changed(const QModelIndex & newIndex, const QModelIndex & oldIndex)
{
  QList<boost::shared_ptr< Option > > options;
  NTree::global()->list_node_options(newIndex, options);

  this->set_options(options);
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::advanced_mode_changed(bool advanced)
{
  Handle< NTree > tree = NTree::global();

  // if the node went to a hidden state, we clear everything
  /// @todo what if options are modified ???
  if(!tree->check_index_visible(tree->current_index()))
  {
    m_basic_option_layout->clear_options();
    m_advanced_option_layout->clear_options();
  }

  m_scroll_advanced_options->setVisible(advanced && m_advanced_option_layout->has_options());

  // To avoid confusion, basic option panel is always showed if there is at
  // least one option for the selected object, even if all options are advanced.
  // Doing so, we ensure that the advanced options panel is *never* the
  // top one (if visible).
  m_scroll_basic_options->setVisible(m_basic_option_layout->has_options() ||
                                   m_scroll_advanced_options->isVisible());

  set_buttons_visible(m_scroll_basic_options->isVisible());
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::data_changed(const QModelIndex & first, const QModelIndex & last)
{
  QModelIndex currIndex = NTree::global()->current_index();

  if(first == last && first.row() == currIndex.row() && first.parent() == currIndex.parent())
    this->current_index_changed(first, QModelIndex());
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::bt_see_changes_clicked()
{
  CommitDetails details;
  ModifiedOptionsDialog dialog;

  this->list_modified_options(details);
  dialog.show(details);
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::bt_forget_clicked()
{
  this->current_index_changed(NTree::global()->current_index(), QModelIndex());
}

//////////////////////////////////////////////////////////////////////////

void CentralPanel::value_changed()
{
  this->set_buttons_enabled(this->is_modified());
}

//////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3
