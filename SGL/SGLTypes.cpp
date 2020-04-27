#include "SGLTypes.h"

void register_datatypes()
{
	register_type<int>("int32");
	register_type<float>("float");
	
	// registering the "void" type is special
	SGLType voidType;
	voidType.TypeAlignment = 0;
	voidType.TypeSize = 0;
	voidType.TypeName = "void";
	get_types()["void"] = voidType;
}