#include "FreeRTOS.h"
#include "thread.hpp"
#include "ticks.hpp"
#include <stdio.h>

using namespace cpp_freertos;

class HelloTask : public Thread
{
  public:
	using Thread::Thread;

  private:
	void Run() override
	{
		for (;;) {
			printf("Hello FreeRTOS!\n");
			Delay(Ticks::MsToTicks(500));
		}
	}
};

int main()
{
	HelloTask hello(configMINIMAL_STACK_SIZE, 0);
	hello.Start();
	Thread::StartScheduler();
	return 0;
}