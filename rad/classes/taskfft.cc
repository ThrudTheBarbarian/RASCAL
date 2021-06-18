#include <cstring>

#include <libra.h>

#include "taskfft.h"

/******************************************************************************\
|* Constructor: single buffer
\******************************************************************************/
TaskFFT::TaskFFT(double *iq, int num)
		: QRunnable()
		, _numIQ(num/2)
		, _data(-1)
		,_results(-1)
	{
	Q_ASSERT(num % 2 == 0);

	// Obtain two buffers, one for the I,Q inputs, one for outputs
	DataMgr &dmgr		= DataMgr::instance();
	_results			= dmgr.fftBlockFor(_numIQ);
	_data				= dmgr.fftBlockFor(_numIQ);

	// Copy the IQ data (_numIQ= number of complex doubles) to the input data
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
	double *window		= dmgr.asDouble(_window);
	fftw_complex *input	= dmgr.asFFT(_data);

	for (int i=0; i<_numIQ; i++)
		{
		input[i][0] *= window[i];
		input[i][1] *= window[i];
		}

	/**********************************************************************\
	|* Perform the FFT
	\**********************************************************************/
	fftw_execute_dft(_plan, dmgr.asFFT(_data), dmgr.asFFT(_results));

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

	for (int i=0; i<_numIQ; i+=2)
		{
		input[i][1] = - input[i][1];
		input[i][0] = - input[i][0];
		}
	}

