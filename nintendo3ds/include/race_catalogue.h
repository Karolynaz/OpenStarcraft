#pragma once
#include "terran_catalogue.h"
#include "protoss_catalogue.h"
namespace port3ds {namespace catalogue {
template<class F> void buildings(F f){for(const auto& p:terran::buildings)f(p);for(const auto& p:protoss::buildings)f(p);}
template<class F> void units(F f){for(const auto& p:terran::units)f(p);for(const auto& p:protoss::units)f(p);}
template<class F> void techs(F f){for(const auto& p:terran::techs)f(p);for(const auto& p:protoss::techs)f(p);}
template<class F> void upgrades(F f){for(const auto& p:terran::upgrades)f(p);for(const auto& p:protoss::upgrades)f(p);}
}}
