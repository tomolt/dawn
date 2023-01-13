
typedef struct type Type;

enum {
	TYPE_INT32,
	TYPE_INT64,
	TYPE_RECORD,
	TYPE_BRANDED_RECORD,
};

struct record_field {
	char name[100];
	Type type;
};

struct record {
	size_t length;
	struct record_field fields[];
};

struct type {
	int kind;
	struct record *record;
};

bool
types_are_compatible(Type type1, Type type2)
{
	if (type1.kind != type2.kind) return false;
	switch (type1.kind) {
	case TYPE_RECORD:
		if (type1.record == type2.record) return true;
		if (type1.record->length != type2.record->length) return false;
		for (size_t i = 0; i < type1.record->length; i++) {
			if (!types_are_compatible(type1.record->fields[i].type, type2.record->fields[i].type)) {
				return false;
			}
		}
		return true;

	case TYPE_BRANDED_RECORD:
		return type1.record == type2.record;

	default: return true;
	}
}

