#include "main.h"
#include "Message.h"

// ./Message /dev/ttyS1 9600 172.16.8.23 49152

int main(int argc, char *argv[])
{
  string isUdp;
  Message message;

  cout << "enter udp or uart (other for exit)" << endl;
  cin >> isUdp;

  if(isUdp == "udp")
  {
    message.txrxUDP();
  }
  else if(isUdp == "uart")
  {
    message.txrxUART();
  }

  cout << "end program !" << endl;

  return 0;
}
