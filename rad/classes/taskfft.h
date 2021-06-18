#ifndef TASKFFT_H
#define TASKFFT_H

#include <fftw3.h>

#include <QObject>
#include <QRunnable>

#include "properties.h"

class TaskFFT : public QObject, public QRunnable
	{
	Q_OBJECT

	/**************************************************************************\
	|* Properties
	\**************************************************************************/
	GET(int, numIQ);						// Number of IQ points
	GET(int64_t, data);						// Buffer: Input to FFT
	GET(int64_t, results);					// Buffer: Output from FFT
	SET(fftw_plan, plan, Plan);				// FFT plan for fftw3
	GETSET(int64_t, window, Window);		// Buffer: FFT windowing data

	private:
		/**********************************************************************\
		|* Stolen shamelessly from the rtl-power-fftw source code. The magic
		|* aligment happens here: we have to change the phase of each next
		|* complex sample by pi - this means that even numbered samples stay
		|* the same while odd numbered samples et multiplied by -1 (thus
		|* rotated by pi in complex plane).
		|*
		|* This gets us output spectrum shifted by half its size - just what
		|* we need to get the output right.
		\**********************************************************************/
		void _rotateByPi();

	public:
		/**********************************************************************\
		|* Constructors and destructor
		\**********************************************************************/
		TaskFFT(double *iq, int num);
		TaskFFT(double *iq1, int num1, double *iq2, int num2);
		~TaskFFT(void);

		/**********************************************************************\
		|* Method called to run the task
		\**********************************************************************/
		void run() override;

	signals:
		/**********************************************************************\
		|* FFT done, please aggregate this data
		\**********************************************************************/
		void fftDone(int bufferId);
	};

#endif // TASKFFT_H
