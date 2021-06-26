#include <cstring>

#include <libra.h>

#include "taskfft.h"

/******************************************************************************\
|* Categorised logging support
\******************************************************************************/
#define LOG qDebug(log_data) << QTime::currentTime().toString("hh:mm:ss.zzz")
#define ERR qCritical(log_data) << QTime::currentTime().toString("hh:mm:ss.zzz")

/******************************************************************************\
|* Constructor: Only useful for testing
\******************************************************************************/
TaskFFT::TaskFFT(void)
		:QRunnable()
		,_numIQ(0)
		,_data(-1)
		,_results(-1)
		,_window(-1)
	{}

/******************************************************************************\
|* Constructor: single buffer
\******************************************************************************/
TaskFFT::TaskFFT(double *iq, int num)
		: QRunnable()
		, _numIQ(num/2)
		, _data(-1)
		,_results(-1)
		,_window(-1)
	{
	Q_ASSERT(num % 2 == 0);

	// Obtain two buffers, one for the I,Q inputs, one for outputs
	DataMgr &dmgr		= DataMgr::instance();
	_results			= dmgr.fftBlockFor(_numIQ);
	_data				= dmgr.fftBlockFor(_numIQ);

	// Copy the IQ data (_numIQ = number of complex doubles) to the input data
	fftw_complex *data	= dmgr.asFFT(_data);
	::memcpy(data, iq, _numIQ * sizeof(fftw_complex));

	// Now have _numIQ complex-pairs stored into {_data}. Rotate by Pi to
	// re-center the data after FFT
	_rotateByPi();
	}

/******************************************************************************\
|* Constructor: two buffers
\******************************************************************************/
TaskFFT::TaskFFT(double *iq1, int num1, double *iq2, int num2)
		: QRunnable()
		,_numIQ((num1+num2)/2)
		,_data(-1)
		,_results(-1)
		,_window(-1)
	{
	// Obtain two buffers, one for the I,Q inputs, one for outputs
	DataMgr &dmgr		= DataMgr::instance();
	_results			= dmgr.fftBlockFor(_numIQ);
	_data				= dmgr.fftBlockFor(_numIQ);

	// Copy the IQ data (num1= number of doubles) to the input data start
	fftw_complex *data	= dmgr.asFFT(_data);
	::memcpy(data, iq1, num1*sizeof(double));

	// Skip by num1/2 complex doubles, copy remainder of doubles
	data += num1/2;
	::memcpy(data, iq2, num2*sizeof(double));

	// Now have _numIQ complex-pairs stored into {_data}. Rotate by Pi to
	// center the data after FFT
	_rotateByPi();
	}

/******************************************************************************\
|* Destructor
\******************************************************************************/
TaskFFT::~TaskFFT(void)
	{
	if (_data >= 0)
		DataMgr::instance().release(_data);
	}


/******************************************************************************\
|* Process the FFT
\******************************************************************************/
void TaskFFT::run(void)
	{
	DataMgr &dmgr		= DataMgr::instance();

	/**********************************************************************\
	|* Apply the windowing function to the data
	\**********************************************************************/
	if (_window >= 0)
		{
		double *window		= dmgr.asDouble(_window);
		if (window != nullptr)
			{
			fftw_complex *input	= dmgr.asFFT(_data);

			for (int i=0; i<_numIQ; i++)
				{
				input[i][0] *= window[i];
				input[i][1] *= window[i];
				}
			}
		}

	/**********************************************************************\
	|* Perform the FFT
	\**********************************************************************/
	fftw_complex *src = dmgr.asFFT(_data);
	fftw_complex *dst = dmgr.asFFT(_results);
	fftw_execute_dft(_plan, src, dst);

	/**********************************************************************\
	|* And tell the world we're done
	\**********************************************************************/
	emit fftDone(_results);
	}


/******************************************************************************\
|* Stolen shamelessly from the rtl-power-fftw source code. The magic aligment
|* happens here: we have to change the phase of each next complex sample by pi
|* - this means that even numbered samples stay the same while odd numbered
|* samples et multiplied by -1 (thus rotated by pi in complex plane).
|*
|* This gets us output spectrum shifted by half its size - just what we need
|* to get the output right.
\******************************************************************************/
void TaskFFT::_rotateByPi(void)
	{
	DataMgr &dmgr		= DataMgr::instance();
	fftw_complex *input	= dmgr.asFFT(_data);

	for (int i=1; i<_numIQ; i+=2)
		{
		input[i][1] = - input[i][1];
		input[i][0] = - input[i][0];
		}
	}


/******************************************************************************\
|* Test interface : Return the number of tests we implement
\******************************************************************************/
int TaskFFT::numTests(void)
	{
	return 4;
	}

/******************************************************************************\
|* Test interface : identify the class being tested
\******************************************************************************/
const char * TaskFFT::testClassName(void)
	{
	return "TaskFFT";
	}

/******************************************************************************\
|* Test interface : Run a given test
\******************************************************************************/
Testable::TestResult TaskFFT::runTest(int idx)
	{
	switch (idx)
		{
		case 0:
			return _checkSingleBufferConstructor();
		case 1:
			return _checkDoubleBufferConstructor();
		case 2:
			return _checkComplexDataAccess();
		case 3:
			return _checkFFTCorrectness();
		}

	ERR << "Test requested outside of range";
	return Testable::TEST_FAIL;
	}


/******************************************************************************\
|* Test interface : Test out the first constructor
\******************************************************************************/
Testable::TestResult TaskFFT::_checkSingleBufferConstructor(void)
	{
	DataMgr &dmgr = DataMgr::instance();

	/**************************************************************************\
	|* Construct a double* array of known IQ values
	\**************************************************************************/
	double data[64];
	for (int i=0; i<64; i++)
		data[i] = i;

	TaskFFT dut(data, 64);
	double *input = dmgr.asDouble(dut.data());
	dmgr.release(dut.results());

	// Do the 'rotate by pi' thing on the double data to do the compare
	for (int i=0; i<64; i+=4)
		{
		data[i+2]	= -data[i+2];
		data[i+3]	= -data[i+3];
		}

	for(int i=0; i<64; i++)
		if ((int)(data[i]) != (int)(input[i]))
			{
			ERR << "Mismatch in data at position " << i;

			for (int i=0; i<64; i++)
				fprintf(stderr, "  %3i : %f : %f\n",
						i, data[i], input[i]);
			return Testable::TEST_FAIL;
			}
	return Testable::TEST_PASS;
	}

/******************************************************************************\
|* Test interface : Test out the second constructor
\******************************************************************************/
Testable::TestResult TaskFFT::_checkDoubleBufferConstructor(void)
	{
	DataMgr &dmgr = DataMgr::instance();

	/**************************************************************************\
	|* Construct a double* array of known IQ values
	\**************************************************************************/
	double data[64];

	double data1[16];
	for (int i=0; i<16; i++)
		data1[i] = data[i] = i;

	double data2[48];
	for (int i=0; i<48; i++)
		data2[i] = data[i+16] = i+16;


	TaskFFT dut(data1, 16, data2, 48);
	double *input = dmgr.asDouble(dut.data());
	dmgr.release(dut.results());

	// Do the 'rotate by pi' thing on the double data to do the compare
	for (int i=0; i<64; i+=4)
		{
		data[i+2]	= -data[i+2];
		data[i+3]	= -data[i+3];
		}

	for(int i=0; i<64; i++)
		if ((int)(data[i]) != (int)(input[i]))
			{
			ERR << "Mismatch in data at position " << i;

			for (int i=0; i<64; i++)
				fprintf(stderr, "  %3i : %f : %f\n",
						i, data[i], input[i]);
			return Testable::TEST_FAIL;
			}
	return Testable::TEST_PASS;
	}

/******************************************************************************\
|* Test interface : Test that the complex-structure access works
\******************************************************************************/
Testable::TestResult TaskFFT::_checkComplexDataAccess(void)
	{
	DataMgr &dmgr = DataMgr::instance();

	/**************************************************************************\
	|* Construct a double* array of known IQ values
	\**************************************************************************/
	double data[64];
	for (int i=0; i<64; i++)
		data[i] = i;

	TaskFFT dut(data, 64);
	fftw_complex *input	= dmgr.asFFT(dut.data());
	dmgr.release(dut.results());

	// Do the 'rotate by pi' thing on the double data to do the compare
	for (int i=0; i<64; i+=4)
		{
		data[i+2]	= -data[i+2];
		data[i+3]	= -data[i+3];
		}

	for(int i=0; i<32; i++)
		{
		int I	= input[i][0];
		int Q	= input[i][1];

		int ii	= data[i*2];
		int qq	= data[i*2+1];

		if ((I != ii) || (Q != qq))
			{
			ERR << "Mismatch in data at position " << i;

			for (int i=0; i<32; i++)
				fprintf(stderr, "  %3i : {%f, %f} : {%f, %f}\n",
						i, data[i*2], data[i*2+1],
						input[i][0], input[i][1]);
			return Testable::TEST_FAIL;
			}
		}
	return Testable::TEST_PASS;
	}


/******************************************************************************\
|* Test interface : Test that the FFT returns the results we expect
\******************************************************************************/
Testable::TestResult TaskFFT::_checkFFTCorrectness(void)
	{
	/**************************************************************************\
	|* Create the plan
	\**************************************************************************/
	DataMgr &dmgr		= DataMgr::instance();

	int64_t fftIn		= dmgr.fftBlockFor(8);
	int64_t fftOut		= dmgr.fftBlockFor(8);

	fftw_complex *in	= dmgr.asFFT(fftIn);
	fftw_complex *out	= dmgr.asFFT(fftOut);

	fftw_plan plan = fftw_plan_dft_1d(8,in,out,FFTW_FORWARD,FFTW_PATIENT);

	dmgr.release(fftIn);
	dmgr.release(fftOut);

	/**************************************************************************\
	|* Set up input as follows:
	|*
	|*	REAL[0]=  1.000,   IMAG[0]=  0.000
	|*	REAL[1]=  0.000,   IMAG[1]=  0.000
	|*	REAL[2]=  0.000,   IMAG[2]=  0.000
	|*	REAL[3]=  0.000,   IMAG[3]=  0.000
	|*	REAL[4]=  0.000,   IMAG[4]=  0.000
	|*	REAL[5]=  0.000,   IMAG[5]=  0.000
	|*	REAL[6]=  0.000,   IMAG[6]=  0.000
	|*	REAL[7]=  0.000,   IMAG[7]=  0.000
	|*
	|* Expect output as follows:
	|*
	|*	REAL[0]=  1.000,   IMAG[0]=  0.000
	|*	REAL[1]=  1.000,   IMAG[1]=  0.000
	|*	REAL[2]=  1.000,   IMAG[2]=  0.000
	|*	REAL[3]=  1.000,   IMAG[3]=  0.000
	|*	REAL[4]=  1.000,   IMAG[4]=  0.000
	|*	REAL[5]=  1.000,   IMAG[5]=  0.000
	|*	REAL[6]=  1.000,   IMAG[6]=  0.000
	|*	REAL[7]=  1.000,   IMAG[7]=  0.000
	\**************************************************************************/
	double input[16];
	::memset(input, 0, sizeof(double) * 16);
	input[0] = 1.0;

	/**************************************************************************\
	|* Run through the FFT
	\**************************************************************************/
	TaskFFT dut(input, 16);
	dut.setPlan(plan);
	dut.run();

	/**************************************************************************\
	|* Check the results
	\**************************************************************************/
	fftw_complex *results	= dmgr.asFFT(dut.results());

	bool ok = true;
	for (int i=0; i<8 && ok; i++)
		if ((results[i][0] != 1.0) || (results[i][1] != 0))
			ok = false;

	if (!ok)
		{
		ERR << "FFT test result [1] was not as expected";
		for (int i=0; i<8; i++)
			fprintf(stderr, " %d: %f %f\n", i, results[i][0], results[i][1]);
		dmgr.release(dut.results());
		return Testable::TEST_FAIL;
		}

	/**************************************************************************\
	|* Tidy up the above, now do the second FFT accuracy test
	\**************************************************************************/
	dmgr.release(dut.results());

	/**************************************************************************\
	|* Set up input as follows:
	|*
	|*	REAL[0]=  0.000,		IMAG[0]=  0.000
	|*	REAL[1]=  1.000,		IMAG[1]=  0.000
	|*	REAL[2]=  0.000,		IMAG[2]=  0.000
	|*	REAL[3]=  0.000,		IMAG[3]=  0.000
	|*	REAL[4]=  0.000,		IMAG[4]=  0.000
	|*	REAL[5]=  0.000,		IMAG[5]=  0.000
	|*	REAL[6]=  0.000,		IMAG[6]=  0.000
	|*	REAL[7]=  0.000,		IMAG[7]=  0.000
	|*
	|* Expect output as follows (due to rotate-by-pi effect)
	|*
	|*	REAL[4]=  1.000,		IMAG[4]=  0.000
	|*	REAL[5]=  0.707107,		IMAG[5]=  -0.707107
	|*	REAL[6]=  0.000,		IMAG[6]=  -1.000
	|*	REAL[7]=  -0.707107,	IMAG[7]=  -0.707107
	|*	REAL[0]=  -1.000,		IMAG[0]=  0.000
	|*	REAL[1]=  -0.707107,	IMAG[1]=  0.707107
	|*	REAL[2]=  0.000,		IMAG[2]=  1.000
	|*	REAL[3]=  0.707107,		IMAG[3]=  0.707107
	\**************************************************************************/
	::memset(input, 0, sizeof(double) * 16);
	input[2] = 1.0;

	double R[8] = {-1,-0.707107,0,0.707107,1,0.707107,0,-0.707107};
	double I[8] = {0, 0.707107, 1, 0.707107, 0, -0.707107, -1, -0.707107};

	/**************************************************************************\
	|* Run through the FFT
	\**************************************************************************/
	TaskFFT dut2(input, 16);
	dut2.setPlan(plan);
	dut2.run();

	/**************************************************************************\
	|* Check the results
	\**************************************************************************/
	results	= dmgr.asFFT(dut2.results());

	ok = true;
	for (int i=0; i<8 && ok; i++)
		{
		int rr = results[i][0] * 100000;
		int ii = results[i][1] * 100000;
		int RR = R[i] * 100000;
		int II = I[i] * 100000;

		if ((rr != RR) || (ii != II))
			{
			fprintf(stderr, "Failed at bin %d (%d:%d, %d:%d)\n",
							i, rr, RR, ii, II);
			ok = false;
			}
		}

	if (!ok)
		{
		ERR << "FFT test result [2] was not as expected";
		for (int i=0; i<8; i++)
			fprintf(stderr, " %d: %f %f\n", i, results[i][0], results[i][1]);
		dmgr.release(dut2.results());
		return Testable::TEST_FAIL;
		}

	/**************************************************************************\
	|* Tidy up
	\**************************************************************************/
	dmgr.release(dut2.results());
	return Testable::TEST_PASS;
	}
