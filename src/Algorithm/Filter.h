/*
 * Filter.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTER_H_
#define SRC_ALGORITHM_FILTER_H_

#define SWITCH(x,y,t) (t=x,x=y,y=t)

#define FILTER_BUF 1000

namespace kai
{

class Filter
{
public:
	Filter();
	virtual ~Filter();

	bool startMedian(int windowLength);
	double input(double v);

private:
	unsigned int m_iTraj;
	unsigned int m_iMedian;
	unsigned int m_windowLength;
	double m_trajectory[FILTER_BUF];

};

} /* namespace kai */

#endif /* SRC_ALGORITHM_FILTER_H_ */
