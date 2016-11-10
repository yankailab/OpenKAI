/*
 * Filter.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTER_H_
#define SRC_ALGORITHM_FILTER_H_

#define SWITCH(x,y,t) (t=x,x=y,y=t)

#define FILTER_BUF 100

namespace kai
{

class Filter
{
public:
	Filter();
	virtual ~Filter();

	bool startMedian(int windowLength);
	void input(double v);
	double v(void);

private:
	int m_iTraj;
	int m_iMedian;
	int m_windowLength;
	double m_trajectory[FILTER_BUF];
	double m_vMid;

};

} /* namespace kai */

#endif /* SRC_ALGORITHM_FILTER_H_ */
