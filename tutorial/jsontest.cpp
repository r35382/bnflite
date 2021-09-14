/****************************************************************************\
*   Unit test pf JSONlite parser and repository                              *
*   Copyright (c) 2017  Alexander A. Semjonov <alexander.as0@mail.ru>        *
*                                                                            *
*   Permission to use, copy, modify, and distribute this software for any    *
*   purpose with or without fee is hereby granted, provided that the above   *
*   copyright notice and this permission notice appear in all copies.        *
*                                                                            *
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES *
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF         *
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR  *
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   *
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    *
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF  *
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.           *
\****************************************************************************/

#include "jsonlite.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

const char*	json_file =
"{"
"\"Array\":["
 "{\"Name\" : \"Nested Object\" },"
 "[ 20, 10, true, -1, \"Nested Array\" ]"
"],"
 "\"Object\":{\"name\":\"receipt\", \"dept\":30.5,\"cars\":[\"Ford\", \"KIA\", \"Fiat\"]}"
"}"
;

int main()
{
    const char* tail = 0;
    int status = 0;
	Repo repo =  Repo::ParseJSON(json_file, &status, &tail);
	if (status < 0) {
	    cout << " Parsing error in the command, " << "error flags = {" << std::hex
	        << (status&eRest?"eRest":"")
	        << (status&eOver?", eOver":"")
	        << (status&eEof?", eEof":"")
	        << (status&eBadRule?", eBadRule":"")
	        << (status&eBadLexem?", eBadLexem":"")
	        << (status&eSyntax?", eSyntax":"")
	        << (status&eError?", eError":"")
	        << "},\n stopped at $> "
	        << tail << "}"
	    << endl;
        return -1;
	}
    Repo obj = repo("Object");
    cout << "repo(\"Array\")[1][4] = " << repo("Array")[1][4].ToString() << endl;
    cout << "repo(\"Object\")(\"name\") = " << repo("Object")("name").ToString() << endl;
    cout << "repo(\"Object\")(\"dept\") = " << obj("dept").ToDouble() << endl;
    cout << "repo(\"Object\")(\"cars\").Size = " << repo("Object")("cars").Size() << endl;
#if 1
    /* tree dump of repository */
	repo.dumptree(cout);
#else
    /* raw dump of repository */
    for( auto it: *repo.get()) {
	    cout << "{ (" << it.first.first << ", " << it.first.second << "), "
				<< it.second.first << ", " << it.second.second << " }" << endl;
	}
#endif
	cout << endl;
  
    return 0;
}
