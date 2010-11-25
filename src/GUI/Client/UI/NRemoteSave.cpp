// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QDialogButtonBox>
#include <QFileIconProvider>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/UI/TypeAndNameDialog.hpp"
#include "GUI/Client/UI/NRemoteSave.hpp"

using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

NRemoteSave::NRemoteSave(QMainWindow * parent)
  : NRemoteBrowser("NRemoteSave", parent)
{
  add_tag( type_name() );

  this->setIncludeFiles(true);
  this->setIncludeNoExtension(false);

  this->setWindowTitle("Save configuration");

  this->allowMultipleSelect = false;

  m_btFileName = this->addButton("Set file name", QDialogButtonBox::ActionRole);

  m_btNewDirectory = this->addButton("New directory", QDialogButtonBox::ActionRole);

  m_fileNameDialog = new TypeAndNameDialog("File name", "File extension",
                                                 (QWidget *) this);

  connect(m_btFileName, SIGNAL(clicked()),
          this, SLOT(btFileNameClick()));

  connect(m_btNewDirectory, SIGNAL(clicked()),
          this, SLOT(btNewDirectoryClicked()));

  this->allowModifyBools = false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRemoteSave::~NRemoteSave()
{
  delete m_fileNameDialog;
  delete m_btFileName;
  delete m_btNewDirectory;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NRemoteSave::Ptr NRemoteSave::create(QMainWindow * parent)
{
  NRemoteSave::Ptr rsf(new NRemoteSave(parent));

  ClientRoot::browser()->addNode(rsf);

  return rsf;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteSave::toolTip() const
{
  return this->getComponentType();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteSave::btFileNameClick()
{
  QString selectedExtension;
  QString name;
  bool overwrite;

  do
  {
    overwrite = true;
    name = m_fileNameDialog->show(this->extensions(), selectedExtension);

    if(!name.isEmpty())
    {
      if(!name.endsWith(selectedExtension))
        name.append(".").append(selectedExtension);

      this->setStatus(QString("Current file name: \"%1\"").arg(name));

      if(this->itemExists(name))
      {
        int answer;
        QString path = this->currentPath();
        QString message = "The file '%1' already exists. Are you sure to "
        "overwrite this file?";

        this->assemblePath(path, name);

        answer = QMessageBox::question(this, "Confirmation", message.arg(path),
                                       QMessageBox::Yes | QMessageBox::No);

        overwrite = answer == QMessageBox::Yes;
      }
    }
  } while(!overwrite); // we continue while user says to not overwrite

  m_fileName = name;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteSave::btNewDirectoryClicked()
{
  QString dirName;
  bool ok;

  dirName = QInputDialog::getText(this, "New directory", "Enter the name of the "
                                  "new directory", QLineEdit::Normal, "", &ok);

  if(ok)
  {
//    if(!dirName.isEmpty())
//      m_clientCore->createDir(this->index, this->getCurrentPath(), dirName);
  }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ValidationPolicy NRemoteSave::isAcceptable(const QString & name, bool isDir)
{
  ValidationPolicy validation = POLICY_NOT_VALID;

  if(isDir && m_fileName.isEmpty())
  {
    this->showError(QString("You must select a file or enter a new file name "
                            "using '%1'").arg(m_btFileName->text()));
  }

  else if(isDir)
  {
    // this->selectedFile is empty, because reinitValues() was called
    m_selectedFile.append(name);
    this->assemblePath(m_selectedFile, m_fileName);
    validation = POLICY_VALID;
  }

  else if(!isDir)
  {
    int answer = QMessageBox::question(this, "Confirmation",
                                       QString("The file '%1' already exists. "
                                       "Are you sure you want to overwrite this "
                                       "file?").arg(name),
                                       QMessageBox::Yes | QMessageBox::No);

    if(answer == QMessageBox::Yes)
    {
      validation = POLICY_VALID;
      m_selectedFile = name;
    }
  }

  return validation;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NRemoteSave::reinitValues()
{
  m_selectedFile.clear();
  m_fileName.clear();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QString NRemoteSave::selectedFile() const
{
  return m_selectedFile;
}
