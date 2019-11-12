#pragma once

#include "gen/parser/lowering_spec.h"

namespace lowering_spec {

void Emit(std::ostream &stream, Module* module);

}  // namespace lowering_spec
