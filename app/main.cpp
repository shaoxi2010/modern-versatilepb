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
		for (int i = 0; i < 3; i++) {
			printf("Hello FreeRTOS!\n");
			Delay(Ticks::MsToTicks(500));
		}
        Thread::EndScheduler();
	}
};

int main()
{
	HelloTask hello(configMINIMAL_STACK_SIZE, 0);
	hello.Start();
	Thread::StartScheduler();
	printf("scheduler ending!\n");
	return 0;
}