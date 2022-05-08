/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}
#include <string>
#include <vector>

#pragma comment(lib,"E:/projects/pragma/third_party_libs/luajit/src/lua51.lib")
#pragma comment(lib,"E:/projects/pragma/third_party_libs/luajit/src/luajit.lib")

static FILE *f_compile = nullptr;
static int lua_write_binary(lua_State*,unsigned char *str,size_t len,struct luaL_Buffer*)
{
	fwrite(str,1,len,f_compile);
    return 0;
}

static bool compile_file(lua_State *l,const std::string &inFile,const std::string &outFile)
{
	auto *f = fopen(inFile.c_str(),"r");
	if(!f)
		return false;
	fseek(f,0,SEEK_END);
	auto len = ftell(f);
	fseek(f,0,SEEK_SET);
	std::vector<char> buffer;
	buffer.resize(len);
	fread(buffer.data(),1,buffer.size(),f);
	fclose(f);

	auto res = luaL_loadbuffer(l,buffer.data(),buffer.size(),"load");
	if(res != LUA_OK)
		return false;

	f_compile = fopen(outFile.c_str(),"wb");
	if(f_compile == nullptr)
	{
		lua_pop(l,1);
		return false;
	}
	luaL_Buffer buf;
	luaL_buffinit(l,&buf);
	lua_dump_strip(l,(lua_Writer)lua_write_binary,&buf,1);
	return true;
}

int main(int argc,char *argv[])
{
	if(argc < 2)
		return EXIT_FAILURE;
	auto *l = lua_open();
	if(!l)
		return EXIT_FAILURE;
	std::string inFile = argv[1];
	auto outFile = inFile;
	auto rpos = outFile.find_last_of('.');
	if(rpos == std::string::npos)
		return EXIT_FAILURE;
	outFile = outFile.substr(0,rpos) +".clua";
	compile_file(l,inFile,outFile);
	lua_close(l);
	return EXIT_SUCCESS;
}
