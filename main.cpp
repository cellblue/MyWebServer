#include <unistd.h>
#include "WebServer.h"

int main() {
	WebServer server(1416, 3, 60000, false, 4);
	server.Start();
}
