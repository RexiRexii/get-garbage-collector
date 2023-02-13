int custom_environment::getgc(lua_State* state)
{
	bool full = false;

	/*
	Check if there is at least one argument on the top of the stack and whether that argument is a boolean.
	If there is a boolean argument, the function will store its value in the full local variable.
	*/

	if (lua_gettop(state) > 0 && lua_type(state, 1) == LUA_TBOOLEAN)
		full = lua_toboolean(state, 1);

	/*
	Next, the function creates a new empty table on the stack by calling lua_newtable.
	*/

	lua_newtable(state);

	auto cur_page = state->global->allgcopages;
	int idx{};

	while (cur_page)
	{
		/*
		Iterate over all of the memory pages that are used to store garbage collected objects in the Lua environment.
		For each memory page, the function iterates over all of the objects stored on that page.
		*/

		char* start = 0;
		char* end = 0;
		auto block = 0;
		auto size = 0;

		luaM_getpagewalkinfo(cur_page, &start, &end, &block, &size);

		for (auto pos = start; pos != end; pos += size)
		{
			const auto gco = reinterpret_cast<GCObject*>(pos);

			/*
			For each object, the function checks whether it is a function, a table, or a userdata.
			Or if the full variable is true and the object is a table or userdata
			*/

			if (gco->gch.tt == LUA_TFUNCTION || ((gco->gch.tt == LUA_TTABLE || gco->gch.tt == LUA_TUSERDATA) && full))
			{
				/*
				If so, the function pushes the object onto the stack
				and add it to the table that was created earlier.
				*/

				state->top->value.gc = gco;
				state->top->tt = gco->gch.tt;
				state->top++;

				lua_rawseti(state, -2, ++idx);
			}
		}

		cur_page = cur_page->gcolistnext;
	}

	/*
	After all of the objects have been processed, return 1;
	This indicates that table of objects has been pushed onto the stack.
	*/

	return 1;
}
