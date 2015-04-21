#ifndef travel_H_
#define travel_H_
#pragma once

#include "derplanner/runtime/types.h"

#ifndef PLNNR_DOMAIN_API
#define PLNNR_DOMAIN_API
#endif

extern "C" PLNNR_DOMAIN_API void travel_init_domain_info();
extern "C" PLNNR_DOMAIN_API const plnnr::Domain_Info* travel_get_domain_info();

#endif
