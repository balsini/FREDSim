#include "delay_bound.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>



double getDelayBound(const unsigned partId, const unsigned taskId, const Environment_details_t &e)
{
	
	if(partId>=e.slots_per_partition.size() || taskId>=e.task_per_partition.at(partId).size())
		return 0;
	
	double retval = 0;
	
	for (unsigned int i=0; i<e.slots_per_partition.size(); ++i) 
	{
		for (unsigned int j=0; j<e.task_per_partition.at(i).size(); ++j) 
		{
			// Skip myself
			if(i==partId && j==taskId) continue;
			
			// C^H contributes only for tasks with the same affinity
			if(i==partId)
				retval += (double)e.task_per_partition.at(i).at(j).C_HW/e.slots_per_partition.at(i);
			
			// Sum the reconfiguration time
			retval+= (double)e.partition_slot_size.at(i)/e.rho;
			
		}
	}
	
	return retval;
	
}

double getDelayBoundNP(const unsigned partId, const unsigned taskId, const Environment_details_t &e)
{
	
	if(partId>=e.slots_per_partition.size() || taskId>=e.task_per_partition.at(partId).size())
		return 0;
	
	const unsigned int n_HW_tasks = e.task_per_partition.at(partId).size() -1;
	
	double max_r_others = 0;
	
	// For each partition P_i
	for (unsigned int i=0; i<e.slots_per_partition.size(); ++i) 
	{
		if(i==partId) continue;
		const double r = (double)e.partition_slot_size.at(i)/e.rho;
		if(r>max_r_others) max_r_others = r;
	}
	
	return getDelayBound(partId, taskId, e) + n_HW_tasks*max_r_others;

}

SS_taskset_t convertSimplifiedFRED_to_SStaskset(const Environment_details_t &e, FRED::FRED_config_t config)
{
	
	SS_taskset_t taskset;
	
	for (unsigned int i=0; i<e.slots_per_partition.size(); ++i) 
	{
		for (unsigned int j=0; j<e.task_per_partition.at(i).size(); ++j) 
		{
			
			SS_task_t task;
			task.C.push_back(e.task_per_partition.at(i).at(j).C_SW_1);
			task.C.push_back(e.task_per_partition.at(i).at(j).C_SW_2);
			task.T = e.task_per_partition.at(i).at(j).T;
			task.D = e.task_per_partition.at(i).at(j).D;
			
			const double reconfig_time =  (double)e.partition_slot_size.at(i)/e.rho;
			//double S = reconfig_time + e.task_per_partition.at(i).at(j).C_HW + getDelayBound(i,j,e);
			double S = e.task_per_partition.at(i).at(j).C_HW;
			
			if(config==FRED::PREEMPTIVE_FRI)
				S+= reconfig_time + getDelayBound(i,j,e);
			
			if(config==FRED::NON_PREEMPTIVE_FRI)
				S+= reconfig_time + getDelayBoundNP(i,j,e);
			
			task.S.push_back(S);
			
			taskset.push_back(task);
		}
	}
	
	
	std::sort(taskset.begin(), taskset.end() );
	
	return taskset;
}

void printDelayBounds(const Environment_details_t &e, FRED::FRED_config_t config)
{
	 for (unsigned int i=0; i<e.slots_per_partition.size(); ++i) 
	 {
		std::cout << "Partition[" << i << "]:" << std::endl;
		
		for (unsigned int j=0; j<e.task_per_partition.at(i).size(); ++j) 
		{
			std::cout << "\tTask[" << j << "]:" << std::endl;
			switch(config)
			{
				case FRED::PREEMPTIVE_FRI:
					std::cout << "\t\tDELTA\t" << getDelayBound(i,j,e) << std::endl;
					break;
					
				case FRED::NON_PREEMPTIVE_FRI:
					std::cout << "\t\tDELTA\t" << getDelayBoundNP(i,j,e) << std::endl;
					break;
				
				case FRED::STATIC:
					std::cout << "\t\tDELTA\t" << "0" << std::endl;
					break;
			}
		}
	}
}