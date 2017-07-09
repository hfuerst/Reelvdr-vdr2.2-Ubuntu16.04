/*
 * OO wrapper for regular expression functions
 *
 * Copyright (C) 2003  Enrico Zini <enrico@debian.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <debtags/Regexp.h>

using namespace stringf;
using namespace std;
using namespace Debtags;

////// RegexpException

RegexpException::RegexpException(const regex_t& re, int code,
								const string& context) throw ()
							: SystemException(code, context)
{
	int size = 64;
	char* msg = new char[size];
	int nsize = regerror(code, &re, msg, size);
	if (nsize < size)
	{
		delete[] msg;
		msg = new char[nsize];
		regerror(code, &re, msg, nsize);
	}
	_message = msg;
	delete msg;
}


////// Regexp

Regexp::Regexp(const string& expr, int match_count, int flags)
	throw (RegexpException) : pmatch(0), nmatch(match_count)
{
	if (match_count == 0)
		flags |= REG_NOSUB;

	int res = regcomp(&re, expr.c_str(), flags);
	if (res)
		throw RegexpException(re, res, "Compiling regexp \"" + expr + "\"");

	if (match_count > 0)
		pmatch = new regmatch_t[match_count];
}

Regexp::~Regexp() throw ()
{
	regfree(&re);
	if (pmatch)
		delete[] pmatch;
}
	

bool Regexp::match(const string& str, int flags)
	throw(RegexpException)
{
	int res;
	
	if (nmatch)
	{
		res = regexec(&re, str.c_str(), nmatch, pmatch, flags);
		lastMatch = str;
	}
	else
		res = regexec(&re, str.c_str(), 0, 0, flags);

	switch (res)
	{
		case 0:	return true;
		case REG_NOMATCH: return false;
		default: throw RegexpException(re, res, "Matching string \"" + str + "\"");
	}
}

string Regexp::operator[](int idx) throw (Tagcoll::OutOfRangeException)
{
	if (idx > nmatch)
		throw Tagcoll::ValOutOfRangeException<int>("getting submatch of regexp", "index", idx, 0, nmatch);
	
	return string(lastMatch, pmatch[idx].rm_so, pmatch[idx].rm_eo - pmatch[idx].rm_so);
}


// vim:set ts=4 sw=4:
