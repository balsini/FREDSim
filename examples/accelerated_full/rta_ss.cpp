#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "rta_ss.h"

#define EPS 0.0001

void testLP()
{
	
	MIP_TYPE mip;
	
	mip.obj(0);
	mip.min();
	mip.solve();
	
	if (mip.type() == MipSolver::OPTIMAL) 
	{
			std::cout << "OK" << std::endl;
	}
	
}

void SS_Task_RTA::prettyPrintSolution(MIP_TYPE& mip, vector< Mip::Col>& R, 
								  vector< vector< vector< Mip::Col>>>& O,
								  vector< vector< vector< Mip::Col>>>& NI,
								  vector< vector< vector< Mip::Col>>>& CEIL,
								  const unsigned int i)
{
	
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
		std::cout << "R[" << j << "] = " << mip.sol(R[j]) << std::endl;
	
	for(unsigned int k=0; k < i; k++)
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
		{
			for(unsigned int j=0; j < taskset[i].C.size(); j++)
			{
					std::cout << "    O[" << k << "][" << z << "][" << j << "] = " << mip.sol(O[k][z][j]) << std::endl;
					std::cout << "   NI[" << k << "][" << z << "][" << j << "] = " << mip.sol(NI[k][z][j]) << std::endl;
					std::cout << "CEIL[" << k << "][" << z << "][" << j << "] = " << mip.sol(CEIL[k][z][j]) << std::endl;
			}	
			std::cout << "----------------------" << std::endl;
		}	
}

double SS_Task_RTA::computeWCRT_UB(unsigned int i)
{
	double constant = 0;
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
		constant += taskset[i].C[j];
	for(unsigned int j=0; j < taskset[i].C.size()-1; j++)
		constant += taskset[i].S[j];
	
	#ifdef __DEBUG_RTA_SS__
	std::cout << "[computeWCRT_UB] constant part = " << constant << std::endl;
	#endif
	
	vector<double> C;
	for(unsigned int k=0; k < i; k++)
	{
		double C_k = 0;
		for(auto C_kj : taskset[k].C)
			C_k += C_kj;
		
		C.push_back(C_k);
	}
			
	vector<double> J;
	for(unsigned int k=0; k < i; k++)
		J.push_back(WCRTs[k]-C[k]);
	
	double L=constant;
	double newL=-1;
	while(true)
	{
		double acc=0;
		for(unsigned int k=0; k < i; k++)
			acc += ceil((L + J[k])/taskset[k].T)*C[k];
		
		newL = constant + acc;
		
		#ifdef __DEBUG_RTA_SS__
		std::cout << "[computeWCRT_UB] newL = " << newL << std::endl;
		#endif
		
		if(fabs(newL-L)<EPS) break;
		if(newL>taskset[i].D) break;
		
		L = newL;
		
	}
	
	return newL;
}


double SS_Task_RTA::computeWCRTsubtask_UB(unsigned int i, unsigned int j)
{
	vector<double> C;
	for(unsigned int k=0; k < i; k++)
	{
		double C_k = 0;
		for(auto C_kj : taskset[k].C)
			C_k += C_kj;
		
		C.push_back(C_k);
	}
			
	vector<double> J;
	for(unsigned int k=0; k < i; k++)
		J.push_back(WCRTs[k]-C[k]);
	
	double L=taskset[i].C[j];
	double newL=-1;
	while(true)
	{
		double acc=0;
		for(unsigned int k=0; k < i; k++)
			acc += ceil((L + J[k])/taskset[k].T)*C[k];
		
		newL = taskset[i].C[j] + acc;
		
		#ifdef __DEBUG_RTA_SS__
		std::cout << "[computeWCRTsubtask_UB] newL = " << newL << std::endl;
		#endif
		
		if(fabs(newL-L)<EPS) break;
		if(newL>taskset[i].D) break;
		
		L = newL;
		
	}
	
	return newL;
}

double SS_Task_RTA::computeRT(const unsigned int i)
{
	MIP_TYPE mip;
	
	// ---------- [ Variables definition ]---------------
	
	// R_j : Response-time of the j-th sub-task 
	vector< Mip::Col> R(taskset[i].C.size());
    mip.addColSet(R);
	
	// O_{k,z,j} : Offset of z-t sub-task of the k-th hp-task with the j-th sub-task of T_i
	vector< vector< vector< Mip::Col>>> O(i);
	for(unsigned int k=0; k < i; k++)
	{
		vector< vector< Mip::Col>> per_hp_task(taskset[k].C.size());
		O[k] = per_hp_task;
		
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
		{
			vector< Mip::Col> per_hp_subtask(taskset[i].C.size());
			O[k][z] = per_hp_subtask;
			mip.addColSet(O[k][z]);
		}
	}	
	
	// NI_{k,z,j} : Number of interfering jobs of the z-th subtask of k-th hp-task with the j-th sub-task
	vector< vector< vector< Mip::Col>>> NI(i);
	for(unsigned int k=0; k < i; k++)
	{
		vector< vector< Mip::Col>> per_hp_task(taskset[k].C.size());
		NI[k] = per_hp_task;
		
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
		{
			vector< Mip::Col> per_hp_subtask(taskset[i].C.size());
			NI[k][z] = per_hp_subtask;
			mip.addColSet(NI[k][z]);
		}
	}
	
	// CEIL_{k,z,j} : Aux variable, ceil((R_j - O_{k,z,j})/T_k)
	vector< vector< vector< Mip::Col>>> CEIL(i);
	for(unsigned int k=0; k < i; k++)
	{
		vector< vector< Mip::Col>> per_hp_task(taskset[k].C.size());
		CEIL[k] = per_hp_task;
		
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
		{
			vector< Mip::Col> per_hp_subtask(taskset[i].C.size());
			CEIL[k][z] = per_hp_subtask;
			mip.addColSet(CEIL[k][z]);
		}
	}
	
	// ---------- [ Variables type, UB, LB ]---------------
	
	for(unsigned int k=0; k < taskset[i].C.size(); k++)
	{
		mip.colType(R[k], Mip::REAL);
		mip.colLowerBound(R[k], 0); // R_k >= 0
	}
	
	for(unsigned int k=0; k < i; k++)
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
			for(unsigned int j=0; j < taskset[i].C.size(); j++)
				mip.colType(O[k][z][j], Mip::REAL);
	
	for(unsigned int k=0; k < i; k++)
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
			for(unsigned int j=0; j < taskset[i].C.size(); j++)
			{
				mip.colType(NI[k][z][j], Mip::INTEGER);
				mip.colLowerBound(NI[k][z][j], 0); // NI_{k,z,j} >= 0
			}
	
	for(unsigned int k=0; k < i; k++)
		for(unsigned int z=0; z < taskset[k].C.size(); z++)
			for(unsigned int j=0; j < taskset[i].C.size(); j++)
			{
				mip.colType(CEIL[k][z][j], Mip::INTEGER);
				mip.colLowerBound(CEIL[k][z][j], 0); // CEIL_{k,z,j} >= 0
			}
	
	// ---------- [ Objective function ]---------------
	
	Mip::Expr objFun;
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
		objFun+=R[j];
	
	mip.max();
	mip.obj(objFun);
	
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
	{
		double UB_j = computeWCRTsubtask_UB(i, j);
		#ifdef __DEBUG_RTA_SS__
		std::cout << "UB_" << j << " = " << UB_j <<std::endl;
		#endif
		mip.addRow(R[j] <= UB_j);
	}
	
	// Constraint 1
	Mip::Expr C9_expr;
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
		C9_expr+=R[j];
	for(unsigned int j=0; j < taskset[i].C.size()-1; j++)
		C9_expr += taskset[i].S[j];
	
	double UB_ss = computeWCRT_UB(i);
	#ifdef __DEBUG_RTA_SS__
	std::cout << "UB = " << UB_ss <<std::endl;
	#endif
	
	mip.addRow(C9_expr <= UB_ss);
	
	// Constraint 2
	// For each sub-task 
	for(unsigned int j=0; j < taskset[i].C.size(); j++)
	{
		Mip::Expr C10_expr;
		C10_expr += taskset[i].C[j];
		// For each HP-task 
		for(unsigned int k=0; k < i; k++)	
			for(unsigned int z=0; z < taskset[k].C.size(); z++)
				C10_expr += NI[k][z][j]*taskset[k].C[z];
		
		mip.addRow(R[j] == C10_expr);
	}
	
	// Constraint 3
	for(unsigned int k=0; k < i; k++)
		for(unsigned int j=0; j < taskset[i].C.size(); j++)
			for(unsigned int z=0; z < taskset[k].C.size(); z++)
			{
				double J_kz = WCRTs[k]-taskset[k].C[z];
				
				mip.addRow(O[k][z][j] >= -J_kz);
				
				if(j<taskset[i].C.size()-1)
				{
					mip.addRow( O[k][z][j+1] >= O[k][z][j] + NI[k][z][j]*taskset[k].T - R[j] - taskset[i].S[j]  -J_kz);
				}
				
				//Set-up aux variables for ceil
				mip.addRow( CEIL[k][z][j] - 1 <= R[j]/taskset[k].T - O[k][z][j]/taskset[k].T -EPS );
				mip.addRow( CEIL[k][z][j] >= R[j]/taskset[k].T - O[k][z][j]/taskset[k].T );
				
				mip.addRow( NI[k][z][j] <= CEIL[k][z][j] );
			}
	
	#ifdef __DEBUG_RTA_SS__
	std::cout << "pre solve()" << std::endl;
	#endif
	
	// Solve
	mip.solve();
	
	#ifdef __DEBUG_RTA_SS__
	std::cout << "post solve()" << std::endl;
	#endif
	
	if (mip.type() == MipSolver::OPTIMAL)
	{
		#ifdef __DEBUG_RTA_SS__
		std::cout << "MILP successfully solved" << std::endl;
		
		prettyPrintSolution(mip, R, O, NI, CEIL, i);
		#endif
		
		double S = 0;
		for(unsigned int j=0; j < taskset[i].C.size()-1; j++)
			S += taskset[i].S[j];
		
		return mip.solValue() + S;
	}
	// If something went wrong with the MILP, the task is deemed UNSCHEDULABLE
	else
	{
		#ifdef __DEBUG_RTA_SS__
		std::cout << "ERROR when solving the MILP..." << std::endl;
		#endif
		
		return taskset[i].D+1;
	}
	
}

bool SS_Task_RTA::alreadySchedulable()
{
	
	for(unsigned int i=0; i<taskset.size(); i++)
		if(WCRTs[i] > taskset[i].D) 
			return false;
	
	return true;
}


bool SS_Task_RTA::necessaryConditionsHold()
{
	
	double U = 0;
	
	for(unsigned int i=0; i<taskset.size(); i++)
	{	
		double C_i = 0, S_i = 0;
		for(auto C_ij : taskset[i].C)
			C_i += C_ij;
		for(auto S_ij : taskset[i].S)
			S_i += S_ij;
		
		if(C_i + S_i > taskset[i].D) return false;
		U += C_i/taskset[i].T;
	}
	
	if(U>1) return false;
	
	return true;
}

bool SS_Task_RTA::isSchedulable()
{
	
	if(alreadySchedulable()) return true;
	if(!necessaryConditionsHold()) return false;
	
	double C_0 = 0, S_0 = 0;
	for(auto C_0j : taskset[0].C)
		C_0 += C_0j;
	for(auto S_0j : taskset[0].S)
		S_0 += S_0j;
	
	// No interference for the highest-prio task 
	WCRTs[0] = C_0 + S_0;
	
	#ifdef __DEBUG_RTA_SS__
	std::cout << "WCRT0 = " << WCRTs[0] << std::endl;
	#endif
	
	// For all SS-tasks
	for(unsigned int i=1; i<taskset.size(); i++)
	{
		WCRTs[i] = computeRT(i);
		
		#ifdef __DEBUG_RTA_SS__
		std::cout << "WCRT" << i << " = " << WCRTs[i] << std::endl;
		#endif 
		
		if(WCRTs[i]>taskset[i].D) return false;
	}
	
	return true;
}

void SS_Task_RTA::printTaskset()
{
	for(unsigned int i=0; i<taskset.size(); i++)
	{
		std::cout << "Task i=" << i << std::endl;
		
		std::cout << "C = { ";
		for(auto C_ij : taskset[i].C)
			std::cout << C_ij << " ";
		
		std::cout << "}" << std::endl;
		
		std::cout << "S = { ";
		for(auto S_ij : taskset[i].S)
			std::cout << S_ij << " ";
		
		std::cout << "}" << std::endl;
		std::cout << "T = " << taskset[i].T << std::endl;
		std::cout << "D = " << taskset[i].D << std::endl;
		std::cout << std::endl << std::endl;
	}
}