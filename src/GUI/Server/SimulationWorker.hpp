// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_server_SimulationWorker_hpp
#define CF_server_SimulationWorker_hpp

////////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QHash>
#include <mpi.h>

#include "Common/MPI/PEInterface.hpp"

#include "GUI/Server/MPIListener.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Server {

////////////////////////////////////////////////////////////////////////////

  class CSimulator;
  class MPIListener;

  /// @brief Simulation worker

  /// @author Quentin Gasper

  class SimulationWorker : public QObject
  {
    Q_OBJECT

  public:

    /// @brief Constructor
    SimulationWorker();

    /// @brief Begins to listen to MPI communicators
    void listen();

    QString getLineHeader() const;

    public slots:

    /// @brief Sends a message to the manager

    /// @param message The message to send
    void sendToParent(const QString & message);

    private slots:

    /// @brief Slot called when the simulation has finished.
    void simulationFinished();

    /// @brief Slot called when a new frame comes from a MPI intercommunicator.

    /// @param senderComm Sender MPI communicator
    /// @param frameInfo Received frame information
//    void newFrame(const MPI::Intercomm & senderComm,
//                  const CF::Common::BuilderParserFrameInfo & frameInfo);

  private:

    /// @brief Manager-Worker m_protocol rules
//    CF::Common::ManagerWorkerProtocol m_protocol;

    /// @brief MPI listener
    MPIListener m_listener;

    /// @brief Server simulation
    CSimulator * m_srvSimulation;

    /// @brief Subsystem name
    QString m_subSystemName;

    /// @brief Subsystem type
    QString m_subSystemType;

    /// @brief Manager communicator
    MPI::Intercomm m_managerComm;

    /// @brief Communicators to other subsystems

    /// The key is the subsystem name. The value is the associated communicator.
    QHash<QString, MPI::Intercomm> m_comms;

    /// @brief The process rank.
    int m_rank;

    /// @brief Builds and sends a frame to the manager.

    /// @param fi Frame information
//    void buildAndSend(const CF::Common::BuilderParserFrameInfo & fi);

    /// @brief Builds and send an ACK frame

    /// @param type Frame type to ACK.
//    void ack(CF::Common::ManagerWorkerFrameType type);

    /// @brief Runs the simulation
    void runSimulation();

    /// @brief Opens a port

    /// @param remoteRole Remote subsystem name
    void openPort(const std::string & remoteRole);

    /// @brief Connects to a port

    /// @param portName Port name to connect to.
    /// @param remoteRole Remote subsystem name
    void connectToPort(const std::string & portName, const std::string & remoteRole);

    void setStatus(CF::Common::WorkerStatus::Type status);

    void setBarrier(MPI::Intercomm comm);

    void setBarrier(MPI::Intercomm comm, CF::Common::WorkerStatus::Type newStatus);

    void createSimulator();


  }; // class SimulationWorker

////////////////////////////////////////////////////////////////////////////

} // namespace Server
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_server_SimulationWorker_hpp
