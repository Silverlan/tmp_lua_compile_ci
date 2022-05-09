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
#include <iostream>
#include <fstream>

#pragma comment(lib,"E:/projects/pragma/third_party_libs/luajit/src/lua51.lib")
#pragma comment(lib,"E:/projects/pragma/third_party_libs/luajit/src/luajit.lib")

static FILE *f_compile = nullptr;
static int lua_write_binary(lua_State*,unsigned char *str,size_t len,struct luaL_Buffer*)
{
	fwrite(str,1,len,f_compile);
    return 0;
}

static bool compile_file(lua_State *l,const std::string &inFile,const std::string &outFile,std::string &outErr)
{
	std::ifstream f {inFile};
	if(!f)
	{
		outErr = "Failed to open input file '" +inFile +"'!";
		return false;
	}
	std::string content ((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close();

	auto res = luaL_loadbuffer(l,content.data(),content.size(),"load");
	if(res != LUA_OK)
	{
		outErr = "Failed to load buffer: " +std::to_string(res);
		if(lua_isstring(l,-1))
			outErr += ": " +std::string{lua_tostring(l,-1)};
		outErr += "!";
		return false;
	}

	f_compile = fopen(outFile.c_str(),"wb");
	if(f_compile == nullptr)
	{
		outErr = "Failed to open output file '" +outFile +"'!";
		lua_pop(l,1);
		return false;
	}
	luaL_Buffer buf;
	luaL_buffinit(l,&buf);
	res = lua_dump_strip(l,(lua_Writer)lua_write_binary,&buf,1);
	if(res != LUA_OK)
	{
		outErr = "Failed to dump strip: " +std::to_string(res);
		if(lua_isstring(l,-1))
			outErr += ": " +std::string{lua_tostring(l,-1)};
		outErr += "!";
		return false;
	}
	return true;
}

int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		std::cerr<<"Insufficient number of arguments."<<std::endl;
		return EXIT_FAILURE;
	}
	auto *l = lua_open();
	if(!l)
	{
		std::cerr<<"Failed to open lua state."<<std::endl;
		return EXIT_FAILURE;
	}
	std::string inFile = argv[1];
	auto outFile = inFile;
	auto rpos = outFile.find_last_of('.');
	if(rpos == std::string::npos)
	{
		std::cout<<"Unexpected input file name."<<std::endl;
		return EXIT_FAILURE;
	}
	outFile = outFile.substr(0,rpos) +".clua";
	std::string err;
	if(compile_file(l,inFile,outFile,err))
		std::cout<<"Compiling was successful, compiled script has been saved as '"<<outFile<<"'!"<<std::endl;
	else
		std::cout<<"Compiling file '"<<inFile<<"' has failed: "<<err<<std::endl;
	lua_close(l);
	return EXIT_SUCCESS;
}
