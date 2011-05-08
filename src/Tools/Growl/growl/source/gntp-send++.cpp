#include <growl++.hpp>

int main(int argc, char **argv)
{
	const char *n[2] = { "alice" , "willem" };
	//Growl *growl = new Growl(GROWL_UDP,NULL,"gntp_send++",(const char **const)n,2);
	Growl *growl = new Growl(GROWL_UDP,NULL,"COOLFluiD",(const char **const)n,2);

//	Growl(const Growl_Protocol _protocol, const char *const _password, const char* const _appliciation, const char **const _notifications, const int _notifications_count);
//	Growl(const Growl_Protocol _protocol, const char *const _server, const char *const _password, const char *const _application, const char **const _notifications, const int _notifications_count);

	//growl->Notify("willem","title","message");
	growl->Notify("willem","title","message");

	delete(growl);

	return 0;
}
