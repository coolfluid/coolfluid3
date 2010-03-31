#ifndef COOLFluiD_Common_DummyCommon_hh
#define COOLFLuiD_Common_DummyCommon_hh

#include <string>

#include "Common/CommonAPI.hh"

namespace COOLFluiD {

namespace Common {
	
	class Common_API DummyCommon
		{
		public:
			
			DummyCommon();
			
			std::string getString(); 
			
		};
}   
}

#endif // COOLFLuiD_Common_DummyCommon_hh