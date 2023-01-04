
struct function {
	struct museq museq;
};

struct module {
	struct function *functions;
	size_t num_functions;
};
