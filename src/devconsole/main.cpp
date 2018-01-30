#include <cppcoretools\print.h>

int main(int argc, char** argv)
{
	cct::scoped_failure_handler{ [](char const* op)
	{
		fprintf(stderr, "Operation '%s' failed\n", op);
	} };
	CCT_CHECK(argc >= 2);
	cct::println("@x = global i32 5, align 4");
	return 0;
}
