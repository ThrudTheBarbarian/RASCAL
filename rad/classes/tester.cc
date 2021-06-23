#include "datamgr.h"
#include "taskfft.h"
#include "tester.h"

/******************************************************************************\
|* Construct a tester
\******************************************************************************/
Tester::Tester()
	{
	_duts.append(&DataMgr::instance());
	_duts.append(new TaskFFT);
	}

void Tester::test(void)
	{
	for (Testable* dut : _duts)
		{
		fprintf(stderr, "Testing class %s:\n", dut->testClassName());

		for (int i=0; i<dut->numTests(); i++)
			{
			Testable::TestResult state = dut->runTest(i);
			const char *msg = (state == Testable::TEST_FAIL) ? "FAIL" : "pass";
			fprintf(stderr, "  Test %3d: %s\n", i, msg);
			}
		}
	}
