#define REGISTER_FUNC(name, parser) int name(const Arg *arg);
#define REGISTER_FUNC_ALIAS(alias, name, parser) int name(const Arg *arg);
#include "function_registry.def"
#undef REGISTER_FUNC
#undef REGISTER_FUNC_ALIAS