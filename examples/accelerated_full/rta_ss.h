#ifndef __SS_TASK_H__
#define __SS_TASK_H__

#include <vector>
#include <lemon/lp.h>

using namespace std;
using namespace lemon;

#define Mip MipSolver

#ifdef __RTA_SS_USE_CLEX__
#define MIP_TYPE CplexMip
#else
#define MIP_TYPE GlpkMip
#endif
//#define __DEBUG_RTA_SS__

//-------------------------------------------------

struct SS_task_t
{
	double T;
	double D;
	vector<double> C;
	vector<double> S;
	
	 bool operator<( const SS_task_t& a ) const 
	 {
		return T < a.T; 
     } 
};

typedef vector<SS_task_t> SS_taskset_t;

//-------------------------------------------------

class SS_Task_RTA
{
	
	private:
	
		double computeRT(const unsigned int taskId);
		SS_taskset_t taskset;
		vector<double> WCRTs;
		
		double computeWCRT_UB(unsigned int i);
		double computeWCRTsubtask_UB(unsigned int i, unsigned int j);
		
		bool alreadySchedulable();
		bool necessaryConditionsHold();
		
		void prettyPrintSolution(MIP_TYPE& mip, vector< Mip::Col>& R, 
										 vector< vector< vector< Mip::Col>>>& O,
										 vector< vector< vector< Mip::Col>>>& NI,
										 vector< vector< vector< Mip::Col>>>& CEIL,
										 const unsigned int i);
		
	public:
	
		SS_Task_RTA(SS_taskset_t tset) 
		: taskset(tset)
		{
			for(auto x : taskset)
				WCRTs.push_back(x.D+1);
		}
		
		bool isSchedulable();
		vector<double> getWCRTs() {return WCRTs;}
		void printTaskset();
				
};

void testLP();



#endif
