#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "lang.h"
#include "syntax.h"

#define MAXBINDINGS 200

typedef struct Binding Binding;

struct Binding {
	uint32_t hash;
	char    *name;
	uint     idx;
};

uint Scope;

static Binding Bindings[MAXBINDINGS];

void
addbinding(const char *name, uint idx)
{
	Binding bind;
	if (Scope == MAXBINDINGS) {
		cerror("too many local bindings.");
	}
	bind.hash = strhash(name);
	bind.name = strdup(name);
	bind.idx  = idx;
	Bindings[Scope++] = bind;
}

uint
lookup(const char *name)
{
	uint32_t hash;
	int i;

	hash = strhash(name);
	for (i = Scope-1; i >= 0; --i) {
		if (Bindings[i].hash == hash && !strcmp(Bindings[i].name, name)) {
			return Bindings[i].idx;
		}
	}
	
	cerror("no binding exists for '%s'.", name);
	return 0;
}

uint
getscope(void)
{
	return Scope;
}

void
resetscope(uint scope)
{
	uint i;
	for (i = scope; i < Scope; ++i) {
		free(Bindings[i].name);
	}
	Scope = scope;
}

