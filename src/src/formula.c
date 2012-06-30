#include "frgg.h"

/*
d - doc
t - term
q - query
N - total number of docs
df(t) - doc freqs of t
tf(d, t) - term freq in d
*/


float
weight_d_t(unsigned short tf)
{
	if (tf == 0)
		return 0;
	else
		return 1 + logf(tf);
}


	
float weight_q_t(unsigned int df, unsigned int ndocs)
{
	if (df == 0)
		return 0;
	return logf(1 + ndocs / (float)df);
}

