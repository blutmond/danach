#pragma once

#include "rules/template-support.h"
#include "parser/ast-context.h"

namespace widget_spec {

void EmitWidgetSpec(ASTContext& ast_ctx, const std::string& spec,
                      std::ostream& h_stream,
                      std::ostream& cc_stream);

}  // namespace
