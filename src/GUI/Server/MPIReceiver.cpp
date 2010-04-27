#include "Common/BuilderParser.hpp"
#include "Common/BuilderParserRules.hpp"
#include "Common/BuilderParserFrameInfo.hpp"

#include "GUI/Server/MPIReceiver.hpp"

using namespace MPI;

using namespace CF::Common;
using namespace CF::GUI::Server;

void MPIReceiver::receive(BuilderParserFrameInfo & frame, Intercomm comm,
                          const BuilderParserRules & rules)
{
  char data[65536];
  frame.clear();

  comm.Recv(data, sizeof(data), CHAR, ANY_SOURCE, ANY_TAG);

  BuilderParser::parseFrame(data, rules, frame);
}
