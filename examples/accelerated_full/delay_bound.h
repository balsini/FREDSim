#pragma once

#include "rta_ss.h"
#include "generator.h"

namespace FRED
{
	enum FRED_config_t { PREEMPTIVE_FRI, NON_PREEMPTIVE_FRI, STATIC };
}

SS_taskset_t convertSimplifiedFRED_to_SStaskset(const Environment_details_t &e, FRED::FRED_config_t);
void printDelayBounds(const Environment_details_t &e, FRED::FRED_config_t config);